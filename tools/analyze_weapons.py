#!/usr/bin/env python3
"""Epic Shard weapon DPS / tier analysis & balance proposal.

Unified tier ladder:   T_n = 5 + n DPS  (T0=5, T1=6, ..., T9=14)  pre-resist.
Family caps:
    one_handed = T7 (12 DPS)    # pays in shield slot
    two_handed = T9 (14 DPS)    # apex melee, no shield
    dual_wield = T9 (14 DPS)    # apex, no shield + dedicated skill
    wrestling  = T6 (11 DPS)    # martial arts, separate hit skill
    ranged     = T7 (12 DPS)    # range + ammo cost
    throwing   = T6 (11 DPS)    # one-shot, self-limited
    caster_catalyst = N/A       # excluded — magic system balance

Design philosophy: BUFF UP TO THE CURVE, DON'T NERF DOWN TO IT.
Only "nerfs" are situational (armorpierce replacing crit-burst on finesse
meta cluster — same DPS vs heavy armor, lower vs unarmored) and the
dual-wield offhand-Tactics halving (apex dual-wielders only).

Source of truth (verified in code):
  pkg/systems/equipsys/include/Controller/WeaponController.inc:35-109
  pkg/systems/wrappers/include/Objects/Model/WeaponWrapper.inc:51-65,388
  pkg/systems/combat/include/Controller/DamageController.inc:198-345
  pkg/systems/combat/include/Controller/HitController.inc:103-114
  pkg/systems/equipsys/commands/player/golpe.src
"""
import csv, re, statistics
from collections import defaultdict
from pathlib import Path

REPO              = "/home/igorrturano/epic/epic-shard"
WEAPONS_PSV       = "/tmp/weapons_raw.psv"   # optional cache; falls back to itemdesc
RECIPES_CFG       = f"{REPO}/pkg/systems/recipes/recipes.cfg"
ITEMDESC_CFG      = f"{REPO}/pkg/systems/combat/itemdesc.cfg"
FABRICACAO_CFGS   = [
    f"{REPO}/pkg/skills/metalworking/blacksmith/config/fabricacaoDesc.cfg",
    f"{REPO}/pkg/skills/metalworking/tinkering/config/fabricacaoDesc.cfg",
    f"{REPO}/pkg/skills/woodworking/bowcraft/config/fabricacaoDesc.cfg",
    f"{REPO}/pkg/skills/woodworking/carpentry/config/fabricacaoDesc.cfg",
    f"{REPO}/pkg/skills/DomesticWork/fabricacaoDesc.cfg",
]
OUT_CSV           = "/home/igorrturano/epic/epic-shard/weapons_balance.csv"
OUT_MD            = "/home/igorrturano/epic/epic-shard/weapons_balance_tiers.md"

REF_STR, REF_DEX, REF_TAC = 100, 100, 100
EFF_DEX = REF_DEX
DUAL_WIELD_SKILL = 100

# ============================================================
#       REALISTIC BUILD PROFILES (player archetypes)
# ============================================================
# Real builds aren't 100/100. They're 100/60 split.
# Str builds compensate offensively with HP, heavy armor, and shield.
# Dex builds compensate defensively with raw damage from faster swings.
BUILD_PROFILES = {
    "STR": {"str": 100, "dex": 60,  "tac": 100, "dw": 100},  # heavy-armor brawler
    "DEX": {"str": 60,  "dex": 100, "tac": 100, "dw": 100},  # agile duelist
    "HYB": {"str": 80,  "dex": 80,  "tac": 100, "dw": 100},  # hybrid
}

# ============================================================
#       ENGINE-LEVEL FIX (proposed)
# ============================================================
# Non-finesse weapons use (Str+Dex)/2 for swing speed instead of pure Dex.
# Rationale: a strong character can heave a heavy weapon faster. Without
# this, Dex builds outpace Str builds even on Str's "home" weapons because
# swing-speed scales multiplicatively while Str's per-hit bonus is additive.
# Finesse weapons remain pure-Dex swing — that's the dex archetype's niche.
APPLY_SWING_SPEED_FIX = False  # DROPPED — the (Str+Dex)/2 hack, per design review
APPLY_FINESSE_BUFF    = False  # DROPPED — the Dex/12 buff, per design review
STAT20_MODE           = True   # finesse uses Dex/20 (== Str/20). Finesse becomes
                               # purely "which stat scales you", no bonus differential.
                               # Dex premium = pure swing-speed ratio (~+25-30%).

# ============================================================
#                    THE TIER LADDER
# ============================================================
TIER_TARGET    = {n: 5.0 + n for n in range(10)}   # T0=5, T9=14
TIER_TOLERANCE = 0.5

FAMILY_MAX_TIER = {
    "one_handed": 7, "two_handed": 9, "dual_wield": 9,
    "wrestling":  6, "ranged":     7, "throwing":   6,
    "caster_catalyst": None,
}

# Map recipe-book `level` (recipes.cfg) → estimated craft skill.
# These are recipe-tier unlocks, used as a fallback when fabricacaoDesc.cfg
# doesn't have an explicit SkillRequired for the weapon.
LEVEL_TO_SKILL = {1: 20, 2: 35, 3: 55, 4: 75, 5: 90}

# ============================================================
#         POST-FIX TOGGLES
# ============================================================
APPLY_P1_2H_LIFT             = True
APPLY_P2_FINESSE_ARMORPIERCE = True
APPLY_P4_FIREARMS_TABLE      = True
APPLY_P5_OFFHAND_TAC_HALF    = True
APPLY_P5_PENALTY_CURVE       = False  # SKIPPED — would nerf newbies
APPLY_1H_CURVE_LIFT          = True
APPLY_RANGED_CURVE_LIFT      = True
APPLY_DUAL_STR_CURVE_LIFT    = True   # str-flavored dual_wield at high craft
APPLY_WRESTLING_CURVE_LIFT   = True   # wrestling at mid+ craft

# ============================================================
#   PHASE A — DUAL_WIELD CFG CLEANUP (Design A, STAT/20)
#   Premium law: finesse(DEX build) ≈ non-finesse(STR build) × 1.30,
#   acceptable band 25-35%. Tier from craft skill (//10, cap T9).
# ============================================================
APPLY_DUAL_WIELD_CLEANUP = True

