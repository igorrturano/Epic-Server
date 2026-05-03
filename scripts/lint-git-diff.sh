#!/bin/bash

# eScript Git Diff Linter - Check only changed lines
# Usage: ./lint-git-diff.sh [commit]
# Default: checks staged changes (git diff --cached)
# With commit: checks changes since that commit (e.g., HEAD, main, origin/main)

# Colors for output
RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Counters
WARNINGS=0
ERRORS=0
OOP_WARNINGS=0
CODE_SMELLS=0

# Define linting rules
check_line() {
    local file="$1"
    local line_num="$2"
    local line="$3"

    # Rule 1: Warn about SendSysMessage usage
    if echo "$line" | grep -q "SendSysMessage\|SendSysMsg"; then
        echo -e "${YELLOW}WARNING${NC} $file:$line_num - Use Message.SendSys() instead of SendSysMessage()"
        echo "  $line"
        ((WARNINGS++))
    fi

    # Rule 2: Warn about direct EraseObjProperty
    if echo "$line" | grep -q "EraseObjProperty"; then
        echo -e "${YELLOW}WARNING${NC} $file:$line_num - Consider using datafile system instead of EraseObjProperty"
        echo "  $line"
        ((WARNINGS++))
    fi

    # Rule 3: Check for potential command injection
    if echo "$line" | grep -E "run_script.*GetInput|Run_Script_To_Completion.*GetInput|start_script.*GetInput" | grep -qv "//"; then
        echo -e "${RED}ERROR${NC} $file:$line_num - Potential command injection: user input directly in script execution"
        echo "  $line"
        ((ERRORS++))
    fi

    # Rule 4: Warn about hardcoded serial numbers
    if echo "$line" | grep -E "serial.*0x[0-9a-fA-F]{8}|== 0x[0-9a-fA-F]{8}" | grep -qv "//"; then
        echo -e "${YELLOW}WARNING${NC} $file:$line_num - Hardcoded serial number detected"
        echo "  $line"
        ((WARNINGS++))
    fi

    # OOP Rule 1: Direct character property access instead of using MobileWrapper
    if echo "$line" | grep -E "\.str[^a-zA-Z]|\.dex[^a-zA-Z]|\.int[^a-zA-Z]|\.hp[^a-zA-Z]|\.maxhp|\.stamina[^a-zA-Z]|\.mana[^a-zA-Z]" | grep -qv "//.*\.str\|//.*\.dex\|//.*\.int"; then
        if ! echo "$line" | grep -qE "GetStr|GetDex|GetInt|GetHP|GetStamina|GetMana|_str|_dex|_int|_hp|_stamina|_mana"; then
            echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct character property access. Consider using MobileWrapper class"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Use MobileWrapper(character) and call GetStr(), GetDex(), GetInt(), GetCurrentHP(), etc."
            ((OOP_WARNINGS++))
        fi
    fi

    # OOP Rule 2: Direct item property modification
    if echo "$line" | grep -E "item\.ar[^a-zA-Z]|item\.damage|item\.maxhp_mod|item\.quality" | grep -qv "//"; then
        if ! echo "$line" | grep -qE "Wrapper|_ar|_damage|_quality"; then
            echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct item property modification. Consider using EquipWrapper classes"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Use EquipWrapperArmor or EquipWrapperWeapon for equipment handling"
            ((OOP_WARNINGS++))
        fi
    fi

    # OOP Rule 3: Procedural function naming
    if echo "$line" | grep -E "function [A-Z][a-z]+_[A-Z][a-z]+\(" | grep -qv "//"; then
        echo -e "${MAGENTA}OOP${NC} $file:$line_num - Procedural function naming detected. Consider using class methods"
        echo "  $line"
        echo -e "  ${BLUE}Tip:${NC} Convert to class methods or use camelCase"
        ((OOP_WARNINGS++))
    fi

    # OOP Rule 4: Direct AP_* function usage
    if echo "$line" | grep -E "AP_SetVital|AP_GetVital|AP_SetTrueSkill|AP_GetTrueSkill" | grep -qv "//"; then
        if ! echo "$file" | grep -q "Wrapper\.inc\|Wrapper\.src"; then
            echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct AP_* function usage. Consider using MobileWrapper methods"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Use MobileWrapper for character operations"
            ((OOP_WARNINGS++))
        fi
    fi

    # Smell 1: Long parameter lists
    if echo "$line" | grep -E "function.*\([^)]*,[^)]*,[^)]*,[^)]*,[^)]*\)" | grep -qv "//"; then
        echo -e "${YELLOW}SMELL${NC} $file:$line_num - Long parameter list (>4 params). Consider using a struct or class"
        echo "  $line"
        ((CODE_SMELLS++))
    fi

    # Smell 2: Magic numbers
    if echo "$line" | grep -E "[^a-zA-Z_][2-9][0-9]+[^0-9]|[^a-zA-Z_][2-9][^0-9]" | grep -qv "//.*[0-9]\|const\|CONST\|#define"; then
        if ! echo "$line" | grep -qE "0x[0-9a-fA-F]+|\[[0-9]+\]|sleep\(|Sleep\(|SleepMS\("; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Magic number detected. Consider using named constants"
            echo "  $line"
            ((CODE_SMELLS++))
        fi
    fi

    # Smell 3: Commented-out code
    if echo "$line" | grep -E "^[[:space:]]*//.*[a-zA-Z_]+\(" | grep -q "//.*:=\|//.*if\|//.*function"; then
        echo -e "${YELLOW}SMELL${NC} $file:$line_num - Commented-out code. Remove or uncomment"
        echo "  $line"
        ((CODE_SMELLS++))
    fi
}

