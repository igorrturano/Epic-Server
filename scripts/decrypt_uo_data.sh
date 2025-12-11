#!/bin/bash
#
# Decrypt UO Data Files
# This script decrypts encrypted .mul files before server startup.
# Called automatically by docker-entrypoint.sh
#

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENCRYPTED_DIR="${ENCRYPTED_DIR:-/opt/uo-data-encrypted}"
DECRYPTED_DIR="${DECRYPTED_DIR:-/opt/uo-data}"
KEY_FILE="${UO_KEY_FILE:-/opt/uo-encryption/key.bin}"
PYTHON_SCRIPT="$SCRIPT_DIR/encrypt_uo_files.py"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if encryption is enabled
if [ ! -d "$ENCRYPTED_DIR" ] && [ -z "$UO_ENCRYPTION_KEY" ]; then
    log_info "Encryption not enabled, skipping decryption"
    exit 0
fi

# Verify key exists
if [ ! -f "$KEY_FILE" ] && [ -z "$UO_ENCRYPTION_KEY" ]; then
    log_error "Encryption key not found at $KEY_FILE and UO_ENCRYPTION_KEY not set"
    log_error "Cannot decrypt game files!"
    exit 1
fi

log_info "Starting UO file decryption..."
log_info "Encrypted dir: $ENCRYPTED_DIR"
log_info "Decrypted dir: $DECRYPTED_DIR"

# Create decrypted directory if it doesn't exist
mkdir -p "$DECRYPTED_DIR"

# Check if we need to decrypt
NEEDS_DECRYPT=false

if [ -d "$ENCRYPTED_DIR" ]; then
    # Check if encrypted files are newer than decrypted
    if [ -n "$(find "$ENCRYPTED_DIR" -name '*.enc' -newer "$DECRYPTED_DIR" 2>/dev/null)" ]; then
        NEEDS_DECRYPT=true
    elif [ -z "$(ls -A "$DECRYPTED_DIR" 2>/dev/null)" ]; then
        # Decrypted dir is empty
        NEEDS_DECRYPT=true
    fi
fi

if [ "$NEEDS_DECRYPT" = false ] && [ -n "$(ls -A "$DECRYPTED_DIR" 2>/dev/null)" ]; then
    log_info "Decrypted files are up to date, skipping decryption"
    exit 0
fi

# Perform decryption
log_info "Decrypting UO game files..."

# Set key environment variable if using keyfile
if [ -f "$KEY_FILE" ]; then
    export UO_ENCRYPTION_KEY=$(xxd -p -c 32 "$KEY_FILE")
fi

# Decrypt all .enc files
if ! python3 "$PYTHON_SCRIPT" decrypt-dir "$ENCRYPTED_DIR" --pattern "*.enc"; then
    log_error "Decryption failed!"
    exit 1
fi

# Move decrypted files to correct location
log_info "Moving decrypted files to $DECRYPTED_DIR..."
find "$ENCRYPTED_DIR" -type f ! -name "*.enc" -exec mv {} "$DECRYPTED_DIR/" \;

# Count decrypted files
FILE_COUNT=$(find "$DECRYPTED_DIR" -type f | wc -l)
log_info "Successfully decrypted $FILE_COUNT files"

# Verify critical files exist
CRITICAL_FILES=(
    "map0.mul"
    "statics0.mul"
    "tiledata.mul"
    "multi.mul"
)

MISSING_FILES=()
for file in "${CRITICAL_FILES[@]}"; do
    if [ ! -f "$DECRYPTED_DIR/$file" ]; then
        MISSING_FILES+=("$file")
    fi
done

if [ ${#MISSING_FILES[@]} -gt 0 ]; then
    log_warn "Warning: Some critical files are missing:"
    for file in "${MISSING_FILES[@]}"; do
        log_warn "  - $file"
    done
fi

log_info "Decryption complete!"