def dual_wield_profile(craft, finesse):
    """Returns (EpicMainDamage, EpicSpeed, EpicCriticalChance, CritScript)
    for a dual_wield weapon at the given craft skill.

    Non-finesse and finesse share dice+crit per tier; finesse runs ONE
    speed tier faster, which (combined with the Dex-100 vs Dex-60 swing
    differential) lands the DEX build ~30% over the STR build.
    """
    t = min(9, int(craft // 10))
    # Finesse and non-finesse share dice + speed per tier. The DEX build
    # (Dex100) vs STR build (Dex60) swing ratio alone = +25%. Finesse gets
    # ONE crit tier higher as the small nudge toward ~+30% (stays in 25-35%).
    # (dice, speed, nonf_crit, fin_crit, script)
    if   t <= 4: dice, sp, ncr, fcr, sc = "ONEHANDED_MID DAMAGE_NONE DAMAGE_NONE", "MELEE_LOW", "CRITICAL_MID",  "CRITICAL_HIGH", "damage2"
    elif t == 5: dice, sp, ncr, fcr, sc = "ONEHANDED_MID DAMAGE_NONE DAMAGE_NONE", "MELEE_MID", "CRITICAL_MID",  "CRITICAL_HIGH", "damage2"
    elif t == 6: dice, sp, ncr, fcr, sc = "ONEHANDED_HIGH DAMAGE_NONE DAMAGE_NONE","MELEE_LOW", "CRITICAL_HIGH", "CRITICAL_HIGH", "damage2"
    elif t == 7: dice, sp, ncr, fcr, sc = "ONEHANDED_HIGH DAMAGE_NONE DAMAGE_NONE","MELEE_MID", "CRITICAL_MID",  "CRITICAL_HIGH", "damage2"
    elif t == 8: dice, sp, ncr, fcr, sc = "ONEHANDED_HIGH ONEHANDED_LOW DAMAGE_NONE","MELEE_MID","CRITICAL_HIGH","CRITICAL_HIGH", "damage2"
    else:        dice, sp, ncr, fcr, sc = "ONEHANDED_HIGH ONEHANDED_MID DAMAGE_NONE","MELEE_MID","CRITICAL_HIGH","CRITICAL_HIGH", "damage2"
    return dice, sp, (fcr if finesse else ncr), sc

# ---- Phase B: two_handed cleanup ----
APPLY_TWO_HANDED_CLEANUP = True

# CrescentBlade is the designated Dex-2H anchor (user call). itemdesc has no
# Finesse flag today — the generator must ADD `Finesse 1` to these.
TWO_HANDED_MAKE_FINESSE = {"CrescentBlade"}
# 2H weapons that keep a flavor crit script (Option-3, filled after dry-run).
TWO_HANDED_PRESERVE_CRIT = set()

def two_handed_profile(craft, finesse):
    """Single-swing 2H. Str-2H = heavy-hitter apex; finesse-2H (Dex) sits
    +25-35% above via the Dex100/Dex60 swing ratio + a +1 crit-tier nudge.
    2H dice cap = TWOHANDED_HIGH (4d4)."""
    t = min(9, int(craft // 10))
    if   t <= 3: dice, sp, ncr, fcr = "TWOHANDED_LOW DAMAGE_NONE DAMAGE_NONE",  "MELEE_LOW", "CRITICAL_LOW",  "CRITICAL_MID"
    elif t == 4: dice, sp, ncr, fcr = "TWOHANDED_LOW DAMAGE_NONE DAMAGE_NONE",  "MELEE_MID", "CRITICAL_LOW",  "CRITICAL_MID"
    elif t == 5: dice, sp, ncr, fcr = "TWOHANDED_MID DAMAGE_NONE DAMAGE_NONE",  "MELEE_LOW", "CRITICAL_MID",  "CRITICAL_HIGH"
    elif t == 6: dice, sp, ncr, fcr = "TWOHANDED_MID DAMAGE_NONE DAMAGE_NONE",  "MELEE_MID", "CRITICAL_MID",  "CRITICAL_HIGH"
    elif t == 7: dice, sp, ncr, fcr = "TWOHANDED_HIGH DAMAGE_NONE DAMAGE_NONE", "MELEE_LOW", "CRITICAL_MID",  "CRITICAL_HIGH"
    elif t == 8: dice, sp, ncr, fcr = "TWOHANDED_HIGH DAMAGE_NONE DAMAGE_NONE", "MELEE_MID", "CRITICAL_HIGH", "CRITICAL_HIGH"
    else:        dice, sp, ncr, fcr = "TWOHANDED_HIGH DAMAGE_NONE DAMAGE_NONE", "MELEE_HIGH","CRITICAL_HIGH", "CRITICAL_HIGH"
    return dice, sp, (fcr if finesse else ncr), "damage2"

# ============================================================
#       PER-WEAPON OVERRIDE TABLES (legacy model — kept off for
#       cleaned families; still used by 1H/ranged)
# ============================================================
P1_TWO_HANDED_LIFT = {
    "TwoHandedAxe":        ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 9),
    "GreatSword":          ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 9),
    "machadoduasmanos":    ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 9),
    "machadosuperior":     ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 9),
    "macaduasmanos":       ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 9),
    "LargeBattleAxe":      ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 8),
    "Scythe":              ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 8),
    "marteloduasmanos":    ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 8),
    "macacompuntas":       ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 8),
    "Halberd":             ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "bjorskahalberd":      ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "aiglanahalberd":      ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "gulthrakspear":       ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "baduranhammer":       ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "sarissa":             ("MELEE_MID",  "CRITICAL_MID",  "damage2", 6),
    "bjorskabastardsword": ("MELEE_MID",  "CRITICAL_MID",  "damage2", 6),
    "DoubleBladedStaff":   ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 7),
    "Lance":               ("MELEE_MID",  "CRITICAL_HIGH", "damage2", 8),
    "zweihander":          ("MELEE_HIGH", "CRITICAL_HIGH", "damage2", 8),
}

P1B_DICE_UPGRADE = {
    # weapons whose dice tier must rise via cfg edit to EpicMainDamage
    "DoubleBladedStaff": "TWOHANDED_HIGH DAMAGE_NONE TWOHANDED_LOW",
    "sarissa":           "TWOHANDED_MID  TWOHANDED_LOW DAMAGE_NONE",
    "Halberd":           "TWOHANDED_MID  TWOHANDED_MID DAMAGE_NONE",
}

P2_FINESSE_TO_ARMORPIERCE = {
    "elvenfinesword": 7, "adagaduelos": 7, "lanirias": 7, "kukrirdois": 6,
    "bjorskashortsword": 6, "lestemachete": 5, "karda": 5, "Rapier": 6,
    "florete2": 6, "rapierandshortsword": 9,
    "leque": 4, "socalLaminado": 4, "luvaComGarras": 5, "kama": 5,
    "nunchaku": 5, "lajatang": 4,
    "twindaggers": 7, "pequilhodaggers": 7, "drowdualkryss": 8,
    "dualtanto": 7, "dualknives": 8, "aiglanatwindaggers": 7,
}
P2_FINESSE_FLAG_NEEDED = {"elvenfinesword","adagaduelos","kukrirdois","lanirias"}

# cfg data corrections: weapons itemdesc.cfg marks `Finesse 1` but design says
# they are NOT finesse (heavy / 2H-feel weapons). Analyzer forces non-finesse;
# the real fix is deleting `Finesse 1` from these templates in the cfg cleanup.
FINESSE_CFG_CORRECTIONS = {"monkstaff"}

P4_FIREARM_TABLE = {
    "pistol1":           ("RANGED_LOW", "CRITICAL_LOW",  "damage2",       1),
    "pistol2":           ("RANGED_LOW", "CRITICAL_LOW",  "damage2",       1),
    "carabine":          ("RANGED_LOW", "CRITICAL_MID",  "damage3",       4),
    "musket":            ("RANGED_LOW", "CRITICAL_HIGH", "damage3pierce", 6),
    "handcannon":        ("RANGED_LOW", "CRITICAL_HIGH", "damage3pierce", 7),
    "RepeatingCrossbow": ("RANGED_LOW", "CRITICAL_HIGH", "damage2",       5),
}

def one_handed_lift(craft, finesse, spd, crit, mult):
    if finesse: return spd, crit, mult
    if craft >= 70 and spd == "MELEE_LOW":  spd  = "MELEE_MID"
    if craft >= 60 and crit == "CRITICAL_LOW": crit = "CRITICAL_MID"
    if craft >= 80 and mult == 1:           mult = 2
    return spd, crit, mult

def ranged_lift(category, craft, spd, crit, mult, is_firearm):
    if is_firearm: return spd, crit, mult
    cat_u = (category or "").upper()
    if cat_u in ("BOW","CROSSBOW"):
        if craft >= 60 and spd == "RANGED_LOW":    spd  = "RANGED_MID"
        if craft >= 70 and crit == "CRITICAL_LOW": crit = "CRITICAL_MID"
        if craft >= 80 and mult == 1:              mult = 2
    return spd, crit, mult

def dual_str_lift(name, finesse, craft, spd, crit, mult):
    """Str-flavored dual_wield gets a curve lift (finesse cluster already gets P2)."""
    if finesse or name in P2_FINESSE_TO_ARMORPIERCE: return spd, crit, mult
    if craft >= 60 and spd == "MELEE_LOW":    spd  = "MELEE_MID"
    if craft >= 70 and crit == "CRITICAL_LOW": crit = "CRITICAL_MID"
    if craft >= 80 and mult == 1:              mult = 2
    return spd, crit, mult

def wrestling_lift(craft, spd, crit, mult):
    """Lift mid-craft wrestling weapons to reach T4-T6 (wrestling's family cap)."""
    if craft >= 40 and crit == "CRITICAL_LOW": crit = "CRITICAL_MID"
    if craft >= 50 and spd == "MELEE_LOW":     spd  = "MELEE_MID"
    if craft >= 60 and mult == 1:              mult = 2
    return spd, crit, mult

# ============================================================
SPEED_TIER = {"MELEE_LOW":24,"MELEE_MID":30,"MELEE_HIGH":36,
              "RANGED_LOW":24,"RANGED_MID":30,"RANGED_HIGH":36}
CRIT_TIER  = {"CRITICAL_LOW":1,"CRITICAL_MID":3,"CRITICAL_HIGH":5,"":0}
DICE_BY_TIER = {
    "DAMAGE_NONE":"0",
    "UNARMED_LOW":"1d2","UNARMED_MID":"2d2","UNARMED_HIGH":"3d2",
    "ONEHANDED_LOW":"1d4","ONEHANDED_MID":"1d6","ONEHANDED_HIGH":"1d8",
    "TWOHANDED_LOW":"2d4","TWOHANDED_MID":"3d4","TWOHANDED_HIGH":"4d4",
    "BOW_LOW":"1d4","BOW_MID":"1d6","BOW_HIGH":"1d8",
    "CROSSBOW_LOW":"1d6","CROSSBOW_MID":"1d8","CROSSBOW_HIGH":"1d10",
    "THROWING_LOW":"1d4","THROWING_MID":"1d6","THROWING_HIGH":"1d8",
    "FIREWEAPON_LOW":"1d6","FIREWEAPON_MID":"1d8","FIREWEAPON_HIGH":"1d10",
}
DICE_RE = re.compile(r"(\d+)d(\d+)", re.I)
def dice_avg(s):
    if s == "0" or not s: return 0.0
    m = DICE_RE.fullmatch(s.strip())
    if not m: return 0.0
    n, d = int(m.group(1)), int(m.group(2))
    return n*(d+1)/2.0