# Main execution
COMMIT="${1:-}"

echo "eScript Git Diff Linter"
echo "================================================"

if [ -z "$COMMIT" ]; then
    echo "Checking staged changes (git diff --cached)"
    DIFF_CMD="git diff --cached -U0"
else
    echo "Checking changes since $COMMIT"
    DIFF_CMD="git diff $COMMIT -U0"
fi

current_file=""
current_line_num=0

# Parse git diff output
$DIFF_CMD -- '*.src' '*.inc' | while IFS= read -r line; do
    # Check for file marker
    if [[ "$line" =~ ^\+\+\+\ b/(.+)$ ]]; then
        current_file="${BASH_REMATCH[1]}"
        continue
    fi

    # Check for line number marker (e.g., @@ -10,5 +12,7 @@)
    if [[ "$line" =~ @@\ -[0-9]+(,[0-9]+)?\ \+([0-9]+)(,[0-9]+)?\ @@ ]]; then
        current_line_num="${BASH_REMATCH[2]}"
        continue
    fi

    # Check only added lines (starting with +)
    if [[ "$line" =~ ^\+(.*)$ ]]; then
        content="${BASH_REMATCH[1]}"

        # Skip the +++ file markers
        if [[ ! "$content" =~ ^\+\+ ]]; then
            check_line "$current_file" "$current_line_num" "$content"
            ((current_line_num++))
        fi
    elif [[ "$line" =~ ^[^-] ]]; then
        # Context line, increment line number
        ((current_line_num++))
    fi
done

echo ""
echo "================================================"
echo -e "Scan complete:"
echo -e "  ${RED}$ERRORS errors${NC}"
echo -e "  ${YELLOW}$WARNINGS warnings${NC}"
echo -e "  ${MAGENTA}$OOP_WARNINGS OOP violations${NC}"
echo -e "  ${YELLOW}$CODE_SMELLS code smells${NC}"
echo ""

if [ $OOP_WARNINGS -gt 0 ] || [ $CODE_SMELLS -gt 0 ]; then
    echo -e "${BLUE}OOP Migration Guide:${NC}"
    echo "  1. Use MobileWrapper for character operations"
    echo "  2. Use EquipWrapper* classes for equipment"
    echo "  3. Replace procedural functions with class methods"
    echo "  4. Encapsulate related data in classes"
    echo ""
fi

if [ $ERRORS -gt 0 ]; then
    exit 1
else
    exit 0
fi
