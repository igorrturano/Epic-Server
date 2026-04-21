use uo;
use os;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":tn:cooldown";
include ":fls_core:packets";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":survival:fishingFunctions";
include "include/epicConstants";
include "include/sounds";

/**
 * Prismatic Shot (Disparo Prismatico) - ABILITY_ID::PRISMATIC_SHOT (2404)
 *
 * Active (OnUse): Always fires 3 shots in sequence, one per element (cold, energy, fire).
 * The active element of the arcane bow defines the special effect of the 3rd shot:
 * - fire_arrow   → fireball on target
 * - energy_arrow → ethereal jump (teleport to targeted location)
 * - cold_arrow   → freezes the target (congelamento)
 * Requires active arcane_bow.
 */
class PrismaticShot(CombatAbility)

	function PrismaticShot(this)
		Super(ABILITY_ID::PRISMATIC_SHOT);
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

		var activeElement := "fire_arrow";
		if (GetObjProperty(weaponRef, "cold_arrow"))
			activeElement := "cold_arrow";
		elseif (GetObjProperty(weaponRef, "energy_arrow"))
			activeElement := "energy_arrow";
		endif

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
			Message("Disparo cancelado.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
			return false;
		endif

		if (!CheckLineOfSight(who, targ))
			Message("Voce nao tem linha de visao para o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		Message("*Disparo Prismatico*").SendAbove(who);

		// Shot 1 — Cold
		PlaySoundEffect(who, 0x5c8);
		PlayMovingEffect(who, targ, 0xe3e5, 10);
		who.attack_once(targ);
		sleep(1);

		// Shot 2 — Energy
		PlaySoundEffect(who, 0x666);
		PlayMovingEffect(who, targ, FX_ELETRIC_BOLT, 10);
		who.attack_once(targ);
		sleep(1);

		// Shot 3 — Fire + active element special effect
		PlaySoundEffect(who, 0x5cb);
		PlayMovingEffect(who, targ, FX_FIRE_ARROW, 10);
		who.attack_once(targ);

		case (activeElement)
			"fire_arrow":
				Run_Script(":spells:spell_logic/mage/fireball", { who, targ });

			"energy_arrow":
				_EtherealJump(who, mobile, targ);

			"cold_arrow":
				var freezeDuration := CInt(mobile.GetSkill(ARCANA) / 10);
				TS_StartTimer(targ, "congelamento", freezeDuration);
				Message($"*O alvo e congelado por {freezeDuration}s*").SendAbove(who);
		endcase

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[PRISMATIC_SHOT] 3 disparos executados, efeito final: {activeElement}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

function _EtherealJump(who, mobile, opponent := 0)
	who.concealed := 1;
	who.frozen := 1;
	PerformAction(who, HUMAN_ANIM::CAST_AREA);
	PlayStationaryEffect(who.x, who.y, who.z, 0xD11B, 0, 35, 0, who.realm);
	PlaySoundEffect(who, 0x1f2);
	sleep(1);

	var targ;
	if (mobile.IsNPC() && opponent)
		// Jump to a random cardinal direction 5 tiles from the opponent (matches EtherealJump.inc NPC behavior)
		var directions := array{ array{ 1, 0 }, array{ 0, 1 }, array{ -1, 0 }, array{ 0, -1 } };
		var random_direction := directions[RandomInt(4) + 1];
		targ := struct{
			x := opponent.x + random_direction[1] * 5,
			y := opponent.y + random_direction[2] * 5,
			z := opponent.z,
			realm := opponent.realm
		};
	else
		targ := TargetCoordinates(who);
	endif

	if (!targ)
		Message("Salto cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		who.concealed := 0;
		who.frozen := 0;
		return;
	endif

	var arcana := CInt(mobile.GetSkill(ARCANA));
	var maxRange := CInt(arcana / 5);
	var dist := Distance(who, targ);

	if (!CheckLosAt(who, targ.x, targ.y, targ.z))
		Message("Voce nao consegue ver o destino.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		who.concealed := 0;
		who.frozen := 0;
		return;
	endif

	if (dist > maxRange)
		Message($"Destino muito distante. Range maximo: {maxRange} tiles.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		who.concealed := 0;
		who.frozen := 0;
		return;
	endif

	var locinfo := GetMapInfo(targ.x, targ.y, who.realm);
	if (IsWater(targ, locinfo))
		Message("Voce nao pode saltar para a agua.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		who.concealed := 0;
		who.frozen := 0;
		return;
	endif

	if (CanWalk("L", who.x, who.y, who.z, targ.x, targ.y, who.realm) == error)
		Message("Local inacessivel.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		who.concealed := 0;
		who.frozen := 0;
		return;
	endif

	MoveObjectToLocation(who, targ.x, targ.y, targ.z, who.realm);
	PlayStationaryEffect(who.x, who.y, who.z, 0xD11B, 0, 35, 0, who.realm);
	PlaySoundEffect(who, 0x1f2);
	sleep(1);

	who.concealed := 0;
	who.frozen := 0;
endfunction
