#!/usr/bin/env bash
# Reassemble src/ chapter files into the single document you paste into
# Homebrewery. Numeric filename prefixes make `sort` produce exact page order.
set -euo pipefail
cd "$(dirname "$0")"

out="dist/handbook.brew.md"
mkdir -p dist
find src -type f -name '*.md' | sort | xargs cat > "$out"

lines=$(wc -l < "$out")
echo "built $out ($lines lines) from $(find src -type f -name '*.md' | wc -l) chapter files"

# Optional safety net: if the untouched backup still matches, prove the
# pipeline is lossless. Diff is silent on success, noisy on drift.
if [ -f homebrewery.txt ]; then
  if cmp -s homebrewery.txt "$out"; then
    echo "round-trip: identical to homebrewery.txt backup (no edits yet)"
  else
    echo "round-trip: differs from backup (expected once you start editing)"
  fi
fi
