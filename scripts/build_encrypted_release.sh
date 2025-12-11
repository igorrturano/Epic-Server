#!/bin/bash
#
# Build Encrypted Release
# Creates a release package with encrypted UO data files
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENCRYPTION_KEY="/opt/uo-encryption/key.bin"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

usage() {
    cat << EOF
Usage: $0 [OPTIONS]

Build an encrypted release package for Epic-Server

OPTIONS:
    -s, --source DIR        Source directory containing UO data files
    -o, --output DIR        Output directory for encrypted release (default: ./release)
    -k, --key FILE          Path to encryption key (default: /opt/uo-encryption/key.bin)
    -n, --name NAME         Release name (default: epic-server-encrypted-YYYYMMDD)
    --include-cfg           Also encrypt .cfg files (default: only .mul files)
    --create-archive        Create tar.gz archive of encrypted files
    -h, --help              Show this help message

EXAMPLES:
    # Basic usage
    $0 --source /path/to/uo-data

    # Custom output and release name
    $0 --source /path/to/uo-data --output ./dist --name my-release-v1.0

    # Create archive for distribution
    $0 --source /path/to/uo-data --create-archive

EOF
}

# Default values
SOURCE_DIR=""
OUTPUT_DIR="$PROJECT_ROOT/release"
RELEASE_NAME="epic-server-encrypted-$(date +%Y%m%d)"
INCLUDE_CFG=false
CREATE_ARCHIVE=false

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -s|--source)
            SOURCE_DIR="$2"
            shift 2
            ;;
        -o|--output)
            OUTPUT_DIR="$2"
            shift 2
            ;;
        -k|--key)
            ENCRYPTION_KEY="$2"
            shift 2
            ;;
        -n|--name)
            RELEASE_NAME="$2"
            shift 2
            ;;
        --include-cfg)
            INCLUDE_CFG=true
            shift
            ;;
        --create-archive)
            CREATE_ARCHIVE=true
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            log_error "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Validate inputs
if [ -z "$SOURCE_DIR" ]; then
    log_error "Source directory is required"
    usage
    exit 1
fi

if [ ! -d "$SOURCE_DIR" ]; then
    log_error "Source directory not found: $SOURCE_DIR"
    exit 1
fi

if [ ! -f "$ENCRYPTION_KEY" ]; then
    log_error "Encryption key not found: $ENCRYPTION_KEY"
    log_info "Generate a key first with: scripts/encrypt_uo_files.py generate-key --output key.bin"
    exit 1
fi

# Create output directory
RELEASE_DIR="$OUTPUT_DIR/$RELEASE_NAME"
mkdir -p "$RELEASE_DIR"

log_info "Building encrypted release..."
log_info "Source: $SOURCE_DIR"
log_info "Output: $RELEASE_DIR"
log_info "Release: $RELEASE_NAME"

# Copy source files to release directory
log_info "Copying source files..."
cp -r "$SOURCE_DIR"/* "$RELEASE_DIR/"

# Encrypt .mul files
log_info "Encrypting .mul files..."
python3 "$SCRIPT_DIR/encrypt_uo_files.py" encrypt-dir \
    "$RELEASE_DIR" \
    --pattern "*.mul" \
    --key "$ENCRYPTION_KEY" \
    --remove-originals

# Encrypt .idx files
log_info "Encrypting .idx files..."
python3 "$SCRIPT_DIR/encrypt_uo_files.py" encrypt-dir \
    "$RELEASE_DIR" \
    --pattern "*.idx" \
    --key "$ENCRYPTION_KEY" \
    --remove-originals

# Optionally encrypt .cfg files
if [ "$INCLUDE_CFG" = true ]; then
    log_info "Encrypting .cfg files..."
    python3 "$SCRIPT_DIR/encrypt_uo_files.py" encrypt-dir \
        "$RELEASE_DIR" \
        --pattern "*.cfg" \
        --key "$ENCRYPTION_KEY" \
        --remove-originals
fi

# Count encrypted files
ENC_COUNT=$(find "$RELEASE_DIR" -name "*.enc" | wc -l)
log_info "Encrypted $ENC_COUNT files"

# Create release manifest
log_info "Creating release manifest..."
cat > "$RELEASE_DIR/RELEASE_INFO.txt" << EOF
Epic-Server Encrypted Release
==============================

Release Name: $RELEASE_NAME
Created: $(date)
Encrypted Files: $ENC_COUNT

Encryption Details:
- Algorithm: AES-256-GCM
- File Format: UOENC v1
- Encrypted Extensions: .mul, .idx$([ "$INCLUDE_CFG" = true ] && echo ", .cfg")

Installation:
1. Copy encrypted files to /opt/uo-data-encrypted/
2. Ensure encryption key is at /opt/uo-encryption/key.bin
3. Or set UO_ENCRYPTION_KEY environment variable
4. Start the server - files will be decrypted automatically

Verify encryption key fingerprint matches:
$(sha256sum "$ENCRYPTION_KEY" | cut -d' ' -f1)

For more information, see docs/ENCRYPTION.md
EOF

# Create installation script
cat > "$RELEASE_DIR/install.sh" << 'INSTALL_EOF'
#!/bin/bash
# Installation script for encrypted UO data

set -e

TARGET_DIR="${1:-/opt/uo-data-encrypted}"

echo "Installing encrypted UO data to $TARGET_DIR"

if [ -d "$TARGET_DIR" ] && [ "$(ls -A $TARGET_DIR)" ]; then
    read -p "Target directory exists and is not empty. Continue? [y/N] " confirm
    if [[ ! $confirm =~ ^[Yy]$ ]]; then
        echo "Installation cancelled"
        exit 0
    fi
fi

mkdir -p "$TARGET_DIR"
cp -v *.enc "$TARGET_DIR/"

echo ""
echo "Installation complete!"
echo "Encrypted files copied to: $TARGET_DIR"
echo ""
echo "Next steps:"
echo "1. Ensure encryption key is at /opt/uo-encryption/key.bin"
echo "2. Or set UO_ENCRYPTION_KEY environment variable"
echo "3. Start the Epic-Server"
echo ""
INSTALL_EOF

chmod +x "$RELEASE_DIR/install.sh"

# Create archive if requested
if [ "$CREATE_ARCHIVE" = true ]; then
    log_info "Creating archive..."
    ARCHIVE_NAME="$OUTPUT_DIR/$RELEASE_NAME.tar.gz"
    tar -czf "$ARCHIVE_NAME" -C "$OUTPUT_DIR" "$RELEASE_NAME"
    log_info "Archive created: $ARCHIVE_NAME"

    # Create checksum
    sha256sum "$ARCHIVE_NAME" > "$ARCHIVE_NAME.sha256"
    log_info "Checksum: $(cat $ARCHIVE_NAME.sha256)"
fi

# Summary
log_info "Build complete!"
echo ""
echo "Release directory: $RELEASE_DIR"
echo "Encrypted files: $ENC_COUNT"
if [ "$CREATE_ARCHIVE" = true ]; then
    echo "Archive: $ARCHIVE_NAME"
fi
echo ""
echo "To distribute this release:"
echo "1. Share the encrypted files from $RELEASE_DIR"
echo "2. Securely share the encryption key separately"
echo "3. Users run: ./install.sh to install"
