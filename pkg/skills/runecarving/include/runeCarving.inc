use uo;

include "include/say";

// Gem objtype constants — these 6 grant a matching-gem bonus.
// Carvability is determined by IsAGem() in :itemutils:itemtypes (which
// covers all 9 raw gems). Star sapphire / citrine / tourmaline can be
// carved but contribute only the base theology scaling.
const GEM_DIAMOND := 0xF30;
const GEM_RUBY := 0xF13;
const GEM_SAPPHIRE := 0xF11;
const GEM_AMETHYST := 0xF16;
const GEM_EMERALD := 0xF10;
const GEM_AMBER := 0xF25;

// Rune type constants
enum class RuneType
	PROTECTION := "Proteção Física",
	FIRE_RESIST := "Resistência ao Fogo",
	COLD_RESIST := "Resistência ao Frio",
	ENERGY_RESIST := "Resistência à Energia",
	POISON_RESIST := "Resistência ao Veneno",
	HOLY_WEAPON := "Arma Sagrada"
endenum

// Carved rune objtypes
const CARVED_RUNE_PROTECTION := 0xc893;
const CARVED_RUNE_FIRE := 0xc894;
const CARVED_RUNE_COLD := 0xc895;
const CARVED_RUNE_ENERGY := 0xc896;
const CARVED_RUNE_POISON := 0xc897;
const CARVED_RUNE_HOLY := 0xc898;

/*
 * Retorna o bonus de material para runas DE ARMADURA (+5%) se o tipo de gema
 * casa com o tipo de runa. Escala % do redesign — gem matching contribui ~20%
 * do total max possivel: base 20 + gem 5 = 25. Ver design_magia.md.
 *
 * NAO usar para HOLY_WEAPON — runas de arma seguem escala flat legacy via
 * getGemBonusFlat() (weapons foram mantidos flat por enquanto, fora do redesign).
 */
function getGemBonus(gemtype, runeType)
	case (runeType)
		RuneType::PROTECTION:
			return (gemtype == GEM_DIAMOND) ? 5 : 0;
		RuneType::FIRE_RESIST:
			return (gemtype == GEM_RUBY) ? 5 : 0;
		RuneType::COLD_RESIST:
			return (gemtype == GEM_SAPPHIRE) ? 5 : 0;
		RuneType::ENERGY_RESIST:
			return (gemtype == GEM_AMETHYST) ? 5 : 0;
		RuneType::POISON_RESIST:
			return (gemtype == GEM_EMERALD) ? 5 : 0;
		default:
			return 0;
	endcase
endfunction

/*
 * Bonus flat (+1) para runas de arma (HOLY_WEAPON). Escala legacy preservada
 * pois weapon damage nao foi convertido para % no redesign atual.
 */
function getGemBonusFlat(gemtype, runeType)
	case (runeType)
		RuneType::HOLY_WEAPON:
			return (gemtype == GEM_AMBER) ? 1 : 0;
		default:
			return 0;
	endcase
endfunction

/*
 * Returns the correct carved rune objtype based on rune type
 */
function getRuneObjtype(runeType)
	case (runeType)
		RuneType::PROTECTION:    return CARVED_RUNE_PROTECTION;
		RuneType::FIRE_RESIST:   return CARVED_RUNE_FIRE;
		RuneType::COLD_RESIST:   return CARVED_RUNE_COLD;
		RuneType::ENERGY_RESIST: return CARVED_RUNE_ENERGY;
		RuneType::POISON_RESIST: return CARVED_RUNE_POISON;
		RuneType::HOLY_WEAPON:   return CARVED_RUNE_HOLY;
		default:            return 0;
	endcase
endfunction

/*
 * Applies the rune bonus to an item.
 * Bonus properties are read by ResistModRuna inside ArmorWrapper constructor.
 */
function applyRuneBonus(item, runeType, bonus, charges)
	SetObjProperty(item, "priestrune", runeType);
	SetObjProperty(item, "runecharges", charges);

	case (runeType)
		RuneType::PROTECTION:
			SetObjProperty(item, "runePhysBonus", bonus);
		RuneType::FIRE_RESIST:
			SetObjProperty(item, "runeFireBonus", bonus);
		RuneType::COLD_RESIST:
			SetObjProperty(item, "runeColdBonus", bonus);
		RuneType::ENERGY_RESIST:
			SetObjProperty(item, "runeEnergyBonus", bonus);
		RuneType::POISON_RESIST:
			SetObjProperty(item, "runePoisonBonus", bonus);
		RuneType::HOLY_WEAPON:
			SetObjProperty(item, "blessedbonus", bonus);
	endcase

	IncRevision(item);
	return 1;
endfunction

/*
 * Called when rune charges are depleted.
 * Preserves priestrune and bonus properties so Imbuir Protecao can recharge them.
 * weardown.inc already sets runecharges to 0; ResistModRuna checks > 0 before applying.
 */
function removeRuneBonus(item, runeType)
	IncRevision(item);
	return 1;
endfunction

function getRuneSound(runeType)
	case (runeType)
		RuneType::PROTECTION:
			return 0x01ef;
		RuneType::FIRE_RESIST:
			return 0x5cb;
		RuneType::COLD_RESIST:
			return 0x5c8;
		RuneType::ENERGY_RESIST:
			return 0x666;
		RuneType::POISON_RESIST:
			return 0x0206;
		RuneType::HOLY_WEAPON:
			return 0x01ea;
		default:
			return SFX_SPELL_HEAL;
	endcase
endfunction