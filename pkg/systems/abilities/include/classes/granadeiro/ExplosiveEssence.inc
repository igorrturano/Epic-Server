include ":abilities:models/CombatAbility";

/**
 * Explosive Essence (Essência Explosiva) - ABILITY_ID::ESSENCIA_EXPLOSIVA (2503)
 *
 * Passive ability (no active ExecuteOn):
 * - Grants resistance to explosions (take half damage or resist completely) [exploder.src]
 * - Grants resistance to walk-on potions [walkonpot.src]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::ESSENCIA_EXPLOSIVA)
 */
class ExplosiveEssence(CombatAbility)

	function ExplosiveEssence(this)
		Super(ABILITY_ID::ESSENCIA_EXPLOSIVA);
	endfunction

endclass
