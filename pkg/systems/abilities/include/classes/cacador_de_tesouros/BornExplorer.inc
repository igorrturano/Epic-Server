include ":abilities:models/CombatAbility";

/**
 * Desbravador Nato (Born Explorer) - ABILITY_ID::DESBRAVADOR_NATO (1100)
 *
 * Passive: Allows complete trap evasion on successful Reflexes check,
 * reading treasure maps, and opening magic locks.
 *
 * Logic lives in:
 *   - trapScripts (poisonTrap, explosionTrap, needleTrap, etc.) — trap evasion
 *   - treasuremaps.inc / decoder/use.src — treasure map decoding
 *   - lockPicking.src — magic lock opening
 */
class BornExplorer(CombatAbility)

	function BornExplorer(this)
		Super(ABILITY_ID::DESBRAVADOR_NATO);
	endfunction

endclass
