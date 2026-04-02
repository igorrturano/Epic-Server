use uo;

include ":abilities:models/CombatAbility";

/**
 * Foco Arcano - ABILITY_ID::FOCO_ARCANO (3402)
 *
 * Passive: Reduz em 20% o consumo de mana em conjuração/disparo arcano
 * quando equipado com Staff, SpiritualBlade ou MagicBow.
 * Effect handled in cast_spell.src and combatHook.src.
 */
class FocoArcano(CombatAbility)

	function FocoArcano(this)
		Super(ABILITY_ID::FOCO_ARCANO);
	endfunction

endclass
