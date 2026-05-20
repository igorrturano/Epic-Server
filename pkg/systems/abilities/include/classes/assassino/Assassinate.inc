use os;
use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":combat:Controller/HitController";
include "include/utils";
include "include/client";

/**
 * Assassinar (Assassinate) - ABILITY_ID::ASSASSINAR (1200)
 *
 * OnUse: While hidden, leaps behind a target (range = SNEAK/5).
 * Guarantees hit if hidden, critical if has Instinto Assassino.
 * PassiveBeforeDamage: Adds SNEAK/20 bonus damage.
 */
class Assassinate(CombatAbility)

	function Assassinate(this)
		Super(ABILITY_ID::ASSASSINAR);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
/* 		if (!who.hidden)
			Message("Você precisa estar furtivo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		else */if (who.frozen || who.paralyzed)
			Message("Você não consegue se mover.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!CanBackStab(mobile) || !weapon || !weapon.IsAssassin())
			Message("Esta arma não é apropriada para um ataque furtivo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var range := CInt(EffectiveSkill(who, "Stealth") / 2);

		var targ;
		if (mobile.IsNPC())
			targ := who.opponent;
			if (!targ)
				targ := GetRandomNearbyHostile(who);
			endif
		else
			targ := Target(who);
		endif

		if (!targ)
			return false;
		elseif (!targ.isA(POLCLASS_MOBILE))
			Message("Alvo inválido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (Distance(who, targ) > range || abs(who.z - targ.z) > 19)
			Message("Alvo muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (!CheckLineOfSight(who, targ))
			Message("Alvo fora da linha de visão.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		who.setwarmode(1);
		mobile.TeleportBehind(MobileWrapper(targ));

		if (mobile.IsHidden())
			mobile.SetProperty("#GuaranteedHit", 1);
			if (mobile.HasAbility(ABILITY_ID::INSTINTO_ASSASSINO))
				mobile.SetProperty("#nextcritical", 1);
			endif
		endif

		GrantPrivilege(who, "hiddenattack");
		who.enable("hiddenattack");

		who.attack_once(targ);

		return true;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!who.hidden)
			return false;
		endif

		var sneak := CInt(EffectiveSkill(who, "Stealth"));
		var bonus := CInt(sneak / 2);

		if(attacker.HasAbility(ABILITY_ID::INSTINTO_ASSASSINO))
			bonus := CInt(sneak);
		endif

		if (bonus > 0)
			attackInfo.GetDamage()
				.IncreaseSlashRawDamage(bonus)
				.IncreasePiercingRawDamage(bonus)
				.IncreaseBluntRawDamage(bonus);

			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[ASSASSINAR] Bônus de dano furtivo (SNEAK/2): +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
			);
		endif

		PlaySoundEffect(who, 0x51D);

		return true;
	endfunction

	function OnDeactivate(this, mobile)
		var who := mobile.GetObjRef();
		RevokePrivilege(who, "hiddenattack");
		who.hidden := 0;
	endfunction

endclass
