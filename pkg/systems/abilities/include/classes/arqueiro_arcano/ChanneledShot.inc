use uo;
use os;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":tn:cooldown";
include ":fls_core:packets";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include "include/epicConstants";

/**
 * Channeled Shot (Disparo Canalizado) - ABILITY_ID::CHANNELED_SHOT (2402)
 *
 * Active (OnUse): Requires active arcane_bow. Fires an elemental shot in a straight
 * line that pierces all targets, applying the active element's effect to each:
 * - fire_arrow  → queimar (burn) on each target hit
 * - cold_arrow  → lentidao (slow) on each target hit
 * - energy_arrow → attacker becomes concealed for arcana/20 seconds
 */
class ChanneledShot(CombatAbility)

	function ChanneledShot(this)
		Super(ABILITY_ID::CHANNELED_SHOT);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();

		if (mobile.GetEquipBySlot(WEAPON).GetCategory() != WEAPON_CATEGORY::MAGICBOW)
			Message("Voce precisa estar com seu arco elemental conjurado!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon)
			Message("Voce precisa estar com o arco elemental equipado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);
		var weaponRef := weapon.GetObjRef();

		var isFireArrow   := GetObjProperty(weaponRef, "fire_arrow");
		var isColdArrow   := GetObjProperty(weaponRef, "cold_arrow");
		var isEnergyArrow := GetObjProperty(weaponRef, "energy_arrow");

		var ammo_effect := 0xf42;
		var sound;
		if (isFireArrow)
			ammo_effect := FX_FIRE_ARROW;
			sound := 0x5cb;
		elseif (isColdArrow)
			ammo_effect := 0xe3e5;
			sound := 0x5c8;
		elseif (isEnergyArrow)
			ammo_effect := FX_ELETRIC_BOLT;
			sound := 0x666;
		else
			Message("Seu arco nao possui elemento ativo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		Message("*Disparo Canalizado*").SendAbove(who);

		SetScriptController(who);

		var targ;
		if (mobile.IsNPC())
			var opponent := who.opponent;
			if (!opponent)
				opponent := GetRandomNearbyHostile(who);
			endif
			if (!opponent)
				return false;
			endif
			targ := struct{
				x := opponent.x,
				y := opponent.y,
				z := opponent.z,
				realm := opponent.realm
			};
		else
			Message("Escolha a direcao do disparo.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			targ := TargetCoordinates(who);
		endif

		if (!targ)
			Message("Disparo cancelado.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			return false;
		endif

		PlayObjectCenteredEffect(who, 0xDF4B, 7, 16);

		if (!CheckLOSAt(who, targ.x, targ.y, targ.z))
			Message("Voce nao tem linha de visao para o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		PlaySoundEffect(who, sound);

		var targetsInLine := _GetTargetsInLine(who, targ.x, targ.y);

		foreach target in targetsInLine
			PlayMovingEffect(who, target, ammo_effect, 10);
			sleepms(80);
			who.attack_once(target);

			if (isFireArrow)
				TS_StartTimer(target, "queimar", 10);
			elseif (isColdArrow)
				TS_StartTimer(target, "lentidao", 10);
			endif
		endforeach

		if (isEnergyArrow)
			var concealDuration := CInt(mobile.GetSkill(ARCANA) / 20);
			who.concealed := 1;
			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[CHANNELED_SHOT] Concealed por {concealDuration}s", Color(SYSTEM_COLOR::SUCCESS))
			);
			sleep(concealDuration);
			who.concealed := 0;
		endif

		var elemento := "fogo";
		if (isColdArrow)
			elemento := "gelo";
		elseif (isEnergyArrow)
			elemento := "raio";
		endif

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[CHANNELED_SHOT] {targetsInLine.size()} alvos atingidos, elemento: {elemento}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _GetTargetsInLine(who, targetX, targetY)
	var validTargets := array{};
	var alreadyTargeted := array{};

	var lineCoords := GetCoordsInLine(who.x, who.y, targetX, targetY);

	foreach point in lineCoords
		var mobiles := ListMobilesNearLocation(point.x, point.y, who.z, 1, who.realm);

		foreach mobile in mobiles
			if (mobile.serial in alreadyTargeted)
				continue;
			endif
			if (mobile == who)
				continue;
			endif
			if (who.party && mobile in who.party.members)
				continue;
			endif
			if (GetObjProperty(mobile, "owner") == who.serial)
				continue;
			endif
			if (who.party)
				var skipPet := 0;
				foreach member in (who.party.members)
					if (GetObjProperty(mobile, "owner") == member.serial)
						skipPet := 1;
						break;
					endif
				endforeach
				if (skipPet)
					continue;
				endif
			endif

			validTargets.append(mobile);
			alreadyTargeted.append(mobile.serial);
		endforeach
	endforeach

	return validTargets;
endfunction

