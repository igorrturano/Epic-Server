# UO File Encryption Documentation

## Overview

Epic-Server supports **AES-256-GCM encryption** for Ultima Online game files (.mul, .idx, and other asset files). This provides strong protection for your custom content while maintaining transparent server-side decryption.

### What This Protects

✅ **Protected:**
- Unauthorized servers copying your custom .mul files
- File theft from your server storage
- Distribution of your custom maps, tiles, and assets
- Tampering detection through authenticated encryption

❌ **Not Protected:**
- Client-side data (requires custom client modifications)
- Network traffic (handled separately by UO encryption)
- Real-time game data in server memory

---

## Architecture

### Encryption Flow

```
┌─────────────────────┐
│  Source .mul Files  │
│  (Unencrypted)      │
└──────────┬──────────┘
           │
           │ encrypt_uo_files.py
           ▼
┌─────────────────────┐
│  Encrypted Storage  │
│  (.enc files)       │
└──────────┬──────────┘
           │
           │ Server Startup
           │ decrypt_uo_data.sh
           ▼
┌─────────────────────┐
│  Temporary Decrypt  │
│  (/opt/uo-data)     │
└──────────┬──────────┘
           │
           │ uoconvert
           ▼
┌─────────────────────┐
│  Config Files       │
│  (.cfg)             │
└──────────┬──────────┘
           │
           │ POL Runtime
           ▼
┌─────────────────────┐
│  Server Memory      │
└─────────────────────┘
```

### Key Components

1. **encrypt_uo_files.py** - Core encryption/decryption utility
   - AES-256-GCM encryption
   - Streaming for large files
   - Authentication to detect tampering

2. **decrypt_uo_data.sh** - Automatic decryption on startup
   - Runs before server initialization
   - Decrypts to temporary directory
   - Verifies critical files exist

3. **docker-entrypoint.sh** - Server startup integration
   - Checks for encrypted data
   - Calls decryption script
   - Continues normal boot process

4. **Key Management** - Secure key storage
   - Binary key file (/opt/uo-encryption/key.bin)
   - Environment variable (UO_ENCRYPTION_KEY)
   - Password-derived keys (optional)

---

## Quick Start

### 1. Initial Setup

Run the interactive setup wizard:

```bash
./scripts/setup_encryption.sh
```

This will:
- Generate an encryption key
- Encrypt your UO data files
- Configure Docker environment
- Create key backups

### 2. Manual Setup

#### Step 1: Generate Encryption Key

**Option A: Random Key (Recommended)**
```bash
python3 scripts/encrypt_uo_files.py generate-key --output /opt/uo-encryption/key.bin
chmod 600 /opt/uo-encryption/key.bin
```

**Option B: Password-Derived Key**
```bash
python3 scripts/encrypt_uo_files.py generate-key \
    --password "YourStrongPassword" \
    --output /opt/uo-encryption/key.bin
```

#### Step 2: Encrypt Your UO Files

```bash
# Encrypt entire directory
python3 scripts/encrypt_uo_files.py encrypt-dir \
    /path/to/uo-data \
    --key /opt/uo-encryption/key.bin \
    --remove-originals

# Encrypt single file
python3 scripts/encrypt_uo_files.py encrypt \
    map0.mul \
    --key /opt/uo-encryption/key.bin
```

#### Step 3: Configure Server

**Option A: Using Key File**
```bash
# Move encrypted files
mv /path/to/uo-data/*.enc /opt/uo-data-encrypted/

# Server will automatically detect and decrypt
docker-compose up
```

**Option B: Using Environment Variable**
```bash
# Export key as environment variable
export UO_ENCRYPTION_KEY=$(xxd -p -c 32 /opt/uo-encryption/key.bin)

# Add to docker-compose.yml
environment:
  - UO_ENCRYPTION_KEY=${UO_ENCRYPTION_KEY}
```

---

## File Format Specification

### Encrypted File Structure

```
┌─────────────────────────────────────┐
│ Magic Header (5 bytes)              │  'UOENC'
├─────────────────────────────────────┤
│ Version (1 byte)                    │  0x01
├─────────────────────────────────────┤
│ Nonce (16 bytes)                    │  Random GCM nonce
├─────────────────────────────────────┤
│ Authentication Tag (16 bytes)       │  GCM tag
├─────────────────────────────────────┤
│ Encrypted Data (variable)           │  AES-256-GCM encrypted
└─────────────────────────────────────┘
```

### Encryption Details

- **Algorithm**: AES-256-GCM (Galois/Counter Mode)
- **Key Size**: 256 bits (32 bytes)
- **Nonce**: 128 bits (16 bytes, unique per file)
- **Authentication**: Built-in GCM authentication tag
- **Streaming**: Processes files in 64KB chunks

### Why AES-GCM?

1. **Authenticated Encryption** - Detects tampering
2. **Industry Standard** - NIST approved, widely audited
3. **Performance** - Hardware acceleration on modern CPUs
4. **Security** - No known practical attacks

---