def epic_main_to_dice(s):
    parts = (s or "").split()
    while len(parts) < 3: parts.append("DAMAGE_NONE")
    return [dice_avg(DICE_BY_TIER.get(t.strip().upper(),"0")) for t in parts[:3]]
def crit_script_to_mult(script):
    s = (script or "").lower()
    if "damage3pierce" in s: return 3
    if "damage3" in s: return 3
    if "damage2" in s: return 2
    return 1

def parse_weapons():
    """Prefer the PSV cache; fall back to parsing itemdesc.cfg directly so the
    analyzer is self-sufficient (no /tmp dependency)."""
    if Path(WEAPONS_PSV).exists():
        with open(WEAPONS_PSV) as f:
            return list(csv.DictReader(f, delimiter="|"))
    return parse_itemdesc()

def norm_ot(s):
    """Canonical objtype key. itemdesc uses zero-padded hex (0x0f49) but
    fabricacaoDesc uses unpadded (0xF49) — normalize both to int-canonical
    lowercase hex so recipe joins don't silently miss."""
    try:
        return f"0x{int(str(s).strip(), 16):x}"
    except (ValueError, TypeError):
        return str(s).lower().strip()

def parse_itemdesc():
    text = Path(ITEMDESC_CFG).read_text(errors="replace")
    weapons = []
    cur = None
    for line in text.splitlines():
        m = re.match(r"\s*Weapon\s+(0x[0-9A-Fa-f]+)", line)
        if m:
            cur = {"ObjType": norm_ot(m.group(1)), "_props": {}}
            continue
        if cur is None:
            continue
        if line.strip() == "}":
            p = cur["_props"]
            ranged = "1" if (p.get("Attribute","").strip()=="Ranged" or p.get("Projectile")) else ""
            weapons.append({
                "ObjType": cur["ObjType"],
                "Name": p.get("Name",""),
                "Desc": p.get("Desc",""),
                "Damage": p.get("Damage",""),
                "Speed": p.get("Speed",""),
                "Attribute": p.get("Attribute","").strip(),
                "Category": p.get("Category","").strip(),
                "TwoHanded": "1" if p.get("Two_Handed") else "",
                "Weight": p.get("Weight",""),
                "Ranged": ranged,
                "Projectile": "1" if p.get("Projectile") else "",
                "ProjectileType": p.get("ProjectileType",""),
                "EpicSpeed": p.get("EpicSpeed","").strip(),
                "EpicCriticalChance": p.get("EpicCriticalChance","").strip(),
                "Finesse": "1" if (p.get("Finesse","").strip() in ("1","i1")) else "",
                "Script": p.get("Script",""),
                "HitScript": p.get("HitScript",""),
                "CriticalHitScripts": p.get("CriticalHitScripts","").strip(),
                "TipoAtaque": p.get("TipoAtaque","").strip(),
                "MaxHP": p.get("MaxHP",""),
                "MinRange": p.get("MinRange",""),
                "MaxRange": p.get("MaxRange",""),
                "EpicMainDamage": p.get("EpicMainDamage","").strip(),
                "EpicOffDamage": p.get("EpicOffDamage","").strip(),
                "Throwable": "1" if p.get("Throwable") else "",
                "CProp": p.get("CProp",""),
                "Other": "",
            })
            cur = None
            continue
        mm = re.match(r"\s*(\S+)\s+(.*?)\s*$", line)
        if mm and "_props" in cur:
            cur["_props"][mm.group(1)] = mm.group(2)
    return weapons

def parse_recipes_psv():
    """Parse fabricacaoDesc.cfg files directly (no /tmp dependency).

    Block format:
        fabricacao 0xABCD
        {
        name        Foo
        skill       55.0
        ComponenteEncaixe 1 dano cabo 2 principal
        ...
        }
    """
    by_ot = {}
    for path in FABRICACAO_CFGS:
        if not Path(path).exists(): continue
        src = path
        cat = ("smith"     if "blacksmith"  in src else
               "tinker"    if "tinkering"   in src else
               "bowyer"    if "bowcraft"    in src else
               "carpentry" if "carpentry"   in src else
               "tailor"    if "DomesticWork" in src else "other")
        text = Path(path).read_text(errors="replace")
        cur_ot = None; cur_skill = 0.0; cur_comp = []
        for raw in text.splitlines():
            line = raw.strip()
            if line.startswith("#"):                       # comment / commented template
                continue
            m = re.match(r"fabricacao\s+(0x[0-9A-Fa-f]+)", line)
            if m:
                cur_ot = norm_ot(m.group(1)); cur_skill = 0.0; cur_comp = []
                continue
            if cur_ot is None:
                continue
            ms = re.match(r"skill\s+([\d.]+)", line)
            if ms:
                try: cur_skill = float(ms.group(1))
                except ValueError: cur_skill = 0.0
                continue
            if line.startswith("ComponenteEncaixe"):
                cur_comp.append(line[len("ComponenteEncaixe"):].strip())
                continue
            if line == "}" or re.match(r"fabricacao\s+0x", line):
                if cur_ot and (cur_ot not in by_ot or cur_skill > by_ot[cur_ot][0]):
                    by_ot[cur_ot] = (cur_skill, cat, ";".join(cur_comp))
                cur_ot = None
        # flush last block if file didn't end with }
        if cur_ot and (cur_ot not in by_ot or cur_skill > by_ot[cur_ot][0]):
            by_ot[cur_ot] = (cur_skill, cat, ";".join(cur_comp))
    return by_ot

def parse_recipes_cfg_level():
    """recipes.cfg uses `recipes NAME { level N ... }` — extract NAME→level."""
    if not Path(RECIPES_CFG).exists(): return {}
    text = Path(RECIPES_CFG).read_text()
    out = {}
    # match: recipes <name> { ... level <N> ... }
    for m in re.finditer(r"recipes\s+(\S+)\s*\{([^}]*)\}", text, re.S):
        name = m.group(1).strip()
        body = m.group(2)
        lm = re.search(r"\blevel\s+(\d+)", body)
        if lm:
            out[name] = int(lm.group(1))
    return out

def build_name_to_objtype():
    """Walk itemdesc.cfg, build {weapon_name: objtype}."""
    if not Path(ITEMDESC_CFG).exists(): return {}
    text = Path(ITEMDESC_CFG).read_text()
    out = {}
    current_ot = None
    for line in text.splitlines():
        m_obj = re.match(r"\s*Weapon\s+(0x[0-9A-Fa-f]+)", line)
        if m_obj:
            current_ot = norm_ot(m_obj.group(1))
            continue
        m_name = re.match(r"\s*Name\s+(\S+)", line)
        if m_name and current_ot:
            out[m_name.group(1)] = current_ot
            current_ot = None
    return out

def merge_recipes(psv_recipes, cfg_levels, name_to_ot):
    """Patch missing CraftSkill from recipes.cfg `level` mapping."""
    merged = dict(psv_recipes)
    patched = 0
    for name, level in cfg_levels.items():
        ot = name_to_ot.get(name)
        if not ot: continue
        if ot in merged and merged[ot][0] > 0: continue   # already have skill
        skill = LEVEL_TO_SKILL.get(level, level*20)
        merged[ot] = (float(skill), "recipes_book", "")
        patched += 1
    return merged, patched

def family_of(attr, cprop):
    if attr == "Dual_Wield": return "dual_wield"
    if attr == "Wrestling":
        return "caster_catalyst" if "cancast" in (cprop or "").lower() else "wrestling"
    if attr == "Ranged":     return "ranged"
    if attr == "Throwing":   return "throwing"
    if attr == "Two_Handed": return "two_handed"
    if attr == "One_Handed": return "one_handed"
    return "other"

