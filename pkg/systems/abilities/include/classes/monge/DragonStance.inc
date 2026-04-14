use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":hotbar:custombuff";

/**
 * Ryū no Kamae / Postura do Dragão (Dragon Stance) - ABILITY_ID::DRAGON_STANCE (701)
 *
 * SingleUse BEFORE_DAMAGE: Next attack deals bonus Tactics/10 damage.
 * After the hit, grants projectile immunity via "Desviar Disparos" cooldown
 * for Tactics/5 seconds.
 */
class DragonStance(CombatAbility)

	function DragonStance(this)
		Super(ABILITY_ID::DRAGON_STANCE);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
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
		var duration := this.GetDurationFor(mobile);
		Message("*postura do dragao*").SendAbove(who);
		PlaySoundEffect(who, 0x50E);
		SendAbilityBuff(who, ABILITY_ID::DRAGON_STANCE, duration, "Bonus de dano + imunidade a projeteis");
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();
		var defender := attackInfo.GetDefender().GetObjRef();

		// Bonus physical + fire damage
		var damageBonus := CInt(attacker.GetSkill(TACTICS) / 10);
		if (damageBonus > 0)
			attackInfo.GetDamage()
				.IncreaseSlashRawDamage(damageBonus)
				.IncreasePiercingRawDamage(damageBonus)
				.IncreaseBluntRawDamage(damageBonus)
				.IncreaseFireRawDamage(damageBonus);
		endif

		PlaySoundEffect(defender, 0x208); // SFX_EXPLOSION_11
		PlayObjectCenteredEffect(defender, FX_FIRE_DRAGON_PUNCH, 0, 15);

		// Projectile immunity after the attack
		var immunityDuration := CInt(attacker.GetSkill(TACTICS) / 5);
		if (immunityDuration < 5)
			immunityDuration := 5;
		endif
		SetCooldown(who, "Desviar Disparos", immunityDuration);

		Message($"*Postura do Dragao!*").SendAbove(who);
		Message($"*Inalvejavel por projeteis por {immunityDuration}s*").SendSys(who);
		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[DRAGON_STANCE] Dano bonus: +{damageBonus}, Desviar Disparos: {immunityDuration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
