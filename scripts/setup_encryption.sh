#!/bin/bash
#
# Setup UO File Encryption
# Interactive script to set up encryption for Ultima Online game files
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENCRYPTION_DIR="/opt/uo-encryption"
ENCRYPTED_DATA_DIR="/opt/uo-data-encrypted"

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}===================================================${NC}"
    echo -e "${BLUE}  UO File Encryption Setup${NC}"
    echo -e "${BLUE}===================================================${NC}"
    echo ""
}

print_step() {
    echo -e "${GREEN}[STEP]${NC} $1"
}

print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

# Check if Python and required packages are installed
check_requirements() {
    print_step "Checking requirements..."

    if ! command -v python3 &> /dev/null; then
        print_error "Python 3 is not installed"
        exit 1
    fi

    if ! python3 -c "import Cryptodome" 2>/dev/null; then
        print_info "Installing pycryptodome..."
        pip3 install pycryptodome || {
            print_error "Failed to install pycryptodome"
            exit 1
        }
    fi

    print_success "All requirements met"
}

# Generate encryption key
generate_key() {
    print_step "Generating encryption key..."

    echo ""
    echo "Choose key generation method:"
    echo "1) Generate random key (recommended for production)"
    echo "2) Derive from password (easier to remember, less secure)"
    echo ""
    read -p "Enter choice [1-2]: " choice

    case $choice in
        1)
            # Generate random key
            python3 "$SCRIPT_DIR/encrypt_uo_files.py" generate-key \
                --output "$ENCRYPTION_DIR/key.bin"
            ;;
        2)
            # Generate from password
            read -s -p "Enter password: " password
            echo ""
            read -s -p "Confirm password: " password_confirm
            echo ""

            if [ "$password" != "$password_confirm" ]; then
                print_error "Passwords do not match!"
                exit 1
            fi

            python3 "$SCRIPT_DIR/encrypt_uo_files.py" generate-key \
                --password "$password" \
                --output "$ENCRYPTION_DIR/key.bin"
            ;;
        *)
            print_error "Invalid choice"
            exit 1
            ;;
    esac

    # Set proper permissions
    chmod 600 "$ENCRYPTION_DIR/key.bin"

    print_success "Encryption key generated at $ENCRYPTION_DIR/key.bin"
    print_info "IMPORTANT: Back up this key securely! Without it, you cannot decrypt your files."
}

