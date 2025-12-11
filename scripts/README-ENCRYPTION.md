# Encryption Scripts

This directory contains tools for encrypting and managing Ultima Online game files.

## Quick Start

### 1. Install Dependencies
```bash
pip3 install -r requirements-encryption.txt
```

### 2. Run Setup Wizard
```bash
./setup_encryption.sh
```

This interactive wizard will guide you through:
- Generating encryption keys
- Encrypting your UO data files
- Configuring Docker environment
- Creating key backups

## Scripts Overview

### Core Tools

#### `encrypt_uo_files.py`
Python utility for encrypting/decrypting UO files using AES-256-GCM.

**Key Features:**
- AES-256-GCM authenticated encryption
- Streaming support for large files
- Password-derived or random key generation
- Batch directory operations

**Usage:**
```bash
# Generate encryption key
python3 encrypt_uo_files.py generate-key --output key.bin

# Encrypt single file
python3 encrypt_uo_files.py encrypt map0.mul --key key.bin

# Encrypt directory
python3 encrypt_uo_files.py encrypt-dir /path/to/uo-data --key key.bin

# Decrypt directory
python3 encrypt_uo_files.py decrypt-dir /encrypted/path --key key.bin
```

#### `decrypt_uo_data.sh`
Automatic decryption script called during server startup.

**Features:**
- Detects encrypted files
- Verifies critical files after decryption
- Skips if already decrypted
- Supports both key files and environment variables

**Environment Variables:**
- `ENCRYPTED_DIR`: Source directory for .enc files (default: /opt/uo-data-encrypted)
- `DECRYPTED_DIR`: Target directory for decrypted files (default: /opt/uo-data)
- `UO_KEY_FILE`: Path to encryption key (default: /opt/uo-encryption/key.bin)
- `UO_ENCRYPTION_KEY`: Hex-encoded key (alternative to key file)

**Usage:**
```bash
# Auto-decrypt on startup (called by docker-entrypoint.sh)
./decrypt_uo_data.sh

# Manual decryption
export UO_ENCRYPTION_KEY=0123...
./decrypt_uo_data.sh
```

### Setup & Management

#### `setup_encryption.sh`
Interactive wizard for initial encryption setup.

**What it does:**
1. Checks Python dependencies
2. Generates encryption key (random or password-derived)
3. Encrypts your UO data files
4. Creates Docker environment configuration
5. Creates secure key backups

**Usage:**
```bash
./setup_encryption.sh
```

#### `build_encrypted_release.sh`
Build distribution packages with encrypted files.

**Features:**
- Creates encrypted release packages
- Generates installation scripts
- Optional archive creation (tar.gz)
- Creates checksums for verification

**Usage:**
```bash
# Basic release
./build_encrypted_release.sh --source /path/to/uo-data

# Full release with archive
./build_encrypted_release.sh \
    --source /path/to/uo-data \
    --output ./dist \
    --name my-server-v1.0 \
    --create-archive

# Include .cfg files (advanced)
./build_encrypted_release.sh \
    --source /path/to/uo-data \
    --include-cfg \
    --create-archive
```

## Workflow Examples

### Initial Setup (First Time)

```bash
# 1. Install dependencies
pip3 install -r requirements-encryption.txt

# 2. Run setup wizard
./setup_encryption.sh
# Follow prompts to:
#   - Generate key
#   - Encrypt UO files
#   - Setup Docker env
#   - Create backups

# 3. Verify encrypted files
ls -lh /opt/uo-data-encrypted/*.enc

# 4. Test decryption
./decrypt_uo_data.sh

# 5. Start server
cd ..
docker-compose up
```

### Manual Encryption

```bash
# 1. Generate key
python3 encrypt_uo_files.py generate-key --output /opt/uo-encryption/key.bin
chmod 600 /opt/uo-encryption/key.bin

# 2. Encrypt files
python3 encrypt_uo_files.py encrypt-dir \
    /path/to/uo-data \
    --key /opt/uo-encryption/key.bin \
    --remove-originals

# 3. Move encrypted files
mkdir -p /opt/uo-data-encrypted
mv /path/to/uo-data/*.enc /opt/uo-data-encrypted/

# 4. Setup environment
export UO_ENCRYPTION_KEY=$(xxd -p -c 32 /opt/uo-encryption/key.bin)

# 5. Test decryption
./decrypt_uo_data.sh
```

### Creating Distribution Package

```bash
# 1. Build encrypted release
./build_encrypted_release.sh \
    --source /path/to/uo-data \
    --output ./releases \
    --name epic-server-2025-12-11 \
    --create-archive

# 2. Output files
ls -lh releases/
# epic-server-2025-12-11/         (directory with encrypted files)
# epic-server-2025-12-11.tar.gz   (archive)
# epic-server-2025-12-11.tar.gz.sha256  (checksum)

# 3. Distribute
# - Share archive with team
# - Send key separately via secure channel
```

