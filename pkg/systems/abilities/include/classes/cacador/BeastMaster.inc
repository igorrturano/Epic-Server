include ":abilities:models/CombatAbility";

/**
 * Mestre de Feras (Beast Master) - ABILITY_ID::MESTRE_DE_FERAS (102)
 *
 * Purely passive ability - no combat hooks.
 * Effects are handled inline in taming.src:
 *   - Enables "Despertar" (Awaken) pet evolution menu
 *   - Gates the "teach pet" feature
 *   - Controls which instinct types are trainable (natural, sense, wild)
 */
class BeastMaster(CombatAbility)

	function BeastMaster(this)
		Super(ABILITY_ID::MESTRE_DE_FERAS);
	endfunction

endclass