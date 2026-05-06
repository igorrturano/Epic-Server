use uo;
use os;

include ":wrappers:Objects/Model/MobileWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":crafting:/include/caracteristicasFabricacao";
include ":crafting:/include/util/fabricacaoConstants";
include ":equipsys:utils/consts";

// Returns the equipped magic-focus ObjRef (staff/wand/spellbook) or 0 if none.
// Containers (existing grimorios) cast via cancast cprop and aren't WEAPON-slot,
// so this returns 0 for them — wrap-style spellbooks (Category SPELLBOOK) only.
function _GetEquippedFocusItem(caster)
	var mw := MobileWrapper(caster);
	if (mw is @Exception)
		return 0;
	endif
	var weapon := mw.GetEquipBySlot(WEAPON);
	if (!weapon || weapon is @Exception)
		return 0;
	endif
	var cat := weapon.GetCategory();
	if (cat != WEAPON_CATEGORY::STAFF &&
	    cat != WEAPON_CATEGORY::WAND &&
	    cat != WEAPON_CATEGORY::SPELLBOOK)
		return 0;
	endif
	return weapon.GetObjRef();
endfunction

// Returns the elemental damage bonus for a spell of the given damageType,
// based on the caster's equipped magic focus (staff/wand/spellbook) and the
// `dano <element>` sub-value baked into its crafted properties.
function GetMagicItemElementalBonus(caster, damageType)
	var item := _GetEquippedFocusItem(caster);
	if (!item)
		return 0;
	endif
	var subKey;
	case (damageType)
		DMG_FIRE:    subKey := SUBCARACTERISTICA_FABRICACAO_FOGO;
		DMG_COLD:    subKey := SUBCARACTERISTICA_FABRICACAO_GELO;
		DMG_POISON:  subKey := SUBCARACTERISTICA_FABRICACAO_VENENO;
		DMG_ENERGY:  subKey := SUBCARACTERISTICA_FABRICACAO_ENERGIA;
		default:     return 0;
	endcase
	return CInt(PegaSubValoresFabricacao(item, subKey));
endfunction

// Returns the sacredWood difficulty reduction (in skill-check points) granted
// by the caster's equipped magic focus.
function GetMagicItemSacredWoodBonus(caster)
	var item := _GetEquippedFocusItem(caster);
	if (!item)
		return 0;
	endif
	return CInt(PegaValorFabricacao(item, CARACTERISTICA_FABRICACAO_SACRED_WOOD));
endfunction

// Returns extra duration (seconds) added to Velkyn-school effect application,
// granted by the caster's equipped magic focus (e.g. Cogumelo staff).
function GetMagicItemVelkynDurationBonus(caster)
	var item := _GetEquippedFocusItem(caster);
	if (!item)
		return 0;
	endif
	return CInt(PegaValorFabricacao(item, CARACTERISTICA_FABRICACAO_BONUS_VELKYN));
endfunction