## Usage Examples

### Encrypting Files

```bash
# Encrypt all .mul files in directory
python3 scripts/encrypt_uo_files.py encrypt-dir /uo-data --pattern "*.mul"

# Encrypt specific file types
python3 scripts/encrypt_uo_files.py encrypt-dir /uo-data --pattern "map*.mul"

# Keep original files
python3 scripts/encrypt_uo_files.py encrypt map0.mul
# Creates map0.mul.enc, keeps map0.mul
```

### Decrypting Files

```bash
# Decrypt entire directory
python3 scripts/encrypt_uo_files.py decrypt-dir /opt/uo-data-encrypted

# Decrypt single file
python3 scripts/encrypt_uo_files.py decrypt map0.mul.enc

# Specify output location
python3 scripts/encrypt_uo_files.py decrypt map0.mul.enc --output /tmp/map0.mul
```

### Key Management

```bash
# View key as hex (for environment variable)
xxd -p -c 32 /opt/uo-encryption/key.bin

# Create key from password
python3 scripts/encrypt_uo_files.py generate-key \
    --password "MyPassword123" \
    --output key.bin

# Verify key fingerprint
sha256sum /opt/uo-encryption/key.bin
```

---

## Advanced Configuration

### Docker Compose Integration

**Method 1: Environment File**

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
    volumes:
      - ./encrypted-data:/opt/uo-data-encrypted:ro
      - ./encryption-key:/opt/uo-encryption:ro
```

**Method 2: Secrets (Recommended for Production)**

```yaml
services:
  epic-server:
    secrets:
      - uo_encryption_key
    volumes:
      - ./encrypted-data:/opt/uo-data-encrypted:ro

secrets:
  uo_encryption_key:
    file: ./uo-encryption/key.bin
```

### Custom Decryption Paths

Set environment variables:
```bash
export ENCRYPTED_DIR=/custom/encrypted/path
export DECRYPTED_DIR=/custom/decrypted/path
export UO_KEY_FILE=/custom/key/path/key.bin

./scripts/decrypt_uo_data.sh
```

### Selective Encryption

Encrypt only custom files, leave standard UO files unencrypted:

```bash
# Encrypt only custom maps
python3 scripts/encrypt_uo_files.py encrypt mapdif0.mul
python3 scripts/encrypt_uo_files.py encrypt stadif0.mul

# Mix encrypted and unencrypted in same directory
/opt/uo-data-encrypted/
  ├── map0.mul         # Standard, unencrypted
  ├── mapdif0.mul.enc  # Custom, encrypted
  ├── statics0.mul     # Standard, unencrypted
  └── stadif0.mul.enc  # Custom, encrypted
```

---

## Building Encrypted Releases

### Create Distribution Package

```bash
# Build encrypted release
./scripts/build_encrypted_release.sh \
    --source /path/to/uo-data \
    --output ./dist \
    --name epic-server-v1.0 \
    --create-archive
```

This creates:
```
dist/epic-server-v1.0/
  ├── *.enc               # Encrypted files
  ├── RELEASE_INFO.txt    # Release metadata
  └── install.sh          # Installation script

dist/epic-server-v1.0.tar.gz        # Archive
dist/epic-server-v1.0.tar.gz.sha256 # Checksum
```

### Distribution Best Practices

1. **Separate Key Distribution**
   - Never include key in same package
   - Use secure channel (password manager, encrypted email)
   - Consider password-derived keys for easier sharing

2. **Verify Integrity**
   ```bash
   # Check archive integrity
   sha256sum -c epic-server-v1.0.tar.gz.sha256

   # Verify key fingerprint matches release info
   sha256sum /opt/uo-encryption/key.bin
   ```

3. **Installation**
   ```bash
   # Extract archive
   tar -xzf epic-server-v1.0.tar.gz
   cd epic-server-v1.0

   # Install encrypted files
   ./install.sh /opt/uo-data-encrypted

   # Copy key separately
   cp key.bin /opt/uo-encryption/key.bin
   chmod 600 /opt/uo-encryption/key.bin
   ```

---

## Security Considerations

### ✅ Best Practices

1. **Key Protection**
   - Store key file with `chmod 600` permissions
   - Never commit keys to version control
   - Use different keys for dev/staging/production
   - Rotate keys periodically

2. **Access Control**
   - Limit who has access to encryption keys
   - Use Docker secrets in production
   - Audit key access regularly

3. **Backup Strategy**
   - Keep encrypted backups of key files
   - Store backups in multiple secure locations
   - Test key recovery procedure

4. **Distribution**
   - Share keys through separate secure channel
   - Use password-derived keys for team distribution
   - Implement key expiration for time-limited access

### ⚠️ Common Pitfalls

1. **Don't encrypt .cfg files unless necessary**
   - .cfg files are human-readable configs
   - Encrypting them prevents manual editing
   - Only encrypt if they contain sensitive custom data

2. **Don't lose your key!**
   - Encrypted files are **permanently unrecoverable** without key
   - Create multiple backups immediately
   - Document key storage location

3. **Don't reuse keys across environments**
   - Use separate keys for development, staging, production
   - Compromised dev key won't affect production

4. **Don't store keys in source control**
   - Add `/opt/uo-encryption/` to .gitignore
   - Add `*.key`, `*.bin` to .gitignore
   - Use environment variables or Docker secrets

---

## Troubleshooting

### Decryption Fails

**Error: "Not an encrypted UO file"**
```bash
# Check if file is actually encrypted
xxd -l 10 file.enc
# Should show: 554f 454e 43 01... (UOENC in hex)

