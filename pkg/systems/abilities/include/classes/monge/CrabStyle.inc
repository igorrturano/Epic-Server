use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":attributes:attributes";

/**
 * Kani no Sutairu / Estilo do Caranguejo (Crab Style) - ABILITY_ID::CRAB_STYLE (702)
 *
 * Active (SingleUse AFTER_DAMAGE): Next attack causes bleed for Tactics/10 seconds.
 *
 * Passive (DefenderBeforeDamage): Reduces incoming physical damage by Tactics/20.
 *
 * Climate resistance passive stays in coldeffect.src/caloreffect.src (outside combat).
 */
class CrabStyle(CombatAbility)

	function CrabStyle(this)
		Super(ABILITY_ID::CRAB_STYLE);
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

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		Message("*estilo do caranguejo*").SendAbove(who);
	endfunction

	// Active: Bleed on next hit
	function OnAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var defender := attackInfo.GetDefender();

		var bleedDuration := CInt(attacker.GetSkill(TACTICS) / 10);
		if (bleedDuration < 3)
			bleedDuration := 3;
		endif

		TS_StartTimer(defender.GetObjRef(), "bleed", bleedDuration);

		Message("*faz um corte profundo!*").SendAbove(attacker.GetObjRef());

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[CRAB_STYLE] Sangramento: {bleedDuration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	// Passive: Physical damage reduction (defender side)
	function OnPassiveBeforeDamage(this, attackInfo)
		var defender := attackInfo.GetDefender();

		var reduction := CInt(defender.GetSkill(TACTICS) / 20);
		if (reduction <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.DecreaseSlashBaseDamage(reduction)
			.DecreasePiercingBaseDamage(reduction)
			.DecreaseBluntBaseDamage(reduction);

		DebugMsg(
			defender.GetObjRef(),
			DEBUG_HAB,
			Message($"[CRAB_STYLE] Protecao fisica: -{reduction}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