# Encrypt UO data
encrypt_data() {
    print_step "Encrypting UO data files..."

    echo ""
    read -p "Enter path to UO data directory: " uo_data_path

    if [ ! -d "$uo_data_path" ]; then
        print_error "Directory not found: $uo_data_path"
        exit 1
    fi

    # Create encrypted data directory
    mkdir -p "$ENCRYPTED_DATA_DIR"

    # Copy files to encrypted directory first
    print_info "Copying files to encrypted directory..."
    cp -r "$uo_data_path"/* "$ENCRYPTED_DATA_DIR/"

    # Encrypt files
    print_info "Encrypting files..."
    python3 "$SCRIPT_DIR/encrypt_uo_files.py" encrypt-dir \
        "$ENCRYPTED_DATA_DIR" \
        --key "$ENCRYPTION_DIR/key.bin" \
        --remove-originals

    print_success "All files encrypted to $ENCRYPTED_DATA_DIR"

    # Count encrypted files
    enc_count=$(find "$ENCRYPTED_DATA_DIR" -name "*.enc" | wc -l)
    print_info "Encrypted $enc_count files"
}

# Setup Docker environment variables
setup_docker_env() {
    print_step "Setting up Docker environment..."

    ENV_FILE="$PROJECT_ROOT/.env.encryption"

    KEY_HEX=$(xxd -p -c 32 "$ENCRYPTION_DIR/key.bin")

    cat > "$ENV_FILE" << EOF
# UO File Encryption Configuration
# Generated on $(date)

# Encryption key (hex encoded)
UO_ENCRYPTION_KEY=$KEY_HEX

# Encrypted data directory
ENCRYPTED_DIR=/opt/uo-data-encrypted

# Decrypted data directory (temporary)
DECRYPTED_DIR=/opt/uo-data

# Key file location
UO_KEY_FILE=/opt/uo-encryption/key.bin
EOF

    chmod 600 "$ENV_FILE"

    print_success "Docker environment configured at $ENV_FILE"
    print_info "Add this to your docker-compose.yml env_file section:"
    echo "    env_file:"
    echo "      - .env.encryption"
}

# Create backup of encryption key
backup_key() {
    print_step "Creating key backup..."

    BACKUP_DIR="$PROJECT_ROOT/encryption-backup-$(date +%Y%m%d)"
    mkdir -p "$BACKUP_DIR"

    cp "$ENCRYPTION_DIR/key.bin" "$BACKUP_DIR/key.bin"

    # Also save as hex for easy recovery
    xxd -p -c 32 "$ENCRYPTION_DIR/key.bin" > "$BACKUP_DIR/key.hex"

    # Create recovery instructions
    cat > "$BACKUP_DIR/RECOVERY_INSTRUCTIONS.txt" << 'EOF'
ENCRYPTION KEY RECOVERY INSTRUCTIONS
====================================

This directory contains your UO file encryption key backup.

Files:
- key.bin: Binary encryption key (32 bytes)
- key.hex: Same key in hexadecimal format
- RECOVERY_INSTRUCTIONS.txt: This file

To restore encryption key:
1. Copy key.bin to /opt/uo-encryption/key.bin on your server
2. Set proper permissions: chmod 600 /opt/uo-encryption/key.bin
3. Restart the server

To use hex key as environment variable:
1. Copy the content of key.hex
2. Set environment variable: export UO_ENCRYPTION_KEY=<hex_value>
3. Restart the server

IMPORTANT:
- Keep this backup in a secure location
- Do not commit this to version control
- Without this key, encrypted files cannot be decrypted
EOF

    print_success "Backup created at $BACKUP_DIR"
    print_info "Store this backup in a secure location (USB drive, password manager, etc.)"
}

# Main setup wizard
main() {
    print_header

    # Create encryption directory
    mkdir -p "$ENCRYPTION_DIR"

    # Check if key already exists
    if [ -f "$ENCRYPTION_DIR/key.bin" ]; then
        print_info "Encryption key already exists at $ENCRYPTION_DIR/key.bin"
        echo ""
        read -p "Do you want to generate a new key? This will make old encrypted files unreadable! [y/N]: " replace

        if [[ ! $replace =~ ^[Yy]$ ]]; then
            print_info "Using existing key"
        else
            check_requirements
            generate_key
        fi
    else
        check_requirements
        generate_key
    fi

    echo ""
    read -p "Do you want to encrypt UO data files now? [Y/n]: " encrypt_now

    if [[ ! $encrypt_now =~ ^[Nn]$ ]]; then
        encrypt_data
    fi

    echo ""
    read -p "Do you want to setup Docker environment? [Y/n]: " setup_env

    if [[ ! $setup_env =~ ^[Nn]$ ]]; then
        setup_docker_env
    fi

    echo ""
    read -p "Do you want to create a key backup? [Y/n]: " create_backup

    if [[ ! $create_backup =~ ^[Nn]$ ]]; then
        backup_key
    fi

    echo ""
    print_success "Setup complete!"
    echo ""
    echo "Next steps:"
    echo "1. Back up your encryption key from: $ENCRYPTION_DIR/key.bin"
    echo "2. Update your Docker configuration to use encrypted files"
    echo "3. Test decryption with: ./scripts/decrypt_uo_data.sh"
    echo ""
    echo "For more information, see: docs/ENCRYPTION.md"
}

main "$@"
