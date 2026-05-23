use uo;
use cfgfile;
use util;

include ":epicitem:properties";
include ":loot:magicNameGen";

// ============================================================
// Loot_CreateMagicSpellItem(container, tier)
//
// Builds one magic spell-stick (or vital-mod jewelry) item and
// drops it in the container. Wraps ApplyEpicProperties so the
// item behaves identically to one crafted via .epicitem.
//
// Tier window for spell circle (sliding):
//   T1 = circles 0-1   T2 = 0-2   T3 = 1-3   T4 = 2-4   T5 = 3-5
//
// School whitelist:
//   Cantrips, Evocatium, Ilusao, Necromancia
//   (deliberately excludes class-locked schools that reference
//    caster class state — Paladin/DemonHunter/Druid/Shaman/Swordmage)
//
// Target whitelist: Unico, Area (no Self — self-buffs on item
//   sticks lead to surprise behavior).
//
// Charges: flat 10 per item, all tiers.
// Sacred/cursed roll: 15% / 15% / 70%. Threshold = +/- (tier*100).
// Jewelry split: 50% spell-stick, 50% vital-mod = +tier.
// Resulting name composed via Loot_GenerateMagicName().
// ============================================================

const MAGIC_CHARGES        := 10;
const MAGIC_SACRED_PCT     := 15;
const MAGIC_CURSED_PCT     := 15;
const MAGIC_ENTROPY_PER_TIER := 100;

function Loot_MagicCircleMin(tier)
	case (tier)
		1: return 0;
		2: return 0;
		3: return 1;
		4: return 2;
		5: return 3;
	endcase
	return 0;
endfunction

function Loot_MagicCircleMax(tier)
	case (tier)
		1: return 1;
		2: return 2;
		3: return 3;
		4: return 4;
		5: return 5;
	endcase
	return 1;
endfunction

function Loot_MagicSchoolConfigs()
	// path -> internal school label (matches LoadSpellList in epicitemGump.inc).
	return array{
		struct{ path := ":spells:config/cantrips",   school := "Cantrips"   },
		struct{ path := ":spells:config/evocation",  school := "Evocatium"  },
		struct{ path := ":spells:config/illusion",   school := "Ilusao"     },
		struct{ path := ":spells:config/necromancy", school := "Necromancia"}
	};
endfunction

function Loot_MagicItemBases()
	// kind: "jewelry" = eligible for vital-mod alternative
	//       "spellstick" = always rolls a spell
	return array{
		struct{ name := "ring",         kind := "jewelry"   },
		struct{ name := "bracelet",     kind := "jewelry"   },
		struct{ name := "earrings",     kind := "jewelry"   },
		struct{ name := "amuleto",      kind := "jewelry"   },
		struct{ name := "longnecklace", kind := "jewelry"   },
		struct{ name := "circlet",      kind := "jewelry"   },
		struct{ name := "staff",        kind := "spellstick"},
		struct{ name := "Scepter",      kind := "spellstick"},
		struct{ name := "WizardHat",    kind := "spellstick"}
	};
endfunction

function Loot_PickMagicSpell(tier)
	var circle_min := Loot_MagicCircleMin(tier);
	var circle_max := Loot_MagicCircleMax(tier);

	var pool := array{};
	foreach cfg_info in Loot_MagicSchoolConfigs()
		var cfg := ReadConfigFile(cfg_info.path);
		if (!cfg || cfg.errortext)
			continue;
		endif

		foreach spell_id in (GetConfigIntKeys(cfg))
			var elem := FindConfigElem(cfg, spell_id);
			if (!elem)
				continue;
			endif

			var script := GetConfigString(elem, "SpellScript");
			if (!script || script == "")
				continue;
			endif

			var circle := CInt(GetConfigString(elem, "Circle"));
			if (circle < circle_min || circle > circle_max)
				continue;
			endif

			var targ_kind := GetConfigString(elem, "Target");
			if (!targ_kind || targ_kind == "" || targ_kind == "Self")
				continue;
			endif

			var entry := struct{};
			entry.+id     := spell_id;
			entry.+name   := GetConfigString(elem, "Name");
			entry.+school := cfg_info.school;
			entry.+target := targ_kind;
			entry.+script := script;
			entry.+circle := circle;
			pool.Append(entry);
		endforeach
	endforeach

	if (pool.size() == 0)
		return 0;
	endif
	return pool[RandomInt(pool.size()) + 1];
