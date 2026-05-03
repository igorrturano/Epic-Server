#!/bin/bash

# eScript Linter - Simple code quality checker for POL eScript
# Usage: ./lint-escript.sh [file|directory]
# If no argument provided, lints entire pkg/ directory

# Don't use set -e because grep -q can return non-zero and cause early exit

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

# Function to check a single file
lint_file() {
    local file="$1"
    local line_num=0

    # Skip compiled files
    if [[ "$file" == *.ecl ]] || [[ "$file" == *.dep ]]; then
        return
    fi

    while IFS= read -r line; do
        ((line_num++))

        # Rule 1: Warn about SendSysMessage usage (should use Message.SendSys instead)
        if echo "$line" | grep -q "SendSysMessage\|SendSysMsg"; then
            echo -e "${YELLOW}WARNING${NC} $file:$line_num - Use Message.SendSys() instead of SendSysMessage()"
            echo "  $line"
            ((WARNINGS++))
        fi

        # Rule 2: Warn about direct EraseObjProperty (should use datafile system for persistence)
        if echo "$line" | grep -q "EraseObjProperty"; then
            echo -e "${YELLOW}WARNING${NC} $file:$line_num - Consider using datafile system instead of EraseObjProperty for persistent data"
            echo "  $line"
            ((WARNINGS++))
        fi

        # Rule 3: Check for common security issues - direct user input in system calls
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

        # Rule 5: Check for missing error handling on critical operations
        if echo "$line" | grep -E "CreateItemAtLocation|CreateItemInBackpack|CreateNPC" | grep -qv "error"; then
            # Check if next few lines have error handling
            local has_error_check=0
            local check_line=$line_num
            for i in {1..3}; do
                ((check_line++))
                local next_line=$(sed -n "${check_line}p" "$file")
                if echo "$next_line" | grep -q "error\|\.errortext"; then
                    has_error_check=1
                    break
                fi
            done

            if [ $has_error_check -eq 0 ]; then
                echo -e "${YELLOW}WARNING${NC} $file:$line_num - Missing error handling for object creation"
                echo "  $line"
                ((WARNINGS++))
            fi
        fi

        # Rule 6: Warn about sleep() in combat or time-sensitive code
        if echo "$line" | grep -q "sleep\|Sleep" && ! echo "$line" | grep -q "//"; then
            if echo "$file" | grep -q "combat\|Combat"; then
                echo -e "${YELLOW}WARNING${NC} $file:$line_num - sleep() in combat code may cause performance issues"
                echo "  $line"
                ((WARNINGS++))
            fi
        fi

        # Rule 7: Check for TODO/FIXME comments
        if echo "$line" | grep -E "TODO|FIXME|XXX|HACK" | grep -q "//"; then
            echo -e "${YELLOW}INFO${NC} $file:$line_num - Code marked for attention"
            echo "  $line"
        fi

        # ========================================
        # OOP PARADIGM VIOLATIONS
        # ========================================

        # OOP Rule 1: Direct character property access instead of using MobileWrapper
        if echo "$line" | grep -E "\.str[^a-zA-Z]|\.dex[^a-zA-Z]|\.int[^a-zA-Z]|\.hp[^a-zA-Z]|\.maxhp|\.stamina[^a-zA-Z]|\.mana[^a-zA-Z]" | grep -qv "//.*\.str\|//.*\.dex\|//.*\.int"; then
            # Exclude lines that are already using wrapper methods (like GetStr, GetDex, etc.)
            if ! echo "$line" | grep -qE "GetStr|GetDex|GetInt|GetHP|GetStamina|GetMana|_str|_dex|_int|_hp|_stamina|_mana"; then
                echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct character property access. Consider using MobileWrapper class"
                echo "  $line"
                echo -e "  ${BLUE}Tip:${NC} Use MobileWrapper(character) and call GetStr(), GetDex(), GetInt(), GetCurrentHP(), etc."
                ((OOP_WARNINGS++))
            fi
        fi

        # OOP Rule 2: Direct item property modification instead of using EquipWrapper
        if echo "$line" | grep -E "item\.ar[^a-zA-Z]|item\.damage|item\.maxhp_mod|item\.quality" | grep -qv "//"; then
            if ! echo "$line" | grep -qE "Wrapper|_ar|_damage|_quality"; then
                echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct item property modification. Consider using EquipWrapper classes"
                echo "  $line"
                echo -e "  ${BLUE}Tip:${NC} Use EquipWrapperArmor or EquipWrapperWeapon for equipment handling"
                ((OOP_WARNINGS++))
            fi
        fi

        # OOP Rule 3: Procedural function naming (verb_noun) instead of OOP methods
        if echo "$line" | grep -E "function [A-Z][a-z]+_[A-Z][a-z]+\(" | grep -qv "//"; then
            echo -e "${MAGENTA}OOP${NC} $file:$line_num - Procedural function naming detected. Consider using class methods"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Convert to class methods or use camelCase: Get_Item() -> GetItem() or item.Get()"
            ((OOP_WARNINGS++))
        fi

        # OOP Rule 4: Global state mutation instead of object encapsulation
        if echo "$line" | grep -E "global [a-zA-Z_]+ :=" | grep -qv "//"; then
            echo -e "${MAGENTA}OOP${NC} $file:$line_num - Global variable mutation. Consider encapsulating in a class"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Use class properties instead of global variables for better encapsulation"
            ((OOP_WARNINGS++))
        fi

        # OOP Rule 5: Direct mobile reference manipulation without wrapper
        if echo "$line" | grep -E "AP_SetVital|AP_GetVital|AP_SetTrueSkill|AP_GetTrueSkill" | grep -qv "//"; then
            # Check if file is not already a wrapper class
            if ! echo "$file" | grep -q "Wrapper\.inc\|Wrapper\.src"; then
                echo -e "${MAGENTA}OOP${NC} $file:$line_num - Direct AP_* function usage. Consider using MobileWrapper methods"
                echo "  $line"
                echo -e "  ${BLUE}Tip:${NC} Use MobileWrapper for character operations"
                ((OOP_WARNINGS++))
            fi
        fi

        # ========================================
        # CODE SMELLS
        # ========================================

        # Smell 1: Long parameter lists (more than 4 parameters)
        if echo "$line" | grep -E "function.*\([^)]*,[^)]*,[^)]*,[^)]*,[^)]*\)" | grep -qv "//"; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Long parameter list (>4 params). Consider using a struct or class"
            echo "  $line"
            ((CODE_SMELLS++))
        fi

        # Smell 2: Magic numbers (hardcoded numeric values except 0, 1, -1)
        if echo "$line" | grep -E "[^a-zA-Z_][2-9][0-9]+[^0-9]|[^a-zA-Z_][2-9][^0-9]" | grep -qv "//.*[0-9]\|const\|CONST\|#define"; then
            # Exclude common patterns like line numbers, array indices, common values
            if ! echo "$line" | grep -qE "0x[0-9a-fA-F]+|\[[0-9]+\]|sleep\(|Sleep\(|SleepMS\("; then
                echo -e "${YELLOW}SMELL${NC} $file:$line_num - Magic number detected. Consider using named constants"
                echo "  $line"
                ((CODE_SMELLS++))
            fi
        fi

        # Smell 3: Deeply nested conditionals (more than 3 levels)
        local indent_level=$(echo "$line" | sed 's/[^\t]//g' | wc -c)
        if [ $indent_level -gt 12 ]; then  # Assuming 4 spaces or 1 tab per level
            if echo "$line" | grep -qE "if|while|for|foreach|case"; then
                echo -e "${YELLOW}SMELL${NC} $file:$line_num - Deep nesting detected (level >3). Consider extracting to separate function"
                echo "  $line"
                ((CODE_SMELLS++))
            fi
        fi

        # Smell 4: God class indicator - file with too many methods
        if echo "$line" | grep -E "^[[:space:]]*function [A-Za-z_]+" | grep -qv "//"; then
            # Count functions in file (simplified check)
            local func_count=$(grep -c "^[[:space:]]*function " "$file" 2>/dev/null || echo "0")
            if [ "$func_count" -gt 20 ]; then
                # Only warn once per file
                if ! grep -q "# GODCLASS_WARNED" "$file" 2>/dev/null; then
                    echo -e "${YELLOW}SMELL${NC} $file - God class detected ($func_count functions). Consider splitting into smaller classes"
                    ((CODE_SMELLS++))
                fi
            fi
        fi

        # Smell 5: Primitive obsession - multiple related primitive parameters
        if echo "$line" | grep -E "GetStr.*GetDex.*GetInt|\.str.*\.dex.*\.int" | grep -qv "//"; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Primitive obsession. Multiple related primitives - consider a Stats class"
            echo "  $line"
            ((CODE_SMELLS++))
        fi

        # Smell 6: Feature envy - accessing another object's data repeatedly
        if echo "$line" | grep -E "([a-zA-Z_]+)\.([a-zA-Z_]+).*\1\.([a-zA-Z_]+).*\1\.([a-zA-Z_]+)" | grep -qv "//"; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Feature envy. Accessing same object's properties multiple times"
            echo "  $line"
            echo -e "  ${BLUE}Tip:${NC} Consider moving this logic to the object's class as a method"
            ((CODE_SMELLS++))
        fi

        # Smell 7: Commented-out code
        if echo "$line" | grep -E "^[[:space:]]*//.*[a-zA-Z_]+\(" | grep -q "//.*:=\|//.*if\|//.*function"; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Commented-out code. Remove or uncomment"
            echo "  $line"
            ((CODE_SMELLS++))
        fi

        # Smell 8: Copy-paste detector - long identical comments or strings
        if echo "$line" | grep -E "^[[:space:]]*//.*TODO.*same|^[[:space:]]*//.*duplicate|^[[:space:]]*//.*copy" | grep -qi "duplicate\|copy"; then
            echo -e "${YELLOW}SMELL${NC} $file:$line_num - Possible code duplication indicated by comment"
            echo "  $line"
            ((CODE_SMELLS++))
        fi

    done < "$file"
}

# Main execution
TARGET="${1:-.}"

if [ ! -e "$TARGET" ]; then
    echo -e "${RED}ERROR${NC}: Path '$TARGET' does not exist"
    exit 1
fi

echo "eScript Linter - Scanning $TARGET"
echo "================================================"

if [ -f "$TARGET" ]; then
    # Single file
    lint_file "$TARGET"
elif [ -d "$TARGET" ]; then
    # Directory - find all .src and .inc files
    while IFS= read -r file; do
        lint_file "$file"
    done < <(find "$TARGET" -type f \( -name "*.src" -o -name "*.inc" \))
fi

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