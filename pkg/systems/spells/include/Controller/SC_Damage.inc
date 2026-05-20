use uo;

include ":wrappers:Objects/Model/MobileWrapper";
include ":combat:Model/Damage";
include ":combat:Controller/DamageController";
include "MagicItemBonus";

/**
 * SC_DealDamage — canonical entry point for spell damage.
 *
 * Thin facade over DealDamage. Adds the equipped-focus elemental bonus,
 * then delegates. Resist, application, ability hooks (AFTER_DAMAGE / ON_KILL),
 * forensics, lastHit tracking and #vinganca are all handled by DealDamage.
 *
 * @param caster         Mobile - the attacker
 * @param targ           Mobile - the victim
 * @param spellId        Integer - spell id (forensics tag, ability gating)
 * @param amount         Integer - base damage before resist
 * @param damageType     DAMAGE_TYPE::* enum value. Defaults to FORCED.
 * @param applyItemBonus 1 to add equipped focus elemental bonus (default);
 *                       DoT tick scripts pass 0 to avoid re-adding per tick.
 * @return Integer - damage actually dealt after resists
 */
function SC_DealDamage(caster, targ, spellId, amount, damageType := DAMAGE_TYPE::FORCED, applyItemBonus := 1)
	amount := CInt(amount);
	if (!caster || !targ || amount <= 0)
		return 0;
	endif

	if (applyItemBonus)
		amount := amount + GetMagicItemElementalBonus(caster, damageType);
	endif

	return DealDamage(targ, amount, damageType, caster, spellId);
endfunction
