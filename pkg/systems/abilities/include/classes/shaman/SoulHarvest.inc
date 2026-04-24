include ":abilities:models/CombatAbility";

/**
 * Colheita de Almas (Soul Harvest) - ABILITY_ID::SOUL_HARVEST (2600)
 *
 * Purely passive ability - no combat hooks.
 * Soul collection logic is handled externally in death.inc:ShamanSoulHarvest()
 * which runs on every NPC death (candie.src hook).
 * Increments "shaman_souls" property on nearby shamans who have this ability.
 * Animals and undead have no soul.
 */
class SoulHarvest(CombatAbility)

	function SoulHarvest(this)
		Super(ABILITY_ID::SOUL_HARVEST);
	endfunction

endclass
