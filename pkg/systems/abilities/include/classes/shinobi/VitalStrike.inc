use uo;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":timedscripts:timedScripts";

/**
 * Golpe nos Pontos Vitais (Vital Strike) - ABILITY_ID::VITAL_STRIKE (1301)
 *
 * Active (BeforeDamage - as ATTACKER, SingleUse):
 * - Paralyzes the target for Tactics/10 seconds
 * - Drains Tactics/3 stamina from the target
 * - SingleUse: auto-deactivates after first hit
 */
class VitalStrike(CombatAbility)

	function VitalStrike(this)
		Super(ABILITY_ID::VITAL_STRIKE);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
		this.SingleUse := true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(SNEAK) / 10);
		if (duration < 2)
			duration := 2;
		endif
		return duration;
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var defender := attackInfo.GetDefender();
		var who := attacker.GetObjRef();
		var targ := defender.GetObjRef();
		var sneak := CInt(attacker.GetSkill(SNEAK));

		var paralyze_duration := CInt(sneak / 10);
		if (paralyze_duration < 1)
			paralyze_duration := 1;
		endif

		var stamina_drain := CInt(sneak / 3);

		// Visual effects
		Message("*Atinge os pontos vitais*").SendEmote(who);
		PlayObjectCenteredEffect(targ, 0x376A, 1, 16);
		PlaySoundEffect(targ, 0x511);

		// Apply paralysis
		TS_StartTimer(targ, "paralysis", paralyze_duration);
		Message("*paralisado*").SendAbove(targ);

		// Drain stamina
		AP_ConsumeVital(targ, "Stamina", stamina_drain);
		
		// Add divine damage to all damage types
        attackInfo.GetDamage()
            .IncreaseBluntRawDamage(stamina_drain)
            .IncreasePiercingRawDamage(stamina_drain)
            .IncreaseSlashRawDamage(stamina_drain);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[VITAL_STRIKE] Paralyze: {paralyze_duration}s, Stamina drain: {stamina_drain}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