# Try decrypting manually
python3 scripts/encrypt_uo_files.py decrypt file.enc --key key.bin
```

**Error: "Authentication failed"**
- File may be corrupted or tampered
- Wrong encryption key
- File was modified after encryption

```bash
# Verify key fingerprint
sha256sum /opt/uo-encryption/key.bin

# Re-encrypt from original
python3 scripts/encrypt_uo_files.py encrypt original.mul --key key.bin
```

### Missing Files

**Error: "Critical files missing after decryption"**
```bash
# Check encrypted directory
ls -la /opt/uo-data-encrypted/

# Manually decrypt to see what's there
./scripts/decrypt_uo_data.sh

# Verify which files decrypted
ls -la /opt/uo-data/
```

### Key Not Found

**Error: "Encryption key not found"**
```bash
# Check key file exists
ls -la /opt/uo-encryption/key.bin

# Check permissions
chmod 600 /opt/uo-encryption/key.bin

# Or use environment variable
export UO_ENCRYPTION_KEY=$(xxd -p -c 32 /path/to/key.bin)
```

### Performance Issues

**Slow decryption on large files**
- Normal for first startup (files are large)
- Subsequent startups skip if files unchanged
- Consider using faster storage (SSD)

```bash
# Monitor decryption progress
tail -f /var/log/epic-server/startup.log

# Time the decryption
time ./scripts/decrypt_uo_data.sh
```

---

## Expert Recommendations

Based on industry best practices for game server file protection:

### ✅ Recommended Approach

1. **Encrypt at Rest**
   - Store files encrypted on disk
   - Decrypt transparently on server startup
   - Keep decrypted files in memory/tmpfs (optional)

2. **Multi-Layer Protection**
   ```
   Layer 1: File encryption (this implementation)
   Layer 2: Network encryption (UO client encryption)
   Layer 3: Access controls (file permissions, firewalls)
   Layer 4: Distribution control (license keys, authentication)
   ```

3. **Custom Content Only**
   - Encrypt your custom modifications (maps, tiles, patches)
   - Leave standard UO files unencrypted
   - Reduces attack surface and improves performance

4. **Client-Side Protection** (Advanced)
   - For maximum security, modify UO client
   - Load encrypted files directly into memory
   - Decrypt on-the-fly during rendering
   - Prevents extraction even with memory dumping

### 📚 Further Reading

- [NIST AES-GCM Guidelines](https://csrc.nist.gov/publications/detail/sp/800-38d/final)
- [OWASP Cryptographic Storage](https://cheatsheetseries.owasp.org/cheatsheets/Cryptographic_Storage_Cheat_Sheet.html)
- [Game Server Security Best Practices](https://www.gamedev.net/articles/programming/general-and-gameplay-programming/game-server-security-r4465/)

---

## FAQ

**Q: Can players extract my custom content?**
A: This encryption protects server-side files. Determined players could still extract data from client memory during gameplay. For complete protection, you'd need custom client modifications.

**Q: What if I lose my encryption key?**
A: Encrypted files are **permanently unrecoverable**. Always maintain secure backups of your key.

**Q: Does this slow down the server?**
A: Decryption happens once at startup. Runtime performance is identical to unencrypted setups.

**Q: Can I use this with existing servers?**
A: Yes! The encryption is transparent. Just encrypt your files and ensure the key is available at startup.

**Q: Should I encrypt .cfg files?**
A: Generally no. .cfg files are converted from .mul files and don't contain additional secrets. Encrypt the source .mul files instead.

**Q: How is this better than XOR encryption?**
A: XOR encryption is easily broken (especially with known-plaintext attacks on standardized .mul formats). AES-256-GCM provides military-grade security with authentication.

**Q: Can I change the encryption key later?**
A: Yes, but you must re-encrypt all files with the new key. Keep the old key until migration is complete.

**Q: Does this work with POL's encryption?**
A: Yes! This encrypts file storage. POL's encryption handles network communication. They work together.

---

## Support

For issues or questions:
1. Check this documentation
2. Review troubleshooting section
3. Check server logs in `/opt/pol/logs/`
4. Open an issue on GitHub with:
   - Error messages
   - Steps to reproduce
   - Environment details (DO NOT include your encryption key!)

---

**Version**: 1.0
**Last Updated**: 2025-12-11
**Encryption**: AES-256-GCM
**Compatibility**: POL 100+, Epic-Server all versions