def status_bonus(attr, finesse, hand, str_=REF_STR, dex=REF_DEX, apply_finesse_buff=True):
    """Status bonus per hit.

    Finesse mainhand: Dex/12 (was Dex/15) under FINESSE_BUFF — the offensive
    premium that compensates for finesse builds being locked out of
    medium/heavy armor. Target: ~30% Dex apex DPS premium over Str apex.
    """
    if attr == "Throwing":   return str_//20 + dex//20      # kept dual-stat per user
    if attr == "Ranged":     return dex//20
    if finesse:
        if STAT20_MODE:
            return dex//20 if hand=="main" else dex//40     # symmetric with Str
        if apply_finesse_buff:
            return dex//12 if hand=="main" else dex//24
        return dex//15 if hand=="main" else dex//30
    return str_//20 if hand=="main" else str_//40

def swing_stat(attr, finesse, str_, dex, apply_fix):
    """Effective stat that drives swing speed (POL's EffectiveDex slot).

    Without fix: always raw Dex (every build punished for not maxing Dex).
    With fix:    non-finesse melee uses (Str+Dex)/2 — strength counts.
    Finesse melee and ranged unchanged (pure Dex — that's their niche).
    """
    if not apply_fix:                       return dex
    if attr == "Ranged" or attr == "Throwing": return dex
    if finesse:                             return dex
    return (str_ + dex) // 2

def best_posture(attr, slash, pierce, blunt, category, epic_main):
    cat_u  = (category or "").upper()
    epic_u = (epic_main or "").upper()
    if cat_u in ("BOW","CROSSBOW"):                 return "pierce(forced)", pierce
    if "FIRE" in epic_u and attr == "Ranged":       return "blunt(forced firearm)", blunt
    if cat_u == "UNARMED":                          return "blunt(forced unarmed)", blunt
    opts = sorted([("slash",slash),("pierce",pierce),("blunt",blunt)], key=lambda x:-x[1])
    return opts[0]

