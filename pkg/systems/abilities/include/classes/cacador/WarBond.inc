include ":abilities:models/CombatAbility";

/**
 * Vínculo de Guerra (War Bond) - ABILITY_ID::VINCULO_DE_GUERRA (100)
 *
 * Purely passive. No combat hooks. Effects handled inline elsewhere:
 *   - +1 war-pet budget: taming GetMaxWarPets reads HasAbility(VINCULO_DE_GUERRA)
 *     (lives on branch feat/taming-beastmastery-petbrain; no-op until merge)
 *   - Unlocks crafting throwable nets (corda.src) and hunting traps
 *     (montartrap.src), both gated on HasAbility(VINCULO_DE_GUERRA)
 */
class WarBond(CombatAbility)

	function WarBond(this)
		Super(ABILITY_ID::VINCULO_DE_GUERRA);
	endfunction

endclass
