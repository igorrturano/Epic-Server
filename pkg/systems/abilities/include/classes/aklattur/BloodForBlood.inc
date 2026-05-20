use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";

/**
 * Sangue por Sangue (Blood for Blood) - ABILITY_ID::SANGUE_POR_SANGUE (501)
 *
 * AfterDamage: 50% of damage dealt returns as healing for Survival/2 seconds (min 5).
 * Supports pre-set duration override (used by Carnage chain-activation).
 */
class BloodForBlood(CombatAbility)

	function BloodForBlood(this)
		Super(ABILITY_ID::SANGUE_POR_SANGUE);
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
		SendAbilityBuff(who, ABILITY_ID::SANGUE_POR_SANGUE, duration, "Roubo de vida em cada golpe");
	endfunction

	function OnAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		var damage := attackInfo.GetDamage();
		var posture := attacker.GetCombatTraits().GetPosture();
		var physicalDamage := 0;

		if (posture == DAMAGE_TYPE::SLASH)
			physicalDamage := damage.GetSlashBaseDamage();
		elseif (posture == DAMAGE_TYPE::PIERCING)
			physicalDamage := damage.GetPiercingBaseDamage();
		elseif (posture == DAMAGE_TYPE::BLUNT)
			physicalDamage := damage.GetBluntBaseDamage();
		endif

		var totalDamage := physicalDamage
			+ damage.GetFireBaseDamage()
			+ damage.GetColdBaseDamage()
			+ damage.GetPoisonBaseDamage()
			+ damage.GetEnergyBaseDamage();

		var healAmount := CInt(totalDamage * 0.2); // 20% of damage dealt

		if (healAmount <= 0)
			return false;
		endif

		HealHits(who, healAmount);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[SANGUE_POR_SANGUE] Roubo de vida: +{healAmount} HP", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
