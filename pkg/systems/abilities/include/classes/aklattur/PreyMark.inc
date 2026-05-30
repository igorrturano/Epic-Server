use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";

/**
 * Marca da Presa (Prey Mark) - ABILITY_ID::MARCA_DA_PRESA (502)
 *
 * BeforeDamage: All attacks ignore armor for Nature/5 seconds (min 3).
 * Supports pre-set duration override (used by Carnage chain-activation).
 */
class PreyMark(CombatAbility)

	function PreyMark(this)
		Super(ABILITY_ID::MARCA_DA_PRESA);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function GetDurationFor(this, mobile)
		if (this.GetDuration() > 0)
			return this.GetDuration();
		endif

		var duration := CInt(mobile.GetSkill(NATURE) / 5);
		if (duration < 3)
			duration := 3;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::MARCA_DA_PRESA, duration, "Ataques ignoram armadura");
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		attackInfo.SetIgnoreArmor();

		Message("*marca a presa!*").SendAbove(attacker.GetObjRef());

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message("[MARCA_DA_PRESA] Ativo - ignorando armadura!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
