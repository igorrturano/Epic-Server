use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Golpe do Vento / Wind Strike - ABILITY_ID::WIND_STRIKE (803)
 *
 * SingleUse AFTER_DAMAGE: On next successful hit, performs a second attack.
 */
class WindStrike(CombatAbility)

	function WindStrike(this)
		Super(ABILITY_ID::WIND_STRIKE);
		this.ActivationMoment := ABILITY_ACTIVATION::AFTER_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS) / 10);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (who.shield)
			Message("Um verdadeiro samurai nao usaria escudo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		Message("*prepara um golpe duplo*").SendAbove(who);
		Message("Voce prepara um golpe duplo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::WIND_STRIKE, duration, "Golpe duplo");
	endfunction

	function OnAfterDamage(this, attackInfo)
		var who := attackInfo.GetAttacker().GetObjRef();

		who.attack_once();

		Message("*desfere um segundo golpe!*").SendAbove(who);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[WIND_STRIKE] Golpe duplo executado!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
