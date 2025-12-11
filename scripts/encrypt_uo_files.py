#!/usr/bin/env python3
"""
UO File Encryption/Decryption Utility
Encrypts Ultima Online .mul and asset files using AES-256-GCM for secure storage.
Provides transparent decryption during server startup.
"""

import os
import sys
import argparse
import hashlib
import secrets
from pathlib import Path
from typing import List, Tuple
from Cryptodome.Cipher import AES
from Cryptodome.Random import get_random_bytes
from Cryptodome.Protocol.KDF import PBKDF2


# File extensions to encrypt
ENCRYPT_EXTENSIONS = [
    '.mul', '.idx', '.uop', '.def',  # UO binary files
    '.cfg',  # Converted config files (optional)
]

# Files that should always be encrypted
CRITICAL_FILES = [
    'map*.mul', 'statics*.mul', 'staidx*.mul',
    'mapdif*.mul', 'stadif*.mul',
    'tiledata.mul', 'multi.mul', 'multi.idx',
    'anim.mul', 'anim.idx', 'animdata.mul',
    'hues.mul', 'verdata.mul', 'radarcol.mul',
]


class UOFileEncryptor:
    """Handles encryption and decryption of UO game files."""

    MAGIC = b'UOENC'  # File magic to identify encrypted files
    VERSION = 1  # Encryption format version
    CHUNK_SIZE = 64 * 1024  # 64KB chunks for streaming large files

    def __init__(self, key: bytes = None):
        """
        Initialize encryptor with AES-256 key.

        Args:
            key: 32-byte AES key. If None, will load from environment or keyfile.
        """
        if key is None:
            key = self._load_key()

        if len(key) != 32:
            raise ValueError("AES-256 key must be exactly 32 bytes")

        self.key = key

    def _load_key(self) -> bytes:
        """Load encryption key from environment or keyfile."""
        # Try environment variable first
        key_env = os.getenv('UO_ENCRYPTION_KEY')
        if key_env:
            return bytes.fromhex(key_env)

        # Try keyfile
        keyfile = Path('/opt/uo-encryption/key.bin')
        if keyfile.exists():
            with open(keyfile, 'rb') as f:
                return f.read(32)

        raise ValueError(
            "No encryption key found. Set UO_ENCRYPTION_KEY environment variable "
            "or create /opt/uo-encryption/key.bin"
        )

    @staticmethod
    def generate_key(password: str = None) -> bytes:
        """
        Generate a secure AES-256 key.

        Args:
            password: Optional password to derive key from. If None, generates random key.

        Returns:
            32-byte AES-256 key
        """
        if password:
            # Derive key from password using PBKDF2
            salt = b'UO-Epic-Server-Salt-v1'  # Static salt for reproducibility
            return PBKDF2(password, salt, dkLen=32, count=100000)
        else:
            # Generate cryptographically secure random key
            return get_random_bytes(32)

    def encrypt_file(self, input_path: Path, output_path: Path = None) -> Path:
        """
        Encrypt a file using AES-256-GCM.

        File format:
        - Magic (5 bytes): 'UOENC'
        - Version (1 byte): 0x01
        - Nonce (16 bytes): Random nonce for GCM
        - Tag (16 bytes): Authentication tag
        - Data (variable): Encrypted file content

        Args:
            input_path: Path to file to encrypt
            output_path: Path for encrypted output (default: input_path.enc)

        Returns:
            Path to encrypted file
        """
        if output_path is None:
            output_path = Path(str(input_path) + '.enc')

        # Generate random nonce for GCM
        nonce = get_random_bytes(16)

        # Create AES-GCM cipher
        cipher = AES.new(self.key, AES.MODE_GCM, nonce=nonce)

        # Write header
        with open(output_path, 'wb') as out_file:
            out_file.write(self.MAGIC)
            out_file.write(bytes([self.VERSION]))
            out_file.write(nonce)

            # Reserve space for tag (will write after encryption)
            tag_pos = out_file.tell()
            out_file.write(b'\x00' * 16)

            # Encrypt file in chunks
            with open(input_path, 'rb') as in_file:
                while True:
                    chunk = in_file.read(self.CHUNK_SIZE)
                    if not chunk:
                        break
                    encrypted_chunk = cipher.encrypt(chunk)
                    out_file.write(encrypted_chunk)

            # Write authentication tag
            tag = cipher.digest()
            out_file.seek(tag_pos)
            out_file.write(tag)

        return output_path

    def decrypt_file(self, input_path: Path, output_path: Path = None) -> Path:
        """
        Decrypt an encrypted UO file.

        Args:
            input_path: Path to encrypted file (.enc)
            output_path: Path for decrypted output (default: remove .enc extension)

        Returns:
            Path to decrypted file

        Raises:
            ValueError: If file is not encrypted or authentication fails
        """
        if output_path is None:
            output_path = Path(str(input_path).replace('.enc', ''))

        with open(input_path, 'rb') as in_file:
            # Read and verify header
            magic = in_file.read(5)
            if magic != self.MAGIC:
                raise ValueError(f"Not an encrypted UO file: {input_path}")

            version = in_file.read(1)[0]
            if version != self.VERSION:
                raise ValueError(f"Unsupported encryption version: {version}")

            # Read nonce and tag
            nonce = in_file.read(16)
            tag = in_file.read(16)

            # Create cipher for decryption
            cipher = AES.new(self.key, AES.MODE_GCM, nonce=nonce)

            # Decrypt file in chunks
            with open(output_path, 'wb') as out_file:
                while True:
                    chunk = in_file.read(self.CHUNK_SIZE)
                    if not chunk:
                        break
                    decrypted_chunk = cipher.decrypt(chunk)
                    out_file.write(decrypted_chunk)

            # Verify authentication tag
            try:
                cipher.verify(tag)
            except ValueError:
                # Authentication failed - delete corrupted output
                output_path.unlink()
                raise ValueError(f"Authentication failed: {input_path} may be corrupted or tampered")

        return output_path

    def is_encrypted(self, file_path: Path) -> bool:
        """Check if a file is encrypted."""
        try:
            with open(file_path, 'rb') as f:
                magic = f.read(5)
                return magic == self.MAGIC
        except Exception:
            return False

    def encrypt_directory(self, directory: Path, pattern: str = '*', recursive: bool = True) -> List[Path]:
        """
        Encrypt all matching files in a directory.

        Args:
            directory: Directory to search
            pattern: Glob pattern for files to encrypt
            recursive: Search subdirectories

        Returns:
            List of encrypted file paths
        """
        encrypted_files = []

        if recursive:
            files = directory.rglob(pattern)
        else:
            files = directory.glob(pattern)

        for file_path in files:
            if not file_path.is_file():
                continue

            # Skip already encrypted files
            if file_path.suffix == '.enc':
                continue

            # Only encrypt specified extensions
            if file_path.suffix.lower() not in ENCRYPT_EXTENSIONS:
                continue

            print(f"Encrypting: {file_path}")
            encrypted_path = self.encrypt_file(file_path)
            encrypted_files.append(encrypted_path)

        return encrypted_files

    def decrypt_directory(self, directory: Path, pattern: str = '*.enc', recursive: bool = True) -> List[Path]:
        """
        Decrypt all .enc files in a directory.

        Args:
            directory: Directory to search
            pattern: Glob pattern for files to decrypt
            recursive: Search subdirectories

        Returns:
            List of decrypted file paths
        """
        decrypted_files = []

        if recursive:
            files = directory.rglob(pattern)
        else:
            files = directory.glob(pattern)

        for file_path in files:
            if not file_path.is_file():
                continue

            print(f"Decrypting: {file_path}")
            try:
                decrypted_path = self.decrypt_file(file_path)
                decrypted_files.append(decrypted_path)
            except Exception as e:
                print(f"ERROR: Failed to decrypt {file_path}: {e}", file=sys.stderr)

        return decrypted_files


