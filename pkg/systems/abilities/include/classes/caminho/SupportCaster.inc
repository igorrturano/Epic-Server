use uo;

include ":abilities:models/CombatAbility";

/**
 * Support Caster - ABILITY_ID::SUPPORT_CASTER (3401)
 *
 * Passive: Maintain spells at half mana cost + refund 50% mana on spell fail.
 * Merges old "Manter Magias" + "Mago Precavido" into one ability.
 * Effects handled in mantainSpell.src and cast_spell.src.
 * This class is a stub for registry purposes.
 */
class SupportCaster(CombatAbility)

	function SupportCaster(this)
		Super(ABILITY_ID::SUPPORT_CASTER);
	endfunction

endclass
