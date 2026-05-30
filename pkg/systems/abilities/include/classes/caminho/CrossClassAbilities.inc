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

// Healing Herbs - ABILITY_ID::HEALING_HERBS (3625)
// Passive: Unlocks herb-based healing (ginseng, garlic, mushrooms).
// Effect handled in medicine/healing.src.
class HealingHerbs(CombatAbility)
	function HealingHerbs(this)
		Super(ABILITY_ID::HEALING_HERBS);
	endfunction
endclass

// Academic Training - ABILITY_ID::ACADEMIC_TRAINING (3626)
// Passive: Better anatomy/diagnosis, leech procedure bonus.
// Effect handled in medicine/anatomy.src, anatomy.inc, leech.src.
class AcademicTraining(CombatAbility)
	function AcademicTraining(this)
		Super(ABILITY_ID::ACADEMIC_TRAINING);
	endfunction
endclass

// Poliglota - ABILITY_ID::POLIGLOTA (3629)
// Passive (pool): each purchase teaches one new language the player doesn't already know.
// Effect handled at purchase time in charactercreation/include/habilidades.inc::comprarHabilidade.
class Poliglota(CombatAbility)
	function Poliglota(this)
		Super(ABILITY_ID::POLIGLOTA);
	endfunction
endclass

// Prestidigitator - ABILITY_ID::PRESTIDIGITATOR (3627)
// Passive: Sleight of hand - cheat at dice/coin games, hide items during searches, rope bonus.
// Effect handled in dicegame.src, caraoucoroa.src, revistar.src, corda.src.
class Prestidigitator(CombatAbility)
	function Prestidigitator(this)
		Super(ABILITY_ID::PRESTIDIGITATOR);
	endfunction
endclass
