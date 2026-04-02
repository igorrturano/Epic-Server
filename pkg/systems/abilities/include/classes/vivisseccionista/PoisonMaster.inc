include ":abilities:models/CombatAbility";

/**
 * Mestre dos Venenos (Poison Master) - ABILITY_ID::MESTRE_DOS_VENENOS (1400)
 *
 * Passive: Allows applying all poisons to food and weapons without damaging them.
 *
 * Logic lives in:
 *   - commands/player/envenenar.src — poison application checks
 */
class PoisonMaster(CombatAbility)

	function PoisonMaster(this)
		Super(ABILITY_ID::MESTRE_DOS_VENENOS);
	endfunction

endclass
