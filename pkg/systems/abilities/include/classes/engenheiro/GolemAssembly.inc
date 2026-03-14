use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";

class GolemAssembly(CombatAbility)

	function GolemAssembly(this)
		Super(ABILITY_ID::GOLEM_ASSEMBLY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		return Run_Script_To_Completion(":abilities:classes/engenheiro/golemassembly", mobile);
	endfunction

endclass
