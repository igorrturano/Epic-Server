#!/bin/bash

# Git Hooks Installer for eScript Linter
# Usage: ./scripts/install-hooks.sh [pre-commit|pre-push|both|uninstall]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GIT_HOOKS_DIR=".git/hooks"

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Check if we're in a git repository
if [ ! -d ".git" ]; then
    echo -e "${RED}Error: Not in a git repository root!${NC}"
    echo "Please run this from the repository root directory."
    exit 1
fi

install_hook() {
    local hook_type="$1"
    local source_file="$SCRIPT_DIR/${hook_type}.example"
    local dest_file="$GIT_HOOKS_DIR/$hook_type"

    if [ ! -f "$source_file" ]; then
        echo -e "${RED}Error: $source_file not found!${NC}"
        return 1
    fi

    # Backup existing hook if it exists
    if [ -f "$dest_file" ]; then
        echo -e "${YELLOW}Backing up existing $hook_type hook to ${hook_type}.backup${NC}"
        cp "$dest_file" "$dest_file.backup"
    fi

    # Install the hook
    cp "$source_file" "$dest_file"
    chmod +x "$dest_file"

    echo -e "${GREEN}✓ Installed $hook_type hook${NC}"
}

uninstall_hook() {
    local hook_type="$1"
    local hook_file="$GIT_HOOKS_DIR/$hook_type"
    local backup_file="$hook_file.backup"

    if [ -f "$hook_file" ]; then
        rm "$hook_file"
        echo -e "${GREEN}✓ Removed $hook_type hook${NC}"

        # Restore backup if it exists
        if [ -f "$backup_file" ]; then
            echo -e "${YELLOW}Restoring backup...${NC}"
            mv "$backup_file" "$hook_file"
            echo -e "${GREEN}✓ Restored previous $hook_type hook from backup${NC}"
        fi
    else
        echo -e "${YELLOW}No $hook_type hook to remove${NC}"
    fi
}

show_help() {
    echo "Git Hooks Installer for eScript Linter"
    echo ""
    echo "Usage: ./scripts/install-hooks.sh [option]"
    echo ""
    echo "Options:"
    echo "  pre-commit   Install pre-commit hook (checks on commit)"
    echo "  pre-push     Install pre-push hook (checks before push)"
    echo "  both         Install both hooks (recommended)"
    echo "  uninstall    Remove installed hooks (restores backups if available)"
    echo "  help         Show this help message"
    echo ""
    echo "Examples:"
    echo "  ./scripts/install-hooks.sh both"
    echo "  ./scripts/install-hooks.sh pre-commit"
    echo "  ./scripts/install-hooks.sh uninstall"
    echo ""
}

case "${1:-}" in
    pre-commit)
        echo "Installing pre-commit hook..."
        install_hook "pre-commit"
        echo ""
        echo "Pre-commit hook installed! It will run on every commit."
        echo "To bypass: git commit --no-verify"
        ;;
    pre-push)
        echo "Installing pre-push hook..."
        install_hook "pre-push"
        echo ""
        echo "Pre-push hook installed! It will run before every push."
        echo "To bypass: git push --no-verify"
        ;;
    both)
        echo "Installing both pre-commit and pre-push hooks..."
        install_hook "pre-commit"
        install_hook "pre-push"
        echo ""
        echo "Both hooks installed!"
        echo ""
        echo "Pre-commit: Checks staged changes before commit"
        echo "Pre-push: Checks all commits before push to main/master"
        echo ""
        echo "To bypass: Use --no-verify flag with git commit/push"
        ;;
    uninstall)
        echo "Uninstalling git hooks..."
        uninstall_hook "pre-commit"
        uninstall_hook "pre-push"
        echo ""
        echo "Git hooks uninstalled."
        ;;
    help|--help|-h)
        show_help
        ;;
    "")
        echo -e "${YELLOW}No option specified. Installing both hooks...${NC}"
        echo ""
        install_hook "pre-commit"
        install_hook "pre-push"
        echo ""
        echo -e "${GREEN}Both hooks installed successfully!${NC}"
        echo ""
        echo "What happens now:"
        echo "  • git commit - runs linter on staged changes"
        echo "  • git push   - runs linter on commits being pushed (strict on main)"
        echo ""
        echo "To bypass checks: use --no-verify flag"
        ;;
    *)
        echo -e "${RED}Unknown option: $1${NC}"
        echo ""
        show_help
        exit 1
        ;;
esac
