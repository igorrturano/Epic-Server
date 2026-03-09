use uo;

include ":abilities:models/CombatAbility";

// Archaeologist - ABILITY_ID::ARCHAEOLOGIST (3608)
// Passive: Can decipher ancient writings.
// Effect handled in sysbook/book/use.src, mapmakersPen.src, lore_utils.inc, merchant.inc.
class Archaeologist(CombatAbility)
	function Archaeologist(this)
		Super(ABILITY_ID::ARCHAEOLOGIST);
	endfunction
endclass

// Expert Herbalist - ABILITY_ID::EXPERT_HERBALIST (3609)
// Passive: 30% chance to get double reagent.
// Effect handled in reagent gathering scripts.
class ExpertHerbalist(CombatAbility)
	function ExpertHerbalist(this)
		Super(ABILITY_ID::EXPERT_HERBALIST);
	endfunction
endclass

// Recovery - ABILITY_ID::RECOVERY (3612)
// Passive: Halve knockout time.
// Effect handled in death/deathscripts/players.src.
class Recovery(CombatAbility)
	function Recovery(this)
		Super(ABILITY_ID::RECOVERY);
	endfunction
endclass

// Architect - ABILITY_ID::ARCHITECT (3617)
// Passive: Build/plan architecture, bypass stone/wood skill checks.
// Effect handled in architect.src and gathering scripts.
class Architect(CombatAbility)
	function Architect(this)
		Super(ABILITY_ID::ARCHITECT);
	endfunction
endclass

// Tool Preservation - ABILITY_ID::TOOL_PRESERVATION (3618)
// Passive: 50% chance to prevent tool wear.
// Effect handled in toolWear.inc and toolWearMethod.inc.
class ToolPreservation(CombatAbility)
	function ToolPreservation(this)
		Super(ABILITY_ID::TOOL_PRESERVATION);
	endfunction
endclass

// Write Scrolls - ABILITY_ID::WRITE_SCROLLS (3624)
// Passive: Can write spell scrolls.
// Effect handled in spell/scroll creation scripts.
class WriteScrolls(CombatAbility)
	function WriteScrolls(this)
		Super(ABILITY_ID::WRITE_SCROLLS);
	endfunction
endclass
