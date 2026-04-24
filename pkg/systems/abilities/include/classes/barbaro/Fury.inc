use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":tn:cooldown";
include ":attributes:attributes";
include ":hotbar:custombuff";


/**
 * Furia (Fury) - ABILITY_ID::FURIA (600)
 *
 * BeforeDamage: Enters a fury state for Tactics seconds.
 * Adds Tactics/20 bonus damage to all physical postures.
 * Sets "furia" cooldown used by other barbarian abilities.
 * If has Destruidor (604), damage bonus is doubled.
 */
class Fury(CombatAbility)

	function Fury(this)
		Super(ABILITY_ID::FURIA);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var itemdesc := ReadConfigFile(":*:itemdesc");
		foreach item in (ListEquippedItems(who))
			if (item.isa(POLCLASS_ARMOR))
				var tipoProtecao := CStr(itemdesc[item.objtype].TipoProtecao);
				if (tipoProtecao["Armadura"])
					Message("Voce nao pode usar Furia com armadura.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
			endif
		endforeach
		return true;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS));
		if (duration < 5)
			duration := 5;
		endif
		return duration;
	endfunction

	function OnActivate(this, mobile)
		var who := mobile.GetObjRef();
		var duration := this.GetDurationFor(mobile);

		SetCooldown(who, "furia", duration);

		var male := array{ 400, 1828, 1830, 1832, 1834 };
		if (who.graphic in male)
			PlaySoundEffect(who, 0x44B);
		else
			PlaySoundEffect(who, 0x339);
		endif
		SendAbilityBuff(who, ABILITY_ID::FURIA, duration, "Bonus de dano em posturas fisicas");
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		var damageBonus := CInt(attacker.GetSkill(TACTICS) / 20);

		if (attacker.HasAbility(ABILITY_ID::DESTRUIDOR) && GetCooldown(attacker.GetObjRef(), "furia"))
			damageBonus := damageBonus * 2;
		endif

		if (damageBonus > 0)
			attackInfo.GetDamage()
				.IncreaseSlashRawDamage(damageBonus)
				.IncreasePiercingRawDamage(damageBonus)
				.IncreaseBluntRawDamage(damageBonus);

			DebugMsg(
				attacker.GetObjRef(),
				DEBUG_HAB,
				Message($"[FURIA] Bonus de dano: +{damageBonus}", Color(SYSTEM_COLOR::SUCCESS))
			);
		endif

		return true;
	endfunction

	function OnDeactivate(this, mobile)
		var who := mobile.GetObjRef();

		if (GetCooldown(who, "furia"))
			SetCooldown(who, "furia", 0);
		endif
	endfunction

endclass
