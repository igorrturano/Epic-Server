use uo;
use os;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include "include/client";

/**
 * Tora no Kamae / Postura do Tigre (Tiger Stance) - ABILITY_ID::TIGER_STANCE (703)
 *
 * ON_USE: Six inch strike — guaranteed hit + disarm + knockback along facing direction.
 * Mobs along the knockback path take Tactics/2 damage.
 * On obstacle collision: extra Tactics/2 damage.
 *
 * Based on V1 golpeseispolegadas.src.
 */
class TigerStance(CombatAbility)

	function TigerStance(this)
		Super(ABILITY_ID::TIGER_STANCE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (!who.opponent)
			Message("Voce precisa estar em combate.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var defender := who.opponent;
		var tactics := mobile.GetSkill(TACTICS);

		// Knockback distance: Tactics/20 tiles (min 1)
		var dist := CInt(tactics / 20);
		if (dist < 1)
			dist := 1;
		endif

		// Direction from defender's facing (pushes them backwards/away from attacker)
		var dx := 0;
		var dy := 0;
		case (defender.facing)
			0: dy := 1;
			1: dy := 1; dx := -1;
			2: dx := -1;
			3: dx := -1; dy := -1;
			4: dy := -1;
			5: dx := 1; dy := -1;
			6: dx := 1;
			7: dx := 1; dy := 1;
		endcase

		// Guaranteed hit + initial attack
		SetObjProperty(who, "#GuaranteedHit", 1);
		who.attack_once(defender);
		PlaySoundEffect(who, 0x50F);

		// Disarm: move weapon to backpack
		var weapon := GetEquipmentByLayer(defender, LAYER_HAND1);
		if (!weapon)
			weapon := GetEquipmentByLayer(defender, LAYER_HAND2);
		endif
		if (weapon && !defender.npctemplate)
			MoveItemToContainer(weapon, defender.backpack);
			Message("*e desarmado!*").SendAbove(defender);
		endif

		// Knockback loop + track path positions
		var dmg := CInt(tactics / 2);
		var pathPositions := array{};

		for i := 1 to dist
			var finalx := defender.x + dx;
			var finaly := defender.y + dy;
			pathPositions.append(struct{ x := finalx, y := finaly });

			var err := MoveObjectToLocation(defender, finalx, finaly, defender.z, defender.realm);
			sleepms(300);

			if (err == error)
				ApplyRawDamage(defender, dmg);
				Message("*bate no obstaculo*").SendAbove(defender);
				break;
			endif
		endfor

		// AoE: damage hostiles along the knockback path
		var alreadyDamaged := array{};
		var hostiles := ListHostiles(who, 20, LH_FLAG_LOS);

		foreach pos in (pathPositions)
			foreach mob in (hostiles)
				if (!(mob.serial in alreadyDamaged) &&
					mob.serial != who.serial &&
					mob.serial != defender.serial &&
					CoordinateDistance(mob.x, mob.y, pos.x, pos.y) <= 1)

					alreadyDamaged.append(mob.serial);
					ApplyRawDamage(mob, dmg);
				endif
			endforeach
		endforeach

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[TIGER_STANCE] Knockback: {dist} tiles, AoE: {alreadyDamaged.size()} alvos", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
