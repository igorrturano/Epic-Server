#!/usr/bin/env python3
"""Phase A — generate itemdesc.cfg edits for dual_wield weapons.

Applies the Design-A / STAT-20 / 25-35%-premium law to every
Attribute=Dual_Wield weapon, using dual_wield_profile() from the analyzer
as the single source of truth. Rewrites EpicMainDamage / EpicSpeed /
EpicCriticalChance / CriticalHitScripts in-place, preserving formatting.

Run from the repo root. Idempotent.
"""
import re, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from analyze_weapons import (dual_wield_profile, parse_recipes_psv,
                             parse_recipes_cfg_level, build_name_to_objtype,
                             merge_recipes, P2_FINESSE_FLAG_NEEDED,
                             FINESSE_CFG_CORRECTIONS, ITEMDESC_CFG)

ITEMDESC = "pkg/systems/combat/itemdesc.cfg"

FIELDS = ("EpicMainDamage", "EpicSpeed", "EpicCriticalChance", "CriticalHitScripts")

# Option 3 + bleed-on-Garras: these keep their existing CriticalHitScripts
# (flavor sub-archetypes). They still get dice/speed/crit-tier from the law;
# only the crit *script* is preserved. Model under-counts them ~10% pre-resist
# (bleed DoT / armorpierce-vs-armor recover it in real combat — by design).
PRESERVE_CRIT_SCRIPT = {
    "Garras",              # bleed bruiser (user's call)
    "tessen",              # anti-armor sub-line
    "badurandoubleaxes",   # anti-armor sub-line
    "aiglanatwindaggers",  # anti-armor sub-line
    "drowdualkryss",       # anti-armor sub-line
}

def craft_lookup():
    psv = parse_recipes_psv()
    lvl = parse_recipes_cfg_level()
    n2o = build_name_to_objtype()
    merged, _ = merge_recipes(psv, lvl, n2o)
    return merged  # {objtype: (skill, system, components)}

def main(apply=False):
    text = Path(ITEMDESC).read_text(errors="replace")
    lines = text.split("\n")
    recipes = craft_lookup()

    # find weapon blocks
    edits = []          # (name, objtype, old, new) for the report
    out = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        m = re.match(r"\s*Weapon\s+(0x[0-9A-Fa-f]+)", line)
        if not m:
            out.append(line); i += 1; continue
        # collect the block until matching closing brace
        ot = m.group(1).lower()
        block = [line]
        i += 1
        depth = 0
        started = False
        while i < n:
            block.append(lines[i])
            s = lines[i].strip()
            if s == "{": depth += 1; started = True
            elif s == "}":
                depth -= 1
                if started and depth == 0:
                    i += 1; break
            i += 1

        # parse props
        props = {}
        for bl in block:
            mm = re.match(r"\s*(\S+)\s+(.*?)\s*$", bl)
            if mm: props.setdefault(mm.group(1), mm.group(2))
        attr = (props.get("Attribute") or "").strip()
        if attr != "Dual_Wield":
            out.extend(block); continue

        name = (props.get("Name") or "").strip()
        finesse = ((props.get("Finesse","").strip() in ("1","i1") or name in P2_FINESSE_FLAG_NEEDED)
                   and name not in FINESSE_CFG_CORRECTIONS)
        skill = recipes.get(ot, (0.0,"",""))[0]
        dice, spd, crit, script = dual_wield_profile(skill, finesse)
        want = {
            "EpicMainDamage": dice,
            "EpicSpeed": spd,
            "EpicCriticalChance": crit,
            "CriticalHitScripts": f":combat:criticalHitScripts/{script}",
        }
        if name in PRESERVE_CRIT_SCRIPT:
            want.pop("CriticalHitScripts")   # keep existing flavor script

        # rewrite the block: replace existing field lines, track which seen
        new_block = []
        seen = set()
        close_idx = None
        for idx, bl in enumerate(block):
            mm = re.match(r"(\s*)(\S+)(\s+)(.*?)\s*$", bl)
            if mm and mm.group(2) in want:
                key = mm.group(2)
                seen.add(key)
                old_val = mm.group(4)
                new_val = want[key]
                if old_val.strip() != new_val:
                    edits.append((name, ot, f"{key} {old_val}", f"{key} {new_val}"))
                new_block.append(f"{mm.group(1)}{key}{mm.group(3)}{new_val}")
            else:
                new_block.append(bl)
            if bl.strip() == "}":
                close_idx = len(new_block) - 1
        # insert missing fields right before the closing brace, tab-indented
        missing = [k for k in FIELDS if k not in seen and k in want]
        if missing and close_idx is not None:
            ins = [f"\t{k}\t{want[k]}" for k in missing]
            for k in missing:
                edits.append((name, ot, f"(missing {k})", f"{k} {want[k]}"))
            new_block = new_block[:close_idx] + ins + new_block[close_idx:]
        out.extend(new_block)

    new_text = "\n".join(out)
    # report
    by_w = {}
    for nm, ot, o, nw in edits:
        by_w.setdefault((nm,ot), []).append((o,nw))
    print(f"dual_wield weapons edited: {len(by_w)}  total field changes: {len(edits)}\n")
    for (nm,ot), chs in sorted(by_w.items()):
        print(f"  {nm} ({ot})")
        for o,nw in chs:
            print(f"      - {o}")
            print(f"      + {nw}")
    if apply:
        Path(ITEMDESC).write_text(new_text)
        print(f"\nWROTE {ITEMDESC}")
    else:
        print(f"\n(dry-run — pass --apply to write {ITEMDESC})")

if __name__ == "__main__":
    main(apply="--apply" in sys.argv)