def main():
    """Command-line interface for encryption utility."""
    parser = argparse.ArgumentParser(
        description='Encrypt/Decrypt Ultima Online game files',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate a new encryption key
  %(prog)s generate-key --output key.bin

  # Generate key from password
  %(prog)s generate-key --password "MySecretPassword" --output key.bin

  # Encrypt a single file
  %(prog)s encrypt map0.mul --key key.bin

  # Encrypt all .mul files in directory
  %(prog)s encrypt-dir /path/to/uo-data --key key.bin

  # Decrypt all .enc files (for server startup)
  %(prog)s decrypt-dir /opt/uo-data --key key.bin

  # Decrypt using environment variable
  export UO_ENCRYPTION_KEY=0123456789abcdef...
  %(prog)s decrypt map0.mul.enc
        """
    )

    subparsers = parser.add_subparsers(dest='command', help='Command to execute')

    # Generate key command
    key_parser = subparsers.add_parser('generate-key', help='Generate encryption key')
    key_parser.add_argument('--password', help='Password to derive key from')
    key_parser.add_argument('--output', required=True, help='Output file for key')

    # Encrypt file command
    enc_parser = subparsers.add_parser('encrypt', help='Encrypt a file')
    enc_parser.add_argument('input', type=Path, help='File to encrypt')
    enc_parser.add_argument('--output', type=Path, help='Output path (default: input.enc)')
    enc_parser.add_argument('--key', type=Path, help='Key file path')
    enc_parser.add_argument('--remove-original', action='store_true', help='Delete original after encryption')

    # Decrypt file command
    dec_parser = subparsers.add_parser('decrypt', help='Decrypt a file')
    dec_parser.add_argument('input', type=Path, help='File to decrypt')
    dec_parser.add_argument('--output', type=Path, help='Output path (default: remove .enc)')
    dec_parser.add_argument('--key', type=Path, help='Key file path')

    # Encrypt directory command
    enc_dir_parser = subparsers.add_parser('encrypt-dir', help='Encrypt directory')
    enc_dir_parser.add_argument('directory', type=Path, help='Directory to encrypt')
    enc_dir_parser.add_argument('--pattern', default='*', help='File pattern (default: *)')
    enc_dir_parser.add_argument('--key', type=Path, help='Key file path')
    enc_dir_parser.add_argument('--remove-originals', action='store_true', help='Delete originals after encryption')

    # Decrypt directory command
    dec_dir_parser = subparsers.add_parser('decrypt-dir', help='Decrypt directory')
    dec_dir_parser.add_argument('directory', type=Path, help='Directory to decrypt')
    dec_dir_parser.add_argument('--pattern', default='*.enc', help='File pattern (default: *.enc)')
    dec_dir_parser.add_argument('--key', type=Path, help='Key file path')

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 1

    # Generate key
    if args.command == 'generate-key':
        key = UOFileEncryptor.generate_key(args.password)
        with open(args.output, 'wb') as f:
            f.write(key)
        print(f"Generated encryption key: {args.output}")
        print(f"Key (hex): {key.hex()}")
        print("\nIMPORTANT: Keep this key secure! You'll need it to decrypt your files.")
        return 0

    # Load key for encryption/decryption
    key = None
    if args.key:
        with open(args.key, 'rb') as f:
            key = f.read(32)

    encryptor = UOFileEncryptor(key)

    # Encrypt file
    if args.command == 'encrypt':
        output = encryptor.encrypt_file(args.input, args.output)
        print(f"Encrypted: {args.input} -> {output}")
        if args.remove_original:
            args.input.unlink()
            print(f"Removed original: {args.input}")
        return 0

    # Decrypt file
    if args.command == 'decrypt':
        output = encryptor.decrypt_file(args.input, args.output)
        print(f"Decrypted: {args.input} -> {output}")
        return 0

    # Encrypt directory
    if args.command == 'encrypt-dir':
        files = encryptor.encrypt_directory(args.directory, args.pattern)
        print(f"\nEncrypted {len(files)} files")
        if args.remove_originals:
            for enc_file in files:
                orig_file = Path(str(enc_file).replace('.enc', ''))
                if orig_file.exists():
                    orig_file.unlink()
            print(f"Removed {len(files)} original files")
        return 0

    # Decrypt directory
    if args.command == 'decrypt-dir':
        files = encryptor.decrypt_directory(args.directory, args.pattern)
        print(f"\nDecrypted {len(files)} files")
        return 0

    return 1


if __name__ == '__main__':
    sys.exit(main())
