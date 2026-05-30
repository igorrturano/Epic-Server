use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":hotbar:custombuff";

class TargetingVisor(CombatAbility)

	function TargetingVisor(this)
		Super(ABILITY_ID::TARGETING_VISOR);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(SCIENCE) / 5);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		Message("*Visor de mira ativado*").SendAbove(who);
		PlaySoundEffect(who, 0x5ab);

		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::TARGETING_VISOR, duration, "Ignora armadura");
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		// Equipment check
		var visor := GetEquipmentByLayer(who, 0x04); // LAYER_HAT
		if (!visor || visor.objtype != 0xFFC4) // Steam Helmet
			return false;
		endif

		attackInfo.SetIgnoreArmor();

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[TARGETING_VISOR] Ativo - ignorando armadura!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
