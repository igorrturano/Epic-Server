use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Faro por Sangue (Blood Scent) - ABILITY_ID::FARO_POR_SANGUE (1204)
 *
 * OnUse: Activates blood scent for 60 seconds.
 * Passive (BeforeDamage): While active, deals bonus damage based on target HP:
 *   - HP <= 25%: +1d12
 *   - HP <= 50%: +1d10
 *   - HP <= 75%: +1d8
 *
 * Migrated from DamageController_legacy.inc:437-451.
 */
class BloodScent(CombatAbility)

	function BloodScent(this)
		Super(ABILITY_ID::FARO_POR_SANGUE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		SetCooldown(who, "faroporsangue", 60);
		Message("*farejando sangue*").SendAbove(who);
		Message("Você explora a fraqueza de seus inimigos.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);
		return true;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		if (!GetCooldown(attacker.GetObjRef(), "faroporsangue"))
			return false;
		endif

		var defender := attackInfo.GetDefender();
		var hpRatio := CDbl(AP_GetVital(defender.GetObjRef(), HITS)) / CDbl(AP_GetVitalMaximumValue(defender.GetObjRef(), HITS));

		var bonus;
		if (hpRatio <= 0.25)
			bonus := "1d12";
		elseif (hpRatio <= 0.5)
			bonus := "1d10";
		elseif (hpRatio <= 0.75)
			bonus := "1d8";
		else
			return false;
		endif

		var damagemod := RandomDiceRoll(bonus);

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(damagemod)
			.IncreasePiercingRawDamage(damagemod)
			.IncreaseBluntRawDamage(damagemod);

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[FARO_POR_SANGUE] +{damagemod} dano ({bonus})", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
