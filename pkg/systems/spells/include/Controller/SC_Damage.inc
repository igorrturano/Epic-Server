use uo;

include ":wrappers:Objects/Model/MobileWrapper";
include ":combat:Model/Damage";
include ":combat:Controller/DamageController";
include "MagicItemBonus";

/**
 * SC_DealDamage — canonical entry point for spell damage.
 *
 * Thin facade over DealDamage. Adds the equipped-focus elemental bonus
 * and maps the legacy DMG_* damage-type strings to the new DAMAGE_TYPE
 * enum so callers don't have to change.
 *
 * Resist, application, ability hooks (AFTER_DAMAGE / ON_KILL), forensics,
 * lastHit tracking and #vinganca are all handled by DealDamage.
 *
 * @param caster       Mobile - the attacker
 * @param targ         Mobile - the victim
 * @param spellId      Integer - spell id (forensics tag, ability gating)
 * @param amount       Integer - base damage before resist (post-resist if DMG_FORCED)
 * @param damageType   Legacy DMG_* (FIRE/COLD/ENERGY/POISON/PHYSICAL/SLASH/...);
 *                     DMG_FORCED falls back to energy channel (matches the
 *                     pre-migration default in _SC_StampSpellDamage).
 * @param applyItemBonus 1 to add equipped focus elemental bonus (default);
 *                     DoT tick scripts pass 0 to avoid re-adding it per tick.
 * @return Integer - damage actually dealt after resists
 */
function SC_DealDamage(caster, targ, spellId, amount, damageType := DMG_FORCED, applyItemBonus := 1)
	amount := CInt(amount);
	if (!caster || !targ || amount <= 0)
		return 0;
	endif

	if (applyItemBonus)
		amount := amount + GetMagicItemElementalBonus(caster, damageType);
	endif

	return DealDamage(targ, amount, _SC_MapLegacyDamageType(damageType), caster, spellId);
endfunction

// Maps the legacy DMG_* enum used by spell scripts to the new DAMAGE_TYPE
// enum the combat pipeline operates on. DMG_FORCED (no thematic type) and
// DMG_PHYSICAL (no slash/pierce/blunt bias) collapse to sensible defaults
// matching the pre-migration channel choice in _SC_StampSpellDamage.
function _SC_MapLegacyDamageType(legacy)
	case (CStr(legacy))
		DMG_FIRE:     return DAMAGE_TYPE::FIRE;
		DMG_COLD:     return DAMAGE_TYPE::COLD;
		DMG_POISON:   return DAMAGE_TYPE::POISON;
		DMG_ENERGY:   return DAMAGE_TYPE::ENERGY;
		DMG_SLASH:    return DAMAGE_TYPE::SLASH;
		DMG_PIERCING: return DAMAGE_TYPE::PIERCING;
		DMG_BLUNT:    return DAMAGE_TYPE::BLUNT;
		DMG_PHYSICAL: return DAMAGE_TYPE::BLUNT;
	endcase
	return DAMAGE_TYPE::ENERGY;
endfunction
