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
// Category roll: 33% magic / 33% sacred / 33% cursed.
//   - Magic   → identified via Arcana skill, then bonuses apply on equip.
//   - Sacred  → identified via Theology, needs +entropy >= tier*100 to equip.
//   - Cursed  → identified via Theology, bonuses apply on equip immediately
//               but can ONLY be unequipped via the priest spell
//               :spells:spell_logic/priest/removermaldicao.
// Every magic item carries BOTH a spell (if one matches the
//   circle window) AND a +tier vital mod on hits / mana / stam
//   (one rolled per item). No jewelry-vs-spellstick distinction.
// Resulting name composed via Loot_GenerateMagicName().
// ============================================================

const MAGIC_CHARGES        := 10;
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
	return array{
		"ring",
		"bracelet",
		"earrings",
		"amuleto",
		"longnecklace",
		"circlet",
		"staff",
		"Scepter",
		"WizardHat"
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

// Vital bonus by chest tier. Capped low so 5 jewelry slots stack to at
// most +15 (vs +25 under the old `tier` scaling), and chest tiers 1-2
// give no vital bonus at all — those drops are spell-only.
function Loot_VitalBonusForTier(tier)
	case (tier)
		1: return 0;
		2: return 0;
		3: return 1;
		4: return 2;
		5: return 3;
	endcase
	return 0;
endfunction

// Vital mod application — three category strategies:
//   sacred  → +N to ONE vital. Pure upside (entropy gate is the cost).
//   magic   → +N to one vital, -N to a DIFFERENT vital. Tradeoff item —
//             forces the player to pick a loadout per fight instead of
//             stacking 5 magic rings for pure upside.
//   cursed  → -5 * tier_bucket on one vital. T3=-5, T4=-10, T5=-15.
//             High risk, the priest spell `removermaldicao` is the way
//             out. Custom/lore curses with worse penalties stay GM-made.
function Loot_ApplyVitalMod(byref state, tier)
	if (state.category == "cursed")
		var bucket := Loot_VitalBonusForTier(tier);  // 1/2/3 at T3/T4/T5
		if (bucket <= 0)
			return;
		endif
		var penalty := -5 * bucket;
		case (RandomInt(3))
			0: state.hitsmod := penalty;
			1: state.manamod := penalty;
			2: state.stammod := penalty;
		endcase
		return;
	endif

	var amount := Loot_VitalBonusForTier(tier);
	if (amount <= 0)
		return;
	endif

	if (state.category == "magic")
		// Pick two distinct vital slots — one gets +amount, one gets -amount.
		var plus_idx  := RandomInt(3);
		var minus_idx := (plus_idx + 1 + RandomInt(2)) % 3;  // guaranteed != plus_idx
		case (plus_idx)
			0: state.hitsmod := amount;
			1: state.manamod := amount;
			2: state.stammod := amount;
		endcase
		case (minus_idx)
			0: state.hitsmod := -amount;
			1: state.manamod := -amount;
			2: state.stammod := -amount;
		endcase
		return;
	endif

	// Sacred (and any future pure-upside category) — one vital, +amount.
	case (RandomInt(3))
		0: state.hitsmod  := amount;
		1: state.manamod  := amount;
		2: state.stammod  := amount;
	endcase
endfunction

function Loot_RollMagicCategory(byref state, tier)
	// Three equal buckets: plain magic / sacred / cursed.
	case (RandomInt(3))
		0:
			state.category := "magic";
		1:
			state.category          := "sacred";
			state.entropy_threshold := tier * MAGIC_ENTROPY_PER_TIER;
		2:
			state.category          := "cursed";
			state.entropy_threshold := -1 * tier * MAGIC_ENTROPY_PER_TIER;
	endcase
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
	var base_name := bases[RandomInt(bases.size()) + 1];

	var item := CreateItemInContainer(container, base_name, 1);
	if (!item || item == error || item.errortext)
		return 0;
	endif

	var state := Loot_BuildMagicState();

	// Category first — it decides whether the vital mod is a bonus
	// (magic / sacred) or a penalty (cursed).
	Loot_RollMagicCategory(state, tier);

	// Vital bonus / penalty on hits / mana / stam (one rolled per item).
	Loot_ApplyVitalMod(state, tier);

	// Spell-stick effect if one matches the tier circle window.
	var spell := Loot_PickMagicSpell(tier);
	if (spell)
		state.effect_type   := "spell";
		state.effect_id     := spell.id;
		state.effect_name   := spell.name;
		state.effect_script := spell.script;
		state.effect_target := spell.target;
		state.charges       := MAGIC_CHARGES;
	endif

	ApplyEpicProperties(item, state);
	item.name := Loot_GenerateMagicName(item.desc);
	SetObjProperty(item, "epic_magic_loot", 1);
	return item;
endfunction