endfunction

function Loot_BuildMagicState()
	// Mirrors CreateEpicItemState in epicitemGump.inc so ApplyEpicProperties
	// gets every field it expects. Unset numerics default to 0.
	var state := struct{};
	state.+category          := "magic";
	state.+strmod            := 0;
	state.+dexmod            := 0;
	state.+intmod            := 0;
	state.+hitsmod           := 0;
	state.+manamod           := 0;
	state.+stammod           := 0;
	state.+ColdResistMod     := 0;
	state.+FireResistMod     := 0;
	state.+PhysicalResistMod := 0;
	state.+EnergyResistMod   := 0;
	state.+PoisonResistMod   := 0;
	state.+entropy_threshold := 0;
	state.+effect_type       := "none";
	state.+effect_id         := "";
	state.+effect_name       := "";
	state.+effect_script     := "";
	state.+effect_target     := "Self";
	state.+charges           := 0;
	state.+cooldown          := 0;
	state.+duration          := 0;
	state.+destroy_on_use    := 0;
	return state;
endfunction

function Loot_ApplyJewelryVitalMod(byref state, tier)
	case (RandomInt(3))
		0: state.hitsmod  := tier;
		1: state.manamod  := tier;
		2: state.stammod  := tier;
	endcase
endfunction

function Loot_RollSacredOrCursed(byref state, tier)
	var roll := RandomInt(100);
	if (roll < MAGIC_SACRED_PCT)
		state.category          := "sacred";
		state.entropy_threshold := tier * MAGIC_ENTROPY_PER_TIER;
	elseif (roll < MAGIC_SACRED_PCT + MAGIC_CURSED_PCT)
		state.category          := "cursed";
		state.entropy_threshold := -1 * tier * MAGIC_ENTROPY_PER_TIER;
	endif
endfunction

// Parse and execute one `MagicSpell <qty_dice> <chance> <tier>` line.
// Rolls chance once for the whole line, then spawns `qty` items.
function Loot_RollMagicSpellLine(container, line)
	var parts := SplitWords(line);
	if (parts.size() < 1)
		return 0;
	endif

	var qty_dice := parts.size() >= 1 ? CStr(parts[1]) : "1";
	var chance   := parts.size() >= 2 ? CDbl(parts[2]) : 100.0;
	var tier     := parts.size() >= 3 ? CInt(parts[3]) : 1;

	if (chance < 100.0)
		var roll := RandomInt(10000) + 1;
		if (CDbl(roll) > chance * 100.0)
			return 0;
		endif
	endif

	var qty := CInt(RandomDiceRoll(qty_dice));
	if (qty < 1)
		qty := 1;
	endif

	var made := 0;
	for i := 1 to qty
		if (Loot_CreateMagicSpellItem(container, tier))
			made := made + 1;
		endif
		sleepms(2);
	endfor
	return made;
endfunction

function Loot_CreateMagicSpellItem(container, tier)
	if (tier < 1)
		tier := 1;
	endif
	if (tier > 5)
		tier := 5;
	endif

	var bases := Loot_MagicItemBases();
	var base  := bases[RandomInt(bases.size()) + 1];

	var item := CreateItemInContainer(container, base.name, 1);
	if (!item || item == error || item.errortext)
		return 0;
	endif

	var state := Loot_BuildMagicState();

	// Jewelry can roll vital-mod instead of spell-stick (50/50).
	var roll_spell := 1;
	if (base.kind == "jewelry" && RandomInt(2) == 0)
		roll_spell := 0;
	endif

	if (roll_spell)
		var spell := Loot_PickMagicSpell(tier);
		if (!spell)
			// No spell matched the window — fall back to vital-mod so the
			// drop still feels magical instead of being a plain ring.
			Loot_ApplyJewelryVitalMod(state, tier);
		else
			state.effect_type   := "spell";
			state.effect_id     := spell.id;
			state.effect_name   := spell.name;
			state.effect_script := spell.script;
			state.effect_target := spell.target;
			state.charges       := MAGIC_CHARGES;
		endif
	else
		Loot_ApplyJewelryVitalMod(state, tier);
	endif

	Loot_RollSacredOrCursed(state, tier);

	ApplyEpicProperties(item, state);
	item.name := Loot_GenerateMagicName(item.desc);
	SetObjProperty(item, "epic_magic_loot", 1);
	return item;
endfunction
