#!/usr/bin/env bash
# Reassemble src/ chapter files into the single document you paste into
# Homebrewery. Numeric filename prefixes make `sort` produce exact page order.
#
# Usage:
#   ./build.sh           build dist/handbook.brew.md
#   ./build.sh --copy     build AND copy the whole thing to the clipboard
#                         (WSL clip.exe / xclip / pbcopy) — then just Ctrl+V
#                         into the Homebrewery editor.
set -euo pipefail
cd "$(dirname "$0")"

out="dist/handbook.brew.md"
mkdir -p dist
find src -type f -name '*.md' | sort | xargs cat > "$out"

lines=$(wc -l < "$out")
echo "built $out ($lines lines) from $(find src -type f -name '*.md' | wc -l) chapter files"

if [ "${1:-}" = "--copy" ]; then
  if command -v clip.exe >/dev/null 2>&1; then
    clip.exe < "$out"; echo "copied to clipboard via clip.exe — paste into Homebrewery (Ctrl+V)"
  elif command -v xclip >/dev/null 2>&1; then
    xclip -selection clipboard < "$out"; echo "copied to clipboard via xclip — paste into Homebrewery"
  elif command -v pbcopy >/dev/null 2>&1; then
    pbcopy < "$out"; echo "copied to clipboard via pbcopy — paste into Homebrewery"
  else
    echo "no clipboard tool found (clip.exe/xclip/pbcopy); open $out and copy manually"
  fi
fi

# Optional safety net: if the untouched backup still matches, prove the
# pipeline is lossless. Diff is silent on success, noisy on drift.
if [ -f homebrewery.txt ]; then
  if cmp -s homebrewery.txt "$out"; then
    echo "round-trip: identical to homebrewery.txt backup (no edits yet)"
  else
    echo "round-trip: differs from backup (expected once you start editing)"
  fi
fi