### Updating Encrypted Files

```bash
# 1. Update source files
cp new-map0.mul /path/to/uo-data/

# 2. Re-encrypt changed files
python3 encrypt_uo_files.py encrypt \
    /path/to/uo-data/map0.mul \
    --key /opt/uo-encryption/key.bin

# 3. Copy to encrypted directory
mv /path/to/uo-data/map0.mul.enc /opt/uo-data-encrypted/

# 4. Restart server (will auto-decrypt)
docker-compose restart
```

## Key Management

### Viewing Key

```bash
# View key as hex (for environment variable)
xxd -p -c 32 /opt/uo-encryption/key.bin

# Get key fingerprint (for verification)
sha256sum /opt/uo-encryption/key.bin
```

### Backing Up Key

```bash
# Create backup
mkdir -p ~/encryption-backup
cp /opt/uo-encryption/key.bin ~/encryption-backup/
xxd -p -c 32 /opt/uo-encryption/key.bin > ~/encryption-backup/key.hex

# Secure backup
chmod 600 ~/encryption-backup/*
```

### Restoring Key

```bash
# From binary backup
cp ~/encryption-backup/key.bin /opt/uo-encryption/key.bin
chmod 600 /opt/uo-encryption/key.bin

# From hex backup
xxd -r -p ~/encryption-backup/key.hex > /opt/uo-encryption/key.bin
chmod 600 /opt/uo-encryption/key.bin
```

## Troubleshooting

### "No module named 'Cryptodome'"

```bash
pip3 install pycryptodome
# or
pip3 install -r requirements-encryption.txt
```

### "Permission denied" errors

```bash
# Fix key file permissions
chmod 600 /opt/uo-encryption/key.bin

# Fix script permissions
chmod +x *.sh
```

### "Encryption key not found"

```bash
# Check if key exists
ls -la /opt/uo-encryption/key.bin

# Or set environment variable
export UO_ENCRYPTION_KEY=$(xxd -p -c 32 /path/to/key.bin)
```

### "Authentication failed"

The file may be corrupted or tampered. Try:

```bash
# Verify file is encrypted
xxd -l 10 file.enc  # Should show "UOENC"

# Re-encrypt from original
python3 encrypt_uo_files.py encrypt original.mul --key key.bin
```

## Security Notes

### ⚠️ Important Warnings

1. **Never commit keys to version control**
   ```bash
   # Add to .gitignore
   /opt/uo-encryption/
   *.key
   *.bin
   .env.encryption
   ```

2. **Always backup your encryption key**
   - Without it, encrypted files are permanently unrecoverable
   - Store backups in multiple secure locations

3. **Use secure key distribution**
   - Never email keys in plain text
   - Use encrypted channels (password managers, encrypted email)
   - Consider password-derived keys for team distribution

4. **Set proper file permissions**
   ```bash
   chmod 600 /opt/uo-encryption/key.bin  # Owner read/write only
   chmod 700 /opt/uo-encryption/         # Owner access only
   ```

### ✅ Best Practices

1. **Use different keys for different environments**
   - Development: One key
   - Staging: Different key
   - Production: Different key

2. **Rotate keys periodically**
   - Generate new key
   - Re-encrypt all files
   - Update server configuration

3. **Test decryption regularly**
   ```bash
   ./decrypt_uo_data.sh
   ls -lh /opt/uo-data/  # Verify files decrypted
   ```

4. **Monitor encryption status**
   ```bash
   # Check encrypted file count
   find /opt/uo-data-encrypted -name "*.enc" | wc -l

   # Check file sizes
   du -sh /opt/uo-data-encrypted/
   ```

## Integration with Docker

### Using Environment Variables

Create `.env.encryption`:
```bash
UO_ENCRYPTION_KEY=0123456789abcdef...
ENCRYPTED_DIR=/opt/uo-data-encrypted
DECRYPTED_DIR=/opt/uo-data
```

Update `docker-compose.yml`:
```yaml
services:
  epic-server:
    env_file:
      - .env.encryption
```

### Using Docker Secrets

```yaml
services:
  epic-server:
    secrets:
      - uo_encryption_key

secrets:
  uo_encryption_key:
    file: /opt/uo-encryption/key.bin
```

### Using Volumes

```yaml
services:
  epic-server:
    volumes:
      - ./encrypted-data:/opt/uo-data-encrypted:ro
      - ./encryption-key:/opt/uo-encryption:ro
```

## Support

For detailed documentation, see:
- [Full Encryption Documentation](../docs/ENCRYPTION.md)
- [Epic-Server README](../README.md)

For issues:
1. Check logs: `docker logs epic-server`
2. Test scripts manually
3. Open GitHub issue (DO NOT include encryption keys!)

---

**Last Updated**: 2025-12-11
**Encryption**: AES-256-GCM
**Python**: 3.8+
