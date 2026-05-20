use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";
include "include/epicCombatUtils";
/**
 * Furia do Deserto (Desert Fury) - ABILITY_ID::FURIA_DO_DESERTO (500)
 *
 * Active (AfterDamage): Cleave mode - each attack hits all nearby enemies.
 * Duration: Survival/2 seconds (min 5).
 * Uses recursion guard to prevent cleave triggering cleave.
 *
 * Passive (BeforeDamage): When raging (low HP), adds Survival/20 bonus
 * to all damage types.
 *
 * Supports pre-set duration override (used by Carnage chain-activation).
 */
class DesertFury(CombatAbility)

	function DesertFury(this)
		Super(ABILITY_ID::FURIA_DO_DESERTO);
		this.ActivationMoment := ABILITY_ACTIVATION::AFTER_DAMAGE;
	endfunction

	function GetDurationFor(this, mobile)
		if (this.GetDuration() > 0)
			return this.GetDuration();
		endif

		var duration := CInt(mobile.GetSkill(SURVIVAL) / 2);
		if (duration < 5)
			duration := 5;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		var duration := this.GetDurationFor(mobile);
		SendAbilityBuff(who, ABILITY_ID::FURIA_DO_DESERTO, duration, "Cleave em todos os inimigos proximos");
	endfunction

	// ACTIVE: Cleave - attack all nearby hostiles after each hit
	function OnAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();
		var defender := attackInfo.GetDefender();

		// Prevent recursion (cleave triggering cleave)
		if (GetObjProperty(who, "#furiadodeserto_cleaving"))
			return false;
		endif

		SetObjProperty(who, "#furiadodeserto_cleaving", 1);

		var hostileList := listAllMobilesNearbyButPetsAndParty(who, 2);

		foreach mobile in (hostileList)
			if (mobile.serial != who.serial &&
				mobile.serial != defender.GetSerial() &&
				GetVital(mobile, HITS) > 0 &&
				!mobile.hidden &&
				!mobile.concealed)
				attacker.Attack(MobileWrapper(mobile));
			endif
		endforeach

		EraseObjProperty(who, "#furiadodeserto_cleaving");

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[FURIA_DO_DESERTO] Cleave ativado!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	// PASSIVE: Rage bonus when at low HP
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		if (!attacker.IsRaging())
			return false;
		endif

		var survival := attacker.GetSkill(SURVIVAL);
		var bonus := CInt(survival / 20);

		if (bonus <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(bonus)
			.IncreasePiercingRawDamage(bonus)
			.IncreaseBluntRawDamage(bonus);

		Message("*enfurecido*").SendAbove(attacker.GetObjRef());

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[FURIA_DO_DESERTO] Bonus de furia: +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
