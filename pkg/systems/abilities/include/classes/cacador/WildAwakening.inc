include ":abilities:models/CombatAbility";

/**
 * Despertar Selvagem (Wild Awakening) - ABILITY_ID::MESTRE_DE_FERAS (102)
 *
 * Purely passive ability - no combat hooks. Effects handled inline:
 *   - taming.src: enables the "Despertar" (Awaken) pet menu, gates the
 *     teach-pet feature and which instinct types are trainable, and
 *     applies the Beastmastery-scaled pet HP/damage bonus at awaken.
 *   - caloreffect.src / coldeffect.src: group weather resistance.
 *   - survival.src: tracking difficulty/range bonus.
 */
class WildAwakening(CombatAbility)

	function WildAwakening(this)
		Super(ABILITY_ID::MESTRE_DE_FERAS);
	endfunction

endclass
