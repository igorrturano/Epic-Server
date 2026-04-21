use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include ":abilities:helpers/shinobiHelpers";
include ":wrappers:Objects/Model/WeaponWrapper";
include "include/client";

/**
 * Caminho Ninja (Ninja Path) - ABILITY_ID::NINJA_PATH (1300)
 *
 * Passive: Better stealth, listen through walls, throw daggers without stamina cost.
 *          Grants access to shinobi techniques (spells).
 * Active (OnUse): Two-state kunai mark system:
 *   - No mark: Select target -> throw dagger visual -> mark target (expires in Tactics/2 seconds)
 *   - Has mark: Teleport behind marked target and attack
 */
class NinjaPath(CombatAbility)

	function NinjaPath(this)
		Super(ABILITY_ID::NINJA_PATH);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var mark_serial := GetObjProperty(who, "#ninja_mark");
		if (mark_serial)
			if (who.frozen || who.paralyzed)
				Message("Voce nao consegue se mover.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif
		else
			if (!_NP_HasDagger(who))
				Message("Voce precisa ter uma adaga para marcar o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var sneak := CInt(mobile.GetSkill(SNEAK));

		var mark_serial := GetObjProperty(who, "#ninja_mark");
		var mark_expires := GetObjProperty(who, "#ninja_mark_expires");

		if (mark_serial)
			if (ReadGameClock() > mark_expires)
				EraseObjProperty(who, "#ninja_mark");
				EraseObjProperty(who, "#ninja_mark_expires");
				Message("A marca expirou.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				return false;
			endif
			return this._TeleportToMark(who, mark_serial, sneak);
		else
			return this._MarkTarget(mobile, who, sneak);
		endif
	endfunction

	function _MarkTarget(this, mobile, who, sneak)
		var item_graphic := 0x1BFE;

		var targ;
		if (mobile.IsNPC())
			targ := who.opponent;
			if (!targ)
				targ := GetRandomNearbyHostile(who);
			endif
		else
			Message("Selecione o alvo para marcar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			targ := Target(who);
		endif

		if (!targ)
			return false;
		elseif (!targ.isA(POLCLASS_MOBILE))
			Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (targ.serial == who.serial)
			Message("Voce nao pode marcar a si mesmo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Visual: throw kunai effect
		PlaySoundEffect(targ, 0x531);
		sleep(1);
		PlayMovingEffect(who, targ, 0xF99A, 10);

		// Set mark with expiration
		var mark_duration := CInt(sneak / 2);
		if (mark_duration < 3)
			mark_duration := 3;
		endif

		SetObjProperty(who, "#ninja_mark", targ.serial);
		SetObjProperty(who, "#ninja_mark_expires", ReadGameClock() + mark_duration);

		Message($"*Marca {targ.name} com uma adaga*").SendEmote(who);
		Message($"Alvo marcado por {mark_duration} segundos. Use novamente para teleportar.", Color(SYSTEM_COLOR::SUCCESS)).SendSys(who);

		// Reset cooldown so player can use the ability again to teleport
		EraseObjProperty(who, $"AbilityCooldown_{ABILITY_ID::NINJA_PATH}");

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[NINJA_PATH] Marked {targ.name} for {mark_duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	// Substituicao passive: when hit while #substituicao_active, stun attacker + hide + leave a log
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var defRef := defender.GetObjRef();

		if (!defender.HasAbility(ABILITY_ID::NINJA_PATH))
			return false;
		endif

		if (!GetObjProperty(defRef, "#substituicao_active"))
			return false;
		endif

		// Consume the substituicao
		EraseObjProperty(defRef, "#substituicao_active");

		var attacker := attackInfo.GetAttacker().GetObjRef();
		var sneak := CInt(defender.GetSkill(SNEAK));

		// Stun the attacker
		var stun_duration := CInt(sneak / 10);
		if (stun_duration < 2)
			stun_duration := 2;
		endif

		// Create a log at the ninja's position
		var log_item := CreateItemAtLocation(defRef.x, defRef.y, defRef.z, 0x1BDD, 1, defRef.realm);
		if (log_item)
			log_item.name := "Tora";
			log_item.decayat := ReadGameClock() + 15;
		endif

		// Hide the ninja
		defRef.hidden := 1;

		// VFX
		PlaySoundEffect(defRef, 0x228);
		PlayObjectCenteredEffect(defRef, 0x3728, 10, 10);

		Message("*Substituicao!*").SendEmote(defRef);
		Message($"Voce substituiu seu corpo por uma tora!", Color(SYSTEM_COLOR::SUCCESS)).SendSys(defRef);

		// Force miss by setting a very negative hit chance modifier
		attackInfo.SetHitChanceMod(-1000);

		DebugMsg(defRef, DEBUG_HAB,
			Message($"[NINJA_PATH] Substituicao triggered!", Color(SYSTEM_COLOR::SUCCESS)));

		return true;
	endfunction

	function _TeleportToMark(this, who, mark_serial, sneak)
		var targ := SystemFindObjectBySerial(mark_serial);

		if (!targ || !targ.isA(POLCLASS_MOBILE))
			EraseObjProperty(who, "#ninja_mark");
			EraseObjProperty(who, "#ninja_mark_expires");
			Message("O alvo marcado nao foi encontrado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var range := CInt(sneak / 5);
		if (range < 5)
			range := 5;
		endif

		if (Distance(who, targ) > range || abs(who.z - targ.z) > 19)
			Message("Alvo muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (!CheckLineOfSight(who, targ))
			Message("Alvo fora da linha de visao.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Teleport behind target
		PlaySoundEffect(who, 0x511);
		TeleportBehindTarget(who, targ);

		// Guaranteed attack
		SetObjProperty(who, "#GuaranteedHit", 1);
		who.attack_once(targ);

		Message("*Aparece atras do alvo!*").SendEmote(who);

		// Clear mark
		EraseObjProperty(who, "#ninja_mark");
		EraseObjProperty(who, "#ninja_mark_expires");

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[NINJA_PATH] Teleported to {targ.name}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _NP_HasDagger(who)
	var weapon := who.weapon;
	if (weapon)
		var ww := WeaponWrapper(weapon);
		if (!ww.GetObjRef().errortext && ww.GetCategory() == WEAPON_CATEGORY::DAGGER)
			return 1;
		endif
	endif
	foreach item in (EnumerateItemsInContainer(who.backpack))
		var ww := WeaponWrapper(item);
		if (!ww.GetObjRef().errortext && ww.GetCategory() == WEAPON_CATEGORY::DAGGER)
			return 1;
		endif
	endforeach
	return 0;
endfunction
