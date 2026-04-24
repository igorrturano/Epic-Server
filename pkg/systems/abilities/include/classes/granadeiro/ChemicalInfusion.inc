use uo;
use os;

include ":abilities:models/CombatAbility";
include ":gumps:gumps";
include ":gumps:gumps_ex";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":fls_core:packets";

/**
 * Chemical Infusion (Infusão Química) - ABILITY_ID::INFUSAO_QUIMICA (2501)
 *
 * Passive gate: unlocks the "Elemento" tab in menuExplosivos2 (ExperiencedAlchemist).
 */
class ChemicalInfusion(CombatAbility)

	function ChemicalInfusion(this)
		Super(ABILITY_ID::INFUSAO_QUIMICA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		return true;
	endfunction

endclass
