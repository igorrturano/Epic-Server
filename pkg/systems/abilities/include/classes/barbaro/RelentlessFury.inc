use uo;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Furia Implacavel (Relentless Fury) - ABILITY_ID::FURIA_IMPLACAVEL (603)
 *
 * BeforeHit (defender-centric): Requires fury active.
 * OnActivate: +30 HP bonus via hitsmod, heals 30 HP.
 * OnBeforeHit: Always counter-attacks when hit.
 * OnDeactivate: Removes HP bonus, caps HP if over max.
 */
class RelentlessFury(CombatAbility)

	function RelentlessFury(this)
		Super(ABILITY_ID::FURIA_IMPLACAVEL);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS) / 2);
		if (duration < 5)
			duration := 5;
		endif
		return duration;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (!GetCooldown(who, "furia"))
			Message("Voce precisa estar em furia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();

		var hitsmod := CInt(mobile.GetProperty("hitsmod"));
		hitsmod := hitsmod + 30;
		mobile.SetProperty("hitsmod", hitsmod);
		RecalcVitals(who);
		HealHits(who, 30);

		Message("Voce esta cego de violencia.", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::FURIA_IMPLACAVEL, duration, "+30 HP, contra-ataque");

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[FURIA_IMPLACAVEL] +30 HP, contra-ataque ativado", Color(SYSTEM_COLOR::SUCCESS))
		);
	endfunction

	function OnBeforeHit(this, attackInfo)
		attackInfo.SetCounterAttack(true);

		DebugMsg(
			attackInfo.GetDefender().GetObjRef(),
			DEBUG_HAB,
			Message("[FURIA_IMPLACAVEL] Contra-ataque!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function OnDeactivate(this, mobile)
		var who := mobile.GetObjRef();

		var hitsmod := CInt(mobile.GetProperty("hitsmod"));
		hitsmod := hitsmod - 30;
		mobile.SetProperty("hitsmod", hitsmod);
		RecalcVitals(who);

		if (GetVital(who, HITS) > GetVitalMaximumValue(who, HITS))
			SetVital(who, HITS, GetVitalMaximumValue(who, HITS));
		endif

		Message("Sua furia implacavel cessou.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
	endfunction

endclass