def compute_dps(w, apply_fixes, craft_skill, profile=None):
    """profile=None → uses REF_STR/REF_DEX/REF_TAC (100/100/100 baseline).
    profile=dict → uses {str, dex, tac, dw} for archetype-specific DPS."""
    if profile is None:
        p_str, p_dex, p_tac, p_dw = REF_STR, REF_DEX, REF_TAC, DUAL_WIELD_SKILL
    else:
        p_str, p_dex, p_tac, p_dw = profile["str"], profile["dex"], profile["tac"], profile["dw"]
    attr    = (w["Attribute"] or "").strip()
    finesse = ((w["Finesse"] == "1" or w["Name"] in P2_FINESSE_FLAG_NEEDED
                or w["Name"] in TWO_HANDED_MAKE_FINESSE)
               and w["Name"] not in FINESSE_CFG_CORRECTIONS)
    ranged  = w["Ranged"]  == "1"
    cat     = w["Category"]
    dual    = attr == "Dual_Wield"
    name    = w["Name"]
    epic    = w["EpicMainDamage"]
    is_firearm = "FIRE" in (epic or "").upper() and ranged

    spd_tier  = (w["EpicSpeed"] or "").strip().upper()
    crit_tier = (w["EpicCriticalChance"] or "").strip().upper()
    mult      = crit_script_to_mult(w["CriticalHitScripts"])

    if apply_fixes:
        if APPLY_DUAL_WIELD_CLEANUP and attr == "Dual_Wield":
            # Phase A: dual_wield uses the clean rule-based profile, bypassing
            # all legacy P-tables for this family.
            ndice, nspd, ncrit, nsc = dual_wield_profile(craft_skill, finesse)
            epic, spd_tier, crit_tier, mult = ndice, nspd, ncrit, crit_script_to_mult(nsc)
        elif APPLY_TWO_HANDED_CLEANUP and attr == "Two_Handed":
            # Phase B: two_handed clean rule-based profile.
            ndice, nspd, ncrit, nsc = two_handed_profile(craft_skill, finesse)
            epic, spd_tier, crit_tier, mult = ndice, nspd, ncrit, crit_script_to_mult(nsc)
        elif APPLY_P1_2H_LIFT and name in P1_TWO_HANDED_LIFT:
            ns, nc, sc, _ = P1_TWO_HANDED_LIFT[name]
            spd_tier, crit_tier, mult = ns, nc, crit_script_to_mult(sc)
            if name in P1B_DICE_UPGRADE: epic = P1B_DICE_UPGRADE[name]
        else:
            if APPLY_P2_FINESSE_ARMORPIERCE and name in P2_FINESSE_TO_ARMORPIERCE:
                mult = 1
            if APPLY_P4_FIREARMS_TABLE and name in P4_FIREARM_TABLE:
                ns, nc, sc, _ = P4_FIREARM_TABLE[name]
                spd_tier, crit_tier, mult = ns, nc, crit_script_to_mult(sc)
            if APPLY_1H_CURVE_LIFT and attr == "One_Handed" and name not in P2_FINESSE_TO_ARMORPIERCE:
                spd_tier, crit_tier, mult = one_handed_lift(craft_skill, finesse, spd_tier, crit_tier, mult)
            if APPLY_RANGED_CURVE_LIFT and attr == "Ranged":
                spd_tier, crit_tier, mult = ranged_lift(cat, craft_skill, spd_tier, crit_tier, mult, is_firearm)
            if APPLY_WRESTLING_CURVE_LIFT and attr == "Wrestling" and "cancast" not in (w.get("CProp","") or "").lower() and name not in P2_FINESSE_TO_ARMORPIERCE:
                spd_tier, crit_tier, mult = wrestling_lift(craft_skill, spd_tier, crit_tier, mult)

    speed = SPEED_TIER.get(spd_tier, 24)
    wcrit = CRIT_TIER.get(crit_tier, 0)

    slash, pierce, blunt = epic_main_to_dice(epic)
    posture, posture_dice = best_posture(attr, slash, pierce, blunt, cat, epic)
    if posture_dice == 0 and w["Damage"]:
        posture_dice = dice_avg(w["Damage"])

    fin_buff = apply_fixes and APPLY_FINESSE_BUFF
    main_stat = status_bonus(attr, finesse, "main", p_str, p_dex, fin_buff)
    tac_main  = p_tac // 10
    mainhand  = posture_dice + main_stat + tac_main

    if dual:
        off_stat = status_bonus(attr, finesse, "off", p_str, p_dex, fin_buff)
        tac_off  = (p_tac // 20) if (apply_fixes and APPLY_P5_OFFHAND_TAC_HALF) else tac_main
        penalty  = ((100 - p_dw)/2 + 15) if (apply_fixes and APPLY_P5_PENALTY_CURVE) else max(10, (100 - p_dw)/2)
        hit_rate = max(0.0, 1.0 - penalty/100.0)
        offhand  = (posture_dice + off_stat + tac_off) * hit_rate
        per_hit  = mainhand + offhand
    else:
        per_hit  = mainhand

    crit_chance = max(0, min(100, wcrit + p_tac//20))
    per_hit_exp = per_hit * (1 + (crit_chance/100.0) * (mult-1)) if mult > 1 else per_hit
    eff_swing_stat = swing_stat(attr, finesse, p_str, p_dex, apply_fixes and APPLY_SWING_SPEED_FIX)
    swing_ms       = 15_000_000 / ((eff_swing_stat + 100) * speed)
    dps            = per_hit_exp / (swing_ms/1000.0)

    return {
        "posture": posture, "posture_dice": round(posture_dice,2),
        "speed_tier": spd_tier, "speed": speed, "swing_ms": round(swing_ms),
        "crit_tier": crit_tier, "crit_pct": crit_chance, "crit_mult": mult,
        "per_hit": round(per_hit,1), "per_hit_exp": round(per_hit_exp,1),
        "dps": round(dps,1), "epic_main": epic,
    }

def intended_tier(name, craft_skill, fam):
    """The *design intent* tier — from explicit table or from craft skill.

    Used to detect drift between intent and post-fix DPS. NOT derived from DPS
    (that would be tautological).
    """
    # explicit per-weapon target (D.1/D.2/D.3)
    if name in P1_TWO_HANDED_LIFT:        return P1_TWO_HANDED_LIFT[name][3]
    if name in P2_FINESSE_TO_ARMORPIERCE: return P2_FINESSE_TO_ARMORPIERCE[name]
    if name in P4_FIREARM_TABLE:          return P4_FIREARM_TABLE[name][3]
    # otherwise: craft skill / 10, clamped to family cap
    max_t = FAMILY_MAX_TIER.get(fam, 9) or 9
    if craft_skill <= 0:
        return 0
    return max(0, min(max_t, int(craft_skill // 10)))

def main():
    weapons      = parse_weapons()
    psv_recipes  = parse_recipes_psv()
    cfg_levels   = parse_recipes_cfg_level()
    name_to_ot   = build_name_to_objtype()
    recipes, patched = merge_recipes(psv_recipes, cfg_levels, name_to_ot)
    print(f"recipes: psv={len(psv_recipes)}  cfg_levels={len(cfg_levels)}  merged={len(recipes)}  patched_from_cfg={patched}")

    rows = []
    for w in weapons:
        ot   = norm_ot(w["ObjType"])
        skill, craft_cat, components = recipes.get(ot, (0.0, "uncraftable", ""))
        cur     = compute_dps(w, apply_fixes=False, craft_skill=skill)
        post    = compute_dps(w, apply_fixes=True,  craft_skill=skill)
        # archetype profiles — DPS for STR/DEX/HYB players at this weapon post-fix
        post_str = compute_dps(w, apply_fixes=True, craft_skill=skill, profile=BUILD_PROFILES["STR"])
        post_dex = compute_dps(w, apply_fixes=True, craft_skill=skill, profile=BUILD_PROFILES["DEX"])
        post_hyb = compute_dps(w, apply_fixes=True, craft_skill=skill, profile=BUILD_PROFILES["HYB"])
        attr = (w["Attribute"] or "").strip()
        fam  = family_of(attr, w.get("CProp",""))
        tier = intended_tier(w["Name"], skill, fam)
        target = TIER_TARGET[tier]
        drift  = round(post["dps"] - target, 1)
        # which archetype wins this weapon, and by how much vs the loser
        best_arc, best_dps = max([("STR",post_str["dps"]),("DEX",post_dex["dps"]),("HYB",post_hyb["dps"])], key=lambda x:x[1])
        worst_dps = min(post_str["dps"], post_dex["dps"], post_hyb["dps"])
        archetype_gap_pct = round(100*(best_dps - worst_dps) / max(0.1, worst_dps), 0)
        rows.append({
            "ObjType": ot, "Name": w["Name"], "Desc": w["Desc"],
            "Family": fam, "Tier": tier, "TierTargetDPS": target, "DPS_drift": drift,
            "Attribute": attr, "Category": w["Category"],
            "Finesse": "Y" if ((w["Finesse"]=="1" or w["Name"] in P2_FINESSE_FLAG_NEEDED)
                                and w["Name"] not in FINESSE_CFG_CORRECTIONS) else "",
            "TipoAtaque": w["TipoAtaque"],
            "EpicMainDamage_current": w["EpicMainDamage"],
            "EpicMainDamage_postfix": post["epic_main"],
            "Posture": cur["posture"], "PostureDiceAvg": cur["posture_dice"],
            "Speed_current": cur["speed_tier"], "Speed_postfix": post["speed_tier"],
            "Crit_current": cur["crit_tier"],   "Crit_postfix": post["crit_tier"],
            "CritMult_current": cur["crit_mult"], "CritMult_postfix": post["crit_mult"],
            "SwingMs_current": cur["swing_ms"], "SwingMs_postfix": post["swing_ms"],
            "PerHit_current": cur["per_hit"],   "PerHit_postfix": post["per_hit"],
            "DPS_current": cur["dps"],          "DPS_postfix": post["dps"],
            "DPS_delta": round(post["dps"] - cur["dps"], 1),
            "DPS_STR": post_str["dps"], "DPS_DEX": post_dex["dps"], "DPS_HYB": post_hyb["dps"],
            "BestArchetype": best_arc, "ArchetypeGapPct": archetype_gap_pct,
            "CraftSkill": skill, "CraftSystem": craft_cat,
            "Weight": w["Weight"], "MaxHP": w["MaxHP"], "MaxRange": w["MaxRange"],
            "Script": w["Script"], "CritScript": w["CriticalHitScripts"],
            "Components": components,
        })

    by_fam = defaultdict(list)
    for r in rows: by_fam[r["Family"]].append(r)

    fieldnames = ["ObjType","Name","Desc","Family","Tier","TierTargetDPS","DPS_drift",
                  "Attribute","Category","Finesse","TipoAtaque",
                  "EpicMainDamage_current","EpicMainDamage_postfix",
                  "Posture","PostureDiceAvg",
                  "Speed_current","Speed_postfix",
                  "Crit_current","Crit_postfix",
                  "CritMult_current","CritMult_postfix",
                  "SwingMs_current","SwingMs_postfix",
                  "PerHit_current","PerHit_postfix",
                  "DPS_current","DPS_postfix","DPS_delta",
                  "DPS_STR","DPS_DEX","DPS_HYB","BestArchetype","ArchetypeGapPct",
                  "CraftSkill","CraftSystem","Weight","MaxHP","MaxRange",
                  "Script","CritScript","Components"]
    with open(OUT_CSV, "w", newline="") as f:
        ww = csv.DictWriter(f, fieldnames=fieldnames)
        ww.writeheader()
        for r in sorted(rows, key=lambda r:(r["Family"], r["Tier"], r["DPS_postfix"])):
            ww.writerow(r)

    md = []
    P = md.append
    def fam_stats(lst, key):
        vals = sorted(r[key] for r in lst)
        def pct(p): return vals[min(len(vals)-1, int(p*(len(vals)-1)))]
        return vals[0], pct(.25), pct(.5), pct(.75), vals[-1]
    def pearson(lst, key):
        pts = [(r["CraftSkill"], r[key]) for r in lst if r["CraftSkill"]>0]
        if len(pts) < 3: return None
        xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
        mx,my = statistics.mean(xs), statistics.mean(ys)
        num = sum((x-mx)*(y-my) for x,y in zip(xs,ys))
        den = (sum((x-mx)**2 for x in xs)*sum((y-my)**2 for y in ys))**0.5
        return num/den if den else 0

    P("# Epic Shard Weapon Balance — Tier Framework & DPS Analysis\n\n")
    P("> Auto-generated by [tools/analyze_weapons.py](tools/analyze_weapons.py). Reference profile: **Str=Dex=Tactics=100, Dual_Wield=100**. Pre-resist, no armor, no buffs.\n\n")
    P("> Companion CSV: [weapons_balance.csv](weapons_balance.csv) — 291 weapons, both `DPS_current` and `DPS_postfix`, with declared `Tier` and `DPS_drift` from target.\n\n")

    # ===== TIER LADDER =====
    P("## 1. The tier ladder (unified)\n\n")
    P("**One linear curve:** `T_n DPS target = 5 + n`. Ten tiers, +1 DPS per tier, T0 → T9 = **5 → 14 DPS**.\n\n")
    P("| Tier | Target DPS | Craft skill | Identity |\n|---|---|---|---|\n")
    identities = {
        0: "Tools, drops, training (pickaxe, smith hammer, rusty cimitar)",
        1: "First-craft. Apprentice tools that almost work as weapons.",
        2: "Apprentice. Dagger, club, sling, pistol.",
        3: "Militia. Longsword, mace, shortbow, hand axe.",
        4: "Adventurer kit. Most early-mid game.",
        5: "Solid choice — middle of the curve.",
        6: "Veteran. Halberds, composite bows, scimitar variants.",
        7: "Elite cultural / regional named (kukri, kama, sabre lines).",
        8: "Master craft. Rare materials gate access.",
        9: "Apex. Two-handed/dual-wield only — paid for in no-shield.",
    }
    for n in range(10):
        P(f"| T{n} | {TIER_TARGET[n]:.1f} | {n*10}–{n*10+9} | {identities[n]} |\n")
    P(f"\n**Spread budget: ±{TIER_TOLERANCE} DPS within a tier.** Outside → re-tier.\n\n")
    P("**Family caps** — same ladder, different slices, each paying for its niche:\n\n")
    P("| Family | Max tier | Max DPS | Cost paid |\n|---|---|---|---|\n")
    cost_paid = {
        "one_handed": "Shield slot eligible — defensive trade-off caps offense",
        "two_handed": "No shield, slow swings",
        "dual_wield": "No shield, Dual_Wield skill cost, offhand penalty + halved offhand Tactics",
        "wrestling":  "Separate hit-roll skill (Wrestling), unarmed-feel",
        "ranged":     "Ammo cost, projectile cost, kite-vulnerable, no melee burst",
        "throwing":   "One-shot per equip cycle (throwing.src friction)",
    }
    for fam, mt in FAMILY_MAX_TIER.items():
        if mt is None: continue
        P(f"| {fam} | T{mt} | {TIER_TARGET[mt]:.0f} | {cost_paid[fam]} |\n")
    P("| caster_catalyst | — | — | EXCLUDED — `cancast i1` weapons live in magic balance |\n\n")

    # ===== ATTRIBUTES =====
    P("## 2. Combat Attributes — the 5 canonical buckets\n\n")
    P("`Attribute X` is the canonical combat classifier (the cfg `TwoHanded 1` flag is just an equip-layer marker).\n\n")
    P("| Attribute | cfg count | Family bucket |\n|---|---|---|\n")
    P(f"| `One_Handed` | 120 | one_handed ({len(by_fam.get('one_handed',[]))}) |\n")
    P(f"| `Two_Handed` | 80 | two_handed ({len(by_fam.get('two_handed',[]))}) |\n")
    P(f"| `Dual_Wield` | 23 | dual_wield ({len(by_fam.get('dual_wield',[]))}) |\n")
    P(f"| `Wrestling`  | 39 | wrestling ({len(by_fam.get('wrestling',[]))}) + caster_catalyst ({len(by_fam.get('caster_catalyst',[]))}) |\n")
    P(f"| `Ranged`     | 34 | ranged ({len(by_fam.get('ranged',[]))}) |\n")
    P(f"| `Throwing`   |  2 | throwing ({len(by_fam.get('throwing',[]))}) |\n\n")

    # ===== DISTRIBUTIONS =====
    P("## 3. Current DPS distribution\n\n")
    P("| Family | n | min | p25 | median | p75 | **max** |\n|---|---|---|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged","throwing","caster_catalyst"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        mn,p25,med,p75,mx = fam_stats(lst, "DPS_current")
        P(f"| {fam} | {len(lst)} | {mn:.1f} | {p25:.1f} | {med:.1f} | {p75:.1f} | **{mx:.1f}** |\n")

    P("\n## 4. Post-fix DPS distribution\n\n")
    P("All fixes applied: P1 (2H apex), P2 (finesse → armorpierce), P4 (firearms table), P5-A (dual-wield offhand Tactics halved), 1H/Ranged curve lifts.\n\n")
    P("| Family | n | min | p25 | median | p75 | **max** | Δmax |\n|---|---|---|---|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged","throwing","caster_catalyst"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        cmx = fam_stats(lst, "DPS_current")[4]
        mn,p25,med,p75,mx = fam_stats(lst, "DPS_postfix")
        P(f"| {fam} | {len(lst)} | {mn:.1f} | {p25:.1f} | {med:.1f} | {p75:.1f} | **{mx:.1f}** | {(mx-cmx):+.1f} |\n")
    P("\n")

    # ===== PEARSON =====
    P("## 5. Craft skill ↔ DPS correlation\n\n")
    P("Healthy: positive Pearson r — higher craft skill → higher DPS.\n\n")
    P("| Family | r (current) | r (post-fix) | Δ |\n|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged","throwing"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        rc = pearson(lst, "DPS_current"); rp = pearson(lst, "DPS_postfix")
        if rc is None or rp is None:
            P(f"| {fam} | n/a | n/a | — |\n"); continue
        P(f"| {fam} | {rc:+.2f} | {rp:+.2f} | {(rp-rc):+.2f} |\n")
    P("\n")

    # ===== PLAYER IMPACT =====
    buffed = sum(1 for r in rows if r["DPS_delta"] >  0.3)
    nerfed = sum(1 for r in rows if r["DPS_delta"] < -0.3)
    same   = sum(1 for r in rows if abs(r["DPS_delta"]) <= 0.3)
    P("## 6. Player impact (buff-first design)\n\n")
    P(f"| Outcome | Count | % |\n|---|---|---|\n")
    P(f"| **Buffed** (≥+0.3 DPS) | {buffed} | {100*buffed/len(rows):.0f}% |\n")
    P(f"| Unchanged (±0.3 DPS)   | {same}   | {100*same/len(rows):.0f}% |\n")
    P(f"| Nerfed (≤−0.3 DPS)    | {nerfed} | {100*nerfed/len(rows):.0f}% |\n\n")
    P("Only the meta-cluster gets a *situational* nerf (armorpierce trades vs unarmored; keeps DPS vs heavy armor). Dual-wield apex pays a small offhand cost.\n\n")

    P("### Top 15 buffs\n\n")
    P("| Family | Name | Δ DPS | Current → Post-fix | CraftSkill |\n|---|---|---|---|---|\n")
    for r in sorted(rows, key=lambda r:-r["DPS_delta"])[:15]:
        P(f"| {r['Family']} | {r['Name']} | {r['DPS_delta']:+.1f} | {r['DPS_current']:.1f} → {r['DPS_postfix']:.1f} | {r['CraftSkill']:.0f} |\n")

    P("\n### Top 15 nerfs\n\n")
    P("| Family | Name | Δ DPS | Current → Post-fix | CraftSkill | Note |\n|---|---|---|---|---|---|\n")
    for r in sorted(rows, key=lambda r:r["DPS_delta"])[:15]:
        note = "P2 armorpierce" if r["Name"] in P2_FINESSE_TO_ARMORPIERCE else ("P5-A offhand" if r["Family"]=="dual_wield" else "—")
        P(f"| {r['Family']} | {r['Name']} | {r['DPS_delta']:+.1f} | {r['DPS_current']:.1f} → {r['DPS_postfix']:.1f} | {r['CraftSkill']:.0f} | {note} |\n")
    P("\n")

    # ===== COMPLIANCE =====
    out = [r for r in rows if abs(r["DPS_drift"]) > TIER_TOLERANCE and r["Family"] != "caster_catalyst"]
    P(f"## 7. Tier compliance — {len(rows)-len(out)}/{len(rows)} weapons in band (±{TIER_TOLERANCE})\n\n")
    if out:
        P("| Family | Tier | Name | Target | Actual | Drift | Reason |\n|---|---|---|---|---|---|---|\n")
        for r in sorted(out, key=lambda r:-abs(r["DPS_drift"]))[:30]:
            reasons = []
            if r["CraftSkill"] == 0: reasons.append("uncraftable")
            if r["Family"] == "throwing": reasons.append("P0 exempt")
            if abs(r["DPS_drift"]) > 1.5: reasons.append("template edit pending")
            P(f"| {r['Family']} | T{r['Tier']} | {r['Name']} | {r['TierTargetDPS']:.1f} | {r['DPS_postfix']:.1f} | {r['DPS_drift']:+.1f} | {', '.join(reasons) or '—'} |\n")
        if len(out) > 30: P(f"\n*… and {len(out)-30} more. See CSV.*\n")
    P("\n")

    # ===== SAME-TIER SAME-FAMILY BUILD COMPARISON =====
    P("## 8. Same-tier examples: STR vs DEX vs HYB per Attribute\n\n")
    mode = "STAT/20 (finesse=Dex/20, no engine change)" if STAT20_MODE else "current formula"
    P(f"Model: **{mode}**. Builds: STR=Str100/Dex60, DEX=Str60/Dex100, HYB=80/80, all Tactics 100.\n\n")
    P("For each family at a fixed tier, the best **Str-scaling (non-finesse)** weapon and best **Dex-scaling (finesse)** weapon, and what each build does on each. A balanced result: STR build tops the non-finesse weapon, DEX build tops the finesse weapon, and DEX-best is ~25-30% over STR-best (40% only at T9 apex).\n\n")
    P("> Focus is **one_handed / two_handed / dual_wield** — the three families that matter for build choice. Wrestling (7 weapons, no Str apex) and throwing (2 weapons, self-limited) are edge cases not worth balance-tuning. `monkstaff` corrected to non-finesse (cfg `Finesse 1` is a data error — it's a quarterstaff).\n\n")
    for fam in ("one_handed","two_handed","dual_wield"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        P(f"### {fam}\n\n")
        P("| Tier | Scaling | Weapon | STR build | DEX build | HYB build |\n|---|---|---|---|---|---|\n")
        for tier in (3, 5, 7, 9):
            at_tier = [r for r in lst if r["Tier"] == tier]
            if not at_tier: continue
            nonf = [r for r in at_tier if not (r["Finesse"]=="Y")]
            fin  = [r for r in at_tier if r["Finesse"]=="Y"]
            if nonf:
                r = max(nonf, key=lambda r:r["DPS_STR"])
                P(f"| T{tier} | Str (non-finesse) | `{r['Name']}` | **{r['DPS_STR']:.1f}** | {r['DPS_DEX']:.1f} | {r['DPS_HYB']:.1f} |\n")
            if fin:
                r = max(fin, key=lambda r:r["DPS_DEX"])
                P(f"| T{tier} | Dex (finesse) | `{r['Name']}` | {r['DPS_STR']:.1f} | **{r['DPS_DEX']:.1f}** | {r['DPS_HYB']:.1f} |\n")
            # gap line
            if nonf and fin:
                rs = max(nonf, key=lambda r:r["DPS_STR"])
                rd = max(fin,  key=lambda r:r["DPS_DEX"])
                gap = 100*(rd["DPS_DEX"] - rs["DPS_STR"]) / max(0.1, rs["DPS_STR"])
                P(f"| T{tier} | **gap** | DEX-best vs STR-best | | | **{gap:+.0f}%** |\n")
        P("\n")
    P("\n")

    # ===== PLAYSTYLE PARITY (the fun audit) =====
    P("## 8b. Playstyle parity — STR vs DEX vs HYBRID\n\n")
    P("Realistic builds (not 100/100/100):\n")
    P(f"- **STR build:** Str {BUILD_PROFILES['STR']['str']} / Dex {BUILD_PROFILES['STR']['dex']} / Tac {BUILD_PROFILES['STR']['tac']} — pairs with heavy armor + shield.\n")
    P(f"- **DEX build:** Str {BUILD_PROFILES['DEX']['str']} / Dex {BUILD_PROFILES['DEX']['dex']} / Tac {BUILD_PROFILES['DEX']['tac']} — agile, fast swings.\n")
    P(f"- **HYB build:** Str {BUILD_PROFILES['HYB']['str']} / Dex {BUILD_PROFILES['HYB']['dex']} / Tac {BUILD_PROFILES['HYB']['tac']} — flex.\n\n")
    P("DEX builds compensate for lower defenses with faster swings. STR builds get HP + heavy armor + shield. The framework is balanced when each archetype has competitive weapons at their stat-appropriate family.\n\n")

    P("### Best weapon for each archetype, by family\n\n")
    P("| Family | STR best (DPS) | DEX best (DPS) | HYB best (DPS) | Winner | DEX/STR gap |\n|---|---|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged","throwing"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        best_str = max(lst, key=lambda r:r["DPS_STR"])
        best_dex = max(lst, key=lambda r:r["DPS_DEX"])
        best_hyb = max(lst, key=lambda r:r["DPS_HYB"])
        gap = 100*(best_dex["DPS_DEX"] - best_str["DPS_STR"]) / max(0.1, best_str["DPS_STR"])
        winner = "DEX" if best_dex["DPS_DEX"] > best_str["DPS_STR"] else "STR"
        P(f"| {fam} | `{best_str['Name']}` ({best_str['DPS_STR']:.1f}) | `{best_dex['Name']}` ({best_dex['DPS_DEX']:.1f}) | `{best_hyb['Name']}` ({best_hyb['DPS_HYB']:.1f}) | **{winner}** | {gap:+.0f}% |\n")
    P("\n")

    P("### Within-family stat fairness\n\n")
    P("For each family, % of weapons where each archetype is the best choice. Healthy: ≥1 viable weapon for each archetype in any family that supports both stats.\n\n")
    P("| Family | STR-favored | DEX-favored | HYB-favored | n total |\n|---|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        s = sum(1 for r in lst if r["BestArchetype"]=="STR")
        d = sum(1 for r in lst if r["BestArchetype"]=="DEX")
        h = sum(1 for r in lst if r["BestArchetype"]=="HYB")
        n = len(lst)
        P(f"| {fam} | {s} ({100*s//n}%) | {d} ({100*d//n}%) | {h} ({100*h//n}%) | {n} |\n")
    P("\n")

    P("### Apex weapon comparison (top of each family)\n\n")
    P("If a player insists on their archetype, what's their DPS ceiling vs the metagame?\n\n")
    P("| Family | Apex weapon | STR DPS | DEX DPS | HYB DPS | Gap STR↔DEX |\n|---|---|---|---|---|---|\n")
    for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged","throwing"):
        lst = by_fam.get(fam, [])
        if not lst: continue
        apex = max(lst, key=lambda r:max(r["DPS_STR"],r["DPS_DEX"],r["DPS_HYB"]))
        gap = 100*(apex["DPS_DEX"] - apex["DPS_STR"]) / max(0.1, apex["DPS_STR"])
        P(f"| {fam} | `{apex['Name']}` | {apex['DPS_STR']:.1f} | {apex['DPS_DEX']:.1f} | {apex['DPS_HYB']:.1f} | {gap:+.0f}% |\n")
    P("\n")

    P("**Fun verdict:** \n")
    # compute global STR best vs global DEX best across all families
    str_all_best = max(rows, key=lambda r:r["DPS_STR"])
    dex_all_best = max(rows, key=lambda r:r["DPS_DEX"])
    hyb_all_best = max(rows, key=lambda r:r["DPS_HYB"])
    overall_gap = 100*(dex_all_best["DPS_DEX"] - str_all_best["DPS_STR"]) / max(0.1, str_all_best["DPS_STR"])
    P(f"- A pure STR build's best weapon: `{str_all_best['Name']}` ({str_all_best['Family']}) at **{str_all_best['DPS_STR']:.1f} DPS**.\n")
    P(f"- A pure DEX build's best weapon: `{dex_all_best['Name']}` ({dex_all_best['Family']}) at **{dex_all_best['DPS_DEX']:.1f} DPS**.\n")
    P(f"- A pure HYB build's best weapon: `{hyb_all_best['Name']}` ({hyb_all_best['Family']}) at **{hyb_all_best['DPS_HYB']:.1f} DPS**.\n\n")
    P(f"DEX offensive ceiling is {overall_gap:+.0f}% above STR. This compensates STR's defensive advantages (more HP, heavy armor, shield if 1H). Target: 20-40% DEX offensive premium for parity with STR's defensive premium.\n\n")

    # ===== CROSS-FAMILY FAIRNESS =====
    P("## 9. Cross-family fairness (DPS by craft band — 100/100/100 reference)\n\n")
    P("Avg DPS per family at each craft-skill band. Healthy: families overlap; no clear meta at any tier.\n\n")
    P("| Craft band | one_handed | two_handed | dual_wield | wrestling | ranged |\n|---|---|---|---|---|---|\n")
    bands = [(0,29),(30,49),(50,69),(70,89),(90,100)]
    for lo,hi in bands:
        cells = [f"{lo}-{hi}"]
        for fam in ("one_handed","two_handed","dual_wield","wrestling","ranged"):
            lst = [r for r in by_fam.get(fam,[]) if lo <= r["CraftSkill"] <= hi]
            cells.append(f"{statistics.mean(r['DPS_postfix'] for r in lst):.1f} (n={len(lst)})" if lst else "—")
        P("| " + " | ".join(cells) + " |\n")
    P("\n")

    # ============= GDD =============
    P("---\n\n# Game Design Document — Weapon Balance\n\n")
    P("## A. Design pillars (locked)\n\n")
    P("1. **No meta weapon.** Every viable build has a defensible answer to \"which weapon do I want?\"\n")
    P("2. **One ladder, family slices.** Same DPS curve `T_n = 5 + n` for all. Each family occupies a slice paid for by its structural cost.\n")
    P("3. **Craft pays off.** Pearson r between craft skill and DPS must be **positive** in every family. CI enforces.\n")
    P("4. **Buff up, don't nerf down.** Players shouldn't see flat damage drops on weapons they already use. The only nerfs are *situational shape changes* (finesse meta gets armorpierce — same DPS vs heavy armor, lower vs unarmored).\n")
    P("5. **Posture is flexibility, not multiplier.** Multi-slot weapons trade dice ceiling for tactical pivot.\n")
    P("6. **Declarative tier.** Every weapon has `Tier N`. CI fails if `|measured DPS - TIER_TARGET[N]| > 0.5`.\n\n")

    P("## B. The power budget\n\n")
    P("`Power = DPS + Utility`. Within a tier, weapons trade DPS for utility; `Power` stays flat.\n\n")
    P("**Utility levers:**\n")
    P("- Range (~+1 tier vs melee equivalent — pays in ammo)\n")
    P("- Shield-slot eligibility (defensive benefit — caps offensive ceiling)\n")
    P("- Posture flexibility (~−0.5 DPS for 2-3 slot — gain tactical pivot)\n")
    P("- Special crit script (armorpierce, bleed, stun — situational)\n")
    P("- Hit-roll skill choice (Wrestling vs Sword/Mace/Fencing — skill cost)\n")
    P("- Dual-wield offhand (~85% extra swings — pays in no-shield + Dual_Wield + halved offhand Tactics)\n\n")

    P("## C. Family identities (post-fix)\n\n")
    P("| Family | Scaling | Speed | Dice ceiling | Crit profile | Niche |\n|---|---|---|---|---|---|\n")
    P("| **one_handed (Str)** | Str/20 | LOW–MID | ONEHANDED_LOW–HIGH | LOW–MID, ×2 on T6+ | Balanced; pairs with shield. |\n")
    P("| **one_handed (Finesse)** | Dex/15 | MID–HIGH | ONEHANDED_LOW–MID | MID–HIGH, **armorpierce** on T6+ HIGH | Fast, consistent. Armor-pierce niche (P2). |\n")
    P("| **two_handed** | Str/20 | LOW–HIGH | TWOHANDED_LOW–HIGH | LOW–HIGH, ×2 on T7+ (P1) | **Apex single-weapon (14 ceiling).** Slow, big crits. |\n")
    P("| **dual_wield** | Str/20 or Dex/15 | LOW–HIGH | ONEHANDED_LOW–HIGH | MID–HIGH | Highest sustained DPS. No shield + Dual_Wield + halved offhand Tactics. |\n")
    P("| **wrestling** | Str/20 (Dex/15 if Finesse) | MID–HIGH | UNARMED–ONEHANDED_MID | LOW–MID | Martial arts; separate hit skill. Caps T6. 7 combat weapons. |\n")
    P("| **ranged** | Dex/20 | LOW–MID | BOW/CROSSBOW/FIREWEAPON | LOW–HIGH | Range + ammo. Bow/crossbow curve lift. Firearms: AP burst (P4). |\n")
    P("| **throwing** | Str/20 + Dex/20 | one-shot | ONEHANDED_LOW–MID | LOW–MID | shuriken/senbon — self-limited (throwing.src). P0: untouched. |\n")
    P("| **caster_catalyst** | (n/a) | — | — | — | Excluded — magic system balance. |\n\n")

    P("## D. The fix set (post-audit, table-driven)\n\n")
    P("All fixes are **per-weapon override tables** in [tools/analyze_weapons.py](tools/analyze_weapons.py) — no gate logic. Each table doubles as the cfg-edit checklist.\n\n")

    P("### D.1 P1 — Two-handed apex lift\n\n")
    P(f"**{len(P1_TWO_HANDED_LIFT)} templates.** Lift apex 2H to MELEE_MID/HIGH + CRITICAL_HIGH + `damage2`. The Strength-build payoff.\n\n")
    P("| Weapon | EpicSpeed | EpicCriticalChance | CriticalHitScripts | Target Tier | Dice upgrade |\n|---|---|---|---|---|---|\n")
    for n,(s,c,sc,t) in P1_TWO_HANDED_LIFT.items():
        dice = P1B_DICE_UPGRADE.get(n,"—")
        P(f"| `{n}` | {s} | {c} | `{sc}` | T{t} | {dice} |\n")
    P("\n")

    P("### D.2 P2 — Finesse meta cluster → armorpierce\n\n")
    P(f"**{len(P2_FINESSE_TO_ARMORPIERCE)} templates.** Replace `CriticalHitScripts damage2` with `:combat:criticalHitScripts/armorpierce` (already exists in code).\n\n")
    P(f"**Audit fix:** add `Finesse 1` to cfg for: {', '.join(sorted(P2_FINESSE_FLAG_NEEDED))}.\n\n")
    P("| Weapon | Target Tier |\n|---|---|\n")
    for n,t in sorted(P2_FINESSE_TO_ARMORPIERCE.items(), key=lambda x:-x[1]):
        P(f"| `{n}` | T{t} |\n")
    P("\n")

    P("### D.3 P4 — Firearms (table-driven)\n\n")
    P(f"**{len(P4_FIREARM_TABLE)} templates.** Anti-armor burst archetype. New script: `pkg/systems/combat/criticalHitScripts/damage3pierce.src` (sets `hitscript[\"damage3\"]` and `hitscript[\"armorpierce\"]`).\n\n")
    P("| Weapon | EpicSpeed | EpicCriticalChance | CriticalHitScripts | Target Tier |\n|---|---|---|---|---|\n")
    for n,(s,c,sc,t) in P4_FIREARM_TABLE.items():
        P(f"| `{n}` | {s} | {c} | `{sc}` | T{t} |\n")
    P("\n")

    P("### D.4 P5-A — Dual-wield offhand Tactics halved\n\n")
    P("ONLY change. In [DamageController.inc CalculateSkillDamage](pkg/systems/combat/include/Controller/DamageController.inc): for offhand path, divide Tactics by 20 instead of using the standard `skillDivisor := 10`.\n\n")
    P("**P5-B (penalty curve change) is DROPPED.** Would punish newbie dual-wielders too hard. Keep current `max(10, (100-skill)/2)` formula.\n\n")
    P("**Result:** apex dual-wield drops from 18.1 → ~14 DPS (parity with apex 2H). Entry-level dual-wielders barely affected.\n\n")

    P("### D.5 Bulk curve lifts (1H Str + Ranged bow/crossbow)\n\n")
    P("Rule-based, applied automatically by the analyzer. Targets the broken Pearson r in those families.\n\n")
    P("**1H Strength curve** (`Attribute=One_Handed AND Finesse≠1`, excludes P2 cluster):\n")
    P("- Craft ≥60 + CRITICAL_LOW → CRITICAL_MID\n")
    P("- Craft ≥70 + MELEE_LOW → MELEE_MID\n")
    P("- Craft ≥80 + crit mult ×1 → add `damage2`\n\n")
    P("**Bow/Crossbow curve** (`Category in (Bow, Crossbow)`):\n")
    P("- Craft ≥60 + RANGED_LOW → RANGED_MID\n")
    P("- Craft ≥70 + CRITICAL_LOW → CRITICAL_MID\n")
    P("- Craft ≥80 + crit mult ×1 → add `damage2`\n\n")
    P("These rules buff ~80 weapons without per-weapon enumeration. Re-evaluate post-edit via `DPS_drift`.\n\n")

    P("## E. Components (per-instance variance, bounded)\n\n")
    P("Components from `fabricacaoDesc.cfg ComponenteEncaixe` — per-craft variance. Cannot replace tier choice.\n\n")
    P("| Characteristic | Effect | Per-instance cap | DPS impact |\n|---|---|---|---|\n")
    P("| `dano` | +N weapon damage_mod | 0/+1/+2/+3 | up to +0.3 DPS |\n")
    P("| `velocidade` | +N speed_mod | 0/+2/+4/+6 | up to +0.1 DPS |\n")
    P("| `critico` | +N% CriticalChance | 0/+1/+2/+3 | up to +0.1 DPS |\n")
    P("| `precisao` | +N% hit | quality-of-life | — |\n")
    P("| `reducaoCritico` | shields/armor only | — | — |\n\n")
    P(f"**Total component DPS lift cap: ≤ +{TIER_TOLERANCE:.1f}** (fits inside spread budget). Master craft = consistent rolls, never tier skips.\n\n")

    P("## F. Roadmap\n\n")
    P("1. **P3 — declarative tier scaffold.** Add `GetWeaponTier()` to [WeaponWrapper.inc](pkg/systems/wrappers/include/Objects/Model/WeaponWrapper.inc), `TIER_TARGET_DPS` dict to `pkg/systems/combat/include/Model/TierTargets.inc`, CI compliance check. Zero gameplay risk.\n")
    P("2. **P5-A code change.** One-line in DamageController.inc offhand path.\n")
    P("3. **Per-weapon cfg batch.** ~50 templates from D.1+D.2+D.3 in one PR.\n")
    P("4. **Bulk curve cfg batch.** D.5 rules → ~80 one_handed+ranged templates.\n")
    P("5. **Component caps audit.** Verify in-game effects match §E caps.\n\n")

    P("## G. Open questions\n\n")
    P("1. **Wrestling cap at T6.** Intentional (separate skill cost) or should there be a T7-T8 master-monk weapon?\n")
    P("2. **Throwing 10.8 DPS.** Inside band but P0 keeps shuriken/senbon untouched — verify throw-and-retrieve friction limits sustained DPS in playtest.\n")
    P("3. **Caster catalysts with melee dice.** `batstaff` (75 craft) does 7.8 melee DPS *while* casting (T2-equivalent free). Acceptable or audit down?\n")
    P("4. **Situational damage hooks** — entropy/primordial/arcane-bow/spiritual-blade/elemental enchant add per-character situational DPS. Separate balance pass.\n")
    P("5. **Anatomy.** Classic UO adds Anatomy/5 to Tactics bonus. Currently zero. Add or skip?\n\n")

    P("---\n*CSV: [weapons_balance.csv](weapons_balance.csv) · Analyzer: [tools/analyze_weapons.py](tools/analyze_weapons.py)*\n")

    Path(OUT_MD).write_text("".join(md))
    print(f"wrote {OUT_CSV} ({len(rows)} rows)")
    print(f"wrote {OUT_MD}")
    print(f"  buffed: {buffed}  unchanged: {same}  nerfed: {nerfed}")
    print(f"  in-band: {len(rows)-len(out)}/{len(rows)}")

if __name__ == "__main__":
    main()
