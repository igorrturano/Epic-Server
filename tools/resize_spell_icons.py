#!/usr/bin/env python3
"""
Resize spell images (1024px jpeg) to 44x44 png icons for ClassicUO.

Usage:
    python3 tools/resize_spell_icons.py [--start-slot 2716] [--dry-run]

Reads spell names from the .cfg, matches them to images in the source folder,
resizes to 44x44, saves as .png in the gumps folder, and prints the cfg edits.
"""

import os
import sys
import re
import argparse
import unicodedata

# Adjust this import for the venv
sys.path.insert(0, "/tmp/imgtools/lib/python3.12/site-packages")
from PIL import Image

# === CONFIGURATION ===
SOURCE_DIR = "/home/igorrturano/epic/epic-shard/epic Figures/Spells"
DEST_DIR = "/mnt/c/Temp/ClassicUO-Debug/ExternalImages/gumps"
CFG_FILE = "/home/igorrturano/epic/epic-shard/pkg/systems/spells/config/spellcaster.cfg"
ICON_SIZE = (44, 44)


def normalize(text):
    """Remove accents and lowercase for fuzzy matching."""
    nfkd = unicodedata.normalize("NFKD", text)
    return "".join(c for c in nfkd if not unicodedata.category(c).startswith("M")).lower().strip()


