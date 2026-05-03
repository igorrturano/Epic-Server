#!/bin/bash

# OOP Adoption Metrics - Track your team's progress migrating to OOP
# Usage: ./oop-metrics.sh [directory]
# Default: scans pkg/ directory

TARGET="${1:-pkg}"

echo "================================================"
echo "OOP Adoption Metrics for: $TARGET"
echo "================================================"
echo ""

# Count total files
TOTAL_SRC=$(find "$TARGET" -type f -name "*.src" | wc -l)
TOTAL_INC=$(find "$TARGET" -type f -name "*.inc" | wc -l)
TOTAL_FILES=$((TOTAL_SRC + TOTAL_INC))

echo "📁 Files:"
echo "   .src files: $TOTAL_SRC"
echo "   .inc files: $TOTAL_INC"
echo "   Total:      $TOTAL_FILES"
echo ""

# Count OOP patterns (good)
echo "✅ OOP Patterns (Good):"

CLASS_COUNT=$(grep -r "^[[:space:]]*class " "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | wc -l)
echo "   Classes defined:        $CLASS_COUNT"

WRAPPER_USAGE=$(grep -r "MobileWrapper\|EquipWrapper" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | wc -l)
echo "   Wrapper usage:          $WRAPPER_USAGE"

METHOD_CALLS=$(grep -r "\.Get[A-Z]\|\.Set[A-Z]\|\.Apply" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | wc -l)
echo "   Method calls:           $METHOD_CALLS"

echo ""

# Count procedural patterns (bad)
echo "⚠️  Procedural Patterns (Needs Migration):"

DIRECT_PROPS=$(grep -rE "\.str[^a-zA-Z]|\.dex[^a-zA-Z]|\.int[^a-zA-Z]" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | grep -v "Wrapper\.inc\|_str\|_dex\|_int" | wc -l)
echo "   Direct property access: $DIRECT_PROPS"

AP_FUNCS=$(grep -rE "AP_SetVital|AP_GetVital|AP_SetTrueSkill|AP_GetTrueSkill" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | grep -v "Wrapper\.inc" | wc -l)
echo "   Direct AP_* functions:  $AP_FUNCS"

SENDSYSMSG=$(grep -r "SendSysMessage\|SendSysMsg" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | wc -l)
echo "   SendSysMessage calls:   $SENDSYSMSG"

PROCEDURAL_FUNCS=$(grep -rE "function [A-Z][a-z]+_[A-Z][a-z]+\(" "$TARGET" --include="*.src" --include="*.inc" 2>/dev/null | wc -l)
echo "   Procedural functions:   $PROCEDURAL_FUNCS"

echo ""

# Calculate OOP adoption percentage
GOOD_PATTERNS=$((WRAPPER_USAGE + METHOD_CALLS))
BAD_PATTERNS=$((DIRECT_PROPS + AP_FUNCS + SENDSYSMSG + PROCEDURAL_FUNCS))
TOTAL_PATTERNS=$((GOOD_PATTERNS + BAD_PATTERNS))

if [ $TOTAL_PATTERNS -gt 0 ]; then
    OOP_PERCENTAGE=$((GOOD_PATTERNS * 100 / TOTAL_PATTERNS))
else
    OOP_PERCENTAGE=0
fi

echo "📊 OOP Adoption Score: ${OOP_PERCENTAGE}%"
echo ""

# Visual progress bar
BAR_LENGTH=50
FILLED_LENGTH=$((OOP_PERCENTAGE * BAR_LENGTH / 100))
EMPTY_LENGTH=$((BAR_LENGTH - FILLED_LENGTH))

printf "   ["
printf "█%.0s" $(seq 1 $FILLED_LENGTH)
printf "░%.0s" $(seq 1 $EMPTY_LENGTH)
printf "]\n"
echo ""

# Recommendations
if [ $OOP_PERCENTAGE -lt 30 ]; then
    echo "🎯 Recommendation: Focus on replacing direct property access with MobileWrapper"
elif [ $OOP_PERCENTAGE -lt 60 ]; then
    echo "🎯 Recommendation: Good progress! Continue migrating AP_* functions to wrapper methods"
elif [ $OOP_PERCENTAGE -lt 90 ]; then
    echo "🎯 Recommendation: Excellent! Focus on edge cases and procedural function naming"
else
    echo "🎯 Recommendation: Outstanding! Your codebase is mostly OOP-compliant"
fi

echo ""

# Top offenders - files with most procedural patterns
echo "🔍 Top Files Needing Migration:"
echo ""

# Create a temporary file to store results
TEMP_FILE=$(mktemp)

# Find files with most direct property access
find "$TARGET" -type f \( -name "*.src" -o -name "*.inc" \) -exec sh -c '
    FILE="$1"
    COUNT=$(grep -cE "\.str[^a-zA-Z]|\.dex[^a-zA-Z]|\.int[^a-zA-Z]|AP_SetVital|AP_GetVital|SendSysMessage" "$FILE" 2>/dev/null || echo 0)
    if [ "$COUNT" -gt 0 ]; then
        echo "$COUNT $FILE"
    fi
' _ {} \; | sort -rn | head -10 > "$TEMP_FILE"

if [ -s "$TEMP_FILE" ]; then
    while IFS= read -r line; do
        COUNT=$(echo "$line" | awk '{print $1}')
        FILE=$(echo "$line" | awk '{print $2}')
        FILENAME=$(basename "$FILE")
        echo "   $COUNT violations - $FILENAME"
    done < "$TEMP_FILE"
else
    echo "   No major violations found!"
fi

rm "$TEMP_FILE"

echo ""
echo "================================================"
echo "Run './scripts/lint-escript.sh $TARGET' for detailed violations"
echo "================================================"
