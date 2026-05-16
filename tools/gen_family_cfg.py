#!/usr/bin/env python3
"""Generic per-family itemdesc.cfg cleanup generator.

Usage:
    python3 tools/gen_family_cfg.py dual_wield [--apply]
    python3 tools/gen_family_cfg.py two_handed [--apply]

Rewrites EpicMainDamage / EpicSpeed / EpicCriticalChance / CriticalHitScripts
(and optionally adds `Finesse 1`) for every weapon of the given Attribute,
using the family's *_profile() from analyze_weapons as the source of truth.
Idempotent. Preserves tab formatting.
"""
import re, sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import analyze_weapons as aw

ITEMDESC = "pkg/systems/combat/itemdesc.cfg"
FIELDS = ("EpicMainDamage", "EpicSpeed", "EpicCriticalChance", "CriticalHitScripts")

FAMILIES = {
    "dual_wield": dict(attr="Dual_Wield", profile=aw.dual_wield_profile,
                       preserve=getattr(aw, "PRESERVE_CRIT_SCRIPT",
                                        {"Garras","tessen","badurandoubleaxes",
                                         "aiglanatwindaggers","drowdualkryss"}),
                       make_finesse=set(), offhand_mirror=True),
    "two_handed": dict(attr="Two_Handed", profile=aw.two_handed_profile,
                       preserve=aw.TWO_HANDED_PRESERVE_CRIT,
                       make_finesse=aw.TWO_HANDED_MAKE_FINESSE, offhand_mirror=False),
    "one_handed": dict(attr="One_Handed", profile=aw.one_handed_profile,
                       preserve=aw.ONE_HANDED_PRESERVE_CRIT,
                       make_finesse=set(), offhand_mirror=False),
}

def craft_lookup():
    merged, _ = aw.merge_recipes(aw.parse_recipes_psv(),
                                 aw.parse_recipes_cfg_level(),
                                 aw.build_name_to_objtype())
    return merged

def is_finesse(name, props, make_finesse):
    return ((props.get("Finesse","").strip() in ("1","i1")
             or name in aw.P2_FINESSE_FLAG_NEEDED
             or name in make_finesse)
            and name not in aw.FINESSE_CFG_CORRECTIONS)

def main(family, apply=False):
    cfg = FAMILIES[family]
    text = Path(ITEMDESC).read_text(errors="replace")
    lines = text.split("\n")
    recipes = craft_lookup()
    edits, out = [], []
    i, n = 0, len(lines)
    while i < n:
        line = lines[i]
        if not re.match(r"\s*Weapon\s+(0x[0-9A-Fa-f]+|[0-9]+)", line):
            out.append(line); i += 1; continue
        ot = aw.norm_ot(re.match(r"\s*Weapon\s+(0x[0-9A-Fa-f]+|[0-9]+)", line).group(1))
        block = [line]; i += 1; depth = 0; started = False
        while i < n:
            block.append(lines[i]); s = lines[i].strip()
            if s == "{": depth += 1; started = True
            elif s == "}":
                depth -= 1
                if started and depth == 0: i += 1; break
            i += 1
        props = {}
        for bl in block:
            mm = re.match(r"\s*(\S+)\s+(.*?)\s*$", bl)
            if mm: props.setdefault(mm.group(1), mm.group(2))
        if (props.get("Attribute") or "").strip() != cfg["attr"]:
            out.extend(block); continue

        name = (props.get("Name") or "").strip()
        fin  = is_finesse(name, props, cfg["make_finesse"])
        skill = recipes.get(ot, (0.0,"",""))[0]
        skill = aw.tier_override_craft(name, skill)  # signature forced-tier weapons
        dice, spd, crit, script = cfg["profile"](skill, fin)
        want = {"EpicMainDamage": dice, "EpicSpeed": spd,
                "EpicCriticalChance": crit,
                "CriticalHitScripts": f":combat:criticalHitScripts/{script}"}
        if cfg.get("offhand_mirror"):
            # dual-wield = twin identical blades; offhand swing reads
            # EpicOffDamage (DamageController.inc:277). Mirror mainhand so
            # the model's "offhand uses same dice" assumption holds in-engine.
            want["EpicOffDamage"] = dice
        if name in cfg["preserve"]:
            want.pop("CriticalHitScripts")
        add_finesse = name in cfg["make_finesse"] and props.get("Finesse","").strip() not in ("1","i1")

        new_block, seen, close_idx = [], set(), None
        for bl in block:
            mm = re.match(r"(\s*)(\S+)(\s+)(.*?)\s*$", bl)
            if mm and mm.group(2) in want:
                k = mm.group(2); seen.add(k)
                if mm.group(4).strip() != want[k]:
                    edits.append((name, ot, f"{k} {mm.group(4)}", f"{k} {want[k]}"))
                new_block.append(f"{mm.group(1)}{k}{mm.group(3)}{want[k]}")
            else:
                new_block.append(bl)
            if bl.strip() == "}": close_idx = len(new_block) - 1
        order = ("EpicMainDamage","EpicOffDamage","EpicSpeed",
                 "EpicCriticalChance","CriticalHitScripts")
        ins = [f"\t{k}\t{want[k]}" for k in order if k not in seen and k in want]
        for k in order:
            if k not in seen and k in want:
                edits.append((name, ot, f"(missing {k})", f"{k} {want[k]}"))
        if add_finesse:
            ins.insert(0, "\tFinesse\t1")
            edits.append((name, ot, "(no Finesse)", "Finesse 1  [Dex-anchor]"))
        if ins and close_idx is not None:
            new_block = new_block[:close_idx] + ins + new_block[close_idx:]
        out.extend(new_block)

    by_w = {}
    for nm, ot, o, nw in edits:
        by_w.setdefault((nm,ot), []).append((o,nw))
    print(f"[{family}] weapons edited: {len(by_w)}  field changes: {len(edits)}\n")
    for (nm,ot), chs in sorted(by_w.items()):
        print(f"  {nm} ({ot})")
        for o,nw in chs:
            print(f"      - {o}\n      + {nw}")
    if apply:
        Path(ITEMDESC).write_text("\n".join(out)); print(f"\nWROTE {ITEMDESC}")
    else:
        print(f"\n(dry-run — add --apply to write)")

if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    fam = args[0] if args else "two_handed"
    main(fam, apply="--apply" in sys.argv)