def parse_cfg(cfg_path):
    """Parse .cfg and return list of {spell_id, name, icon, line_number}."""
    spells = []
    with open(cfg_path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    current_spell = {}
    in_block = False
    for i, line in enumerate(lines):
        stripped = line.strip()

        # Skip commented lines
        if stripped.startswith("#"):
            continue

        if re.match(r"^Spell\s+\d+", stripped):
            in_block = False
            current_spell = {}

        if stripped == "{":
            in_block = True
            continue

        if stripped == "}":
            if current_spell.get("spell_id") and current_spell.get("name"):
                spells.append(current_spell)
            current_spell = {}
            in_block = False
            continue

        if in_block:
            m = re.match(r"SpellId\s+(\d+)", stripped)
            if m:
                current_spell["spell_id"] = int(m.group(1))

            m = re.match(r"Name\s+(.+)", stripped)
            if m:
                current_spell["name"] = m.group(1).strip()

            m = re.match(r"Icon\s+(0x[0-9a-fA-F]+|\d+)", stripped)
            if m:
                current_spell["icon_value"] = m.group(1)
                current_spell["icon_line"] = i  # 0-based line index

    return spells


def find_image_for_spell(spell_name, image_files, prefer_variant=""):
    """Match a spell name to an image file using normalized comparison.
    If prefer_variant is set (e.g. "(2)"), try that suffix first."""
    spell_norm = normalize(spell_name)

    # If prefer_variant, try spell_name + variant first (e.g. "Dissipar Magia (2)")
    if prefer_variant:
        variant_norm = normalize(spell_name + " " + prefer_variant)
        for img_file in image_files:
            img_name = os.path.splitext(img_file)[0]
            if normalize(img_name) == variant_norm:
                return img_file

    # Try exact match (normalized)
    for img_file in image_files:
        img_name = os.path.splitext(img_file)[0]
        if normalize(img_name) == spell_norm:
            return img_file

    # Try contains match
    for img_file in image_files:
        img_name = normalize(os.path.splitext(img_file)[0])
        if img_name in spell_norm or spell_norm in img_name:
            return img_file

    return None


def resize_and_save(src_path, dest_path):
    """Resize image to 44x44 and save as PNG."""
    with Image.open(src_path) as img:
        img = img.resize(ICON_SIZE, Image.Resampling.LANCZOS)
        img.save(dest_path, "PNG")


def main():
    parser = argparse.ArgumentParser(description="Resize spell icons")
    parser.add_argument("--start-slot", type=int, default=2716,
                        help="First icon slot number (default: 2716)")
    parser.add_argument("--dry-run", action="store_true",
                        help="Only show what would be done, don't process")
    parser.add_argument("--skip-existing", action="store_true", default=True,
                        help="Skip if destination .png already exists (default: true)")
    parser.add_argument("--cfg", default=CFG_FILE,
                        help="Path to spell .cfg file")
    parser.add_argument("--source", default=SOURCE_DIR,
                        help="Source image folder")
    parser.add_argument("--dest", default=DEST_DIR,
                        help="Destination gumps folder")
    parser.add_argument("--prefer-variant", default="",
                        help="Prefer image variant suffix (e.g. '(2)') when matching")
    args = parser.parse_args()

    # Parse cfg
    spells = parse_cfg(args.cfg)
    print(f"Found {len(spells)} spells in cfg")

    # List source images
    image_files = [f for f in os.listdir(args.source)
                   if f.lower().endswith((".jpeg", ".jpg", ".png"))]
    print(f"Found {len(image_files)} images in {args.source}")
    print()

    # Placeholder icons that mean "not yet assigned"
    PLACEHOLDERS = {"0x5DCD", "0x8DE", "0x8FA", "0x9b8e", "0x9BA6", "0x9b9f", "0x59db", "0x9b96", "0x9b99", "0x59de", "0x59dd"}

    # Match and assign slots, respecting already-assigned icons in cfg
    results = []
    unmatched_spells = []
    unmatched_images = list(image_files)
    needs_new_slot = []

    # First pass: keep existing assignments, collect spells needing new slots
    for spell in spells:
        icon_val = spell.get("icon_value", "0x5DCD")
        img_file = find_image_for_spell(spell["name"], image_files, args.prefer_variant)

        if not img_file:
            unmatched_spells.append(spell)
            continue

        if img_file in unmatched_images:
            unmatched_images.remove(img_file)

        # If spell already has a real (non-placeholder) icon, keep it
        if icon_val not in PLACEHOLDERS:
            existing_slot = int(icon_val, 16) if icon_val.startswith("0x") else int(icon_val)
            dest_path = os.path.join(args.dest, f"{existing_slot}.png")
            results.append({
                "spell": spell,
                "image": img_file,
                "slot": existing_slot,
                "dest_path": dest_path,
                "skip": os.path.exists(dest_path),
                "kept": True,
            })
        else:
            needs_new_slot.append((spell, img_file))

    # Find next available slot (after highest existing assignment)
    used_slots = {r["slot"] for r in results}
    slot = args.start_slot
    if used_slots:
        slot = max(max(used_slots) + 1, args.start_slot)

    # Second pass: assign new sequential slots
    for spell, img_file in needs_new_slot:
        while slot in used_slots:
            slot += 1
        dest_path = os.path.join(args.dest, f"{slot}.png")
        results.append({
            "spell": spell,
            "image": img_file,
            "slot": slot,
            "dest_path": dest_path,
            "skip": False,
            "kept": False,
        })
        used_slots.add(slot)
        slot += 1

    # Sort results by slot for display
    results.sort(key=lambda r: r["slot"])

    # Show plan
    print("=== MATCHED ===")
    for r in results:
        status = "KEPT" if r.get("kept") else ("SKIP (exists)" if r["skip"] else "NEW")
        hex_slot = hex(r["slot"])
        print(f"  [{status}] SpellId {r['spell']['spell_id']:>3} "
              f"'{r['spell']['name']}' -> {r['image']} -> "
              f"slot {r['slot']} ({hex_slot})")
    print()

    if unmatched_spells:
        print("=== NO IMAGE FOUND ===")
        for s in unmatched_spells:
            print(f"  SpellId {s['spell_id']:>3} '{s['name']}'")
        print()

    if unmatched_images:
        print("=== UNUSED IMAGES ===")
        for img in unmatched_images:
            print(f"  {img}")
        print()

    print(f"Total: {len(results)} matched, {len(unmatched_spells)} without image, "
          f"{len(unmatched_images)} unused images")
    print(f"Slot range: {args.start_slot} - {slot - 1}")
    print()

    if args.dry_run:
        print("DRY RUN - no files modified")
        return

    # Process images
    processed = 0
    for r in results:
        if r["skip"]:
            continue
        src_path = os.path.join(args.source, r["image"])
        resize_and_save(src_path, r["dest_path"])
        processed += 1

    print(f"Processed {processed} images")
    print()

    # Print cfg edits needed
    print("=== CFG ICON UPDATES ===")
    print(f"(Update these Icon values in {os.path.basename(args.cfg)})")
    print()
    for r in results:
        print(f"  SpellId {r['spell']['spell_id']:>3} ({r['spell']['name']}): "
              f"Icon\t\t{r['slot']}")


if __name__ == "__main__":
    main()
