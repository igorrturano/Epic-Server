use uo;
use os;
use cfgfile;
use vitals;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include "include/facings";
include "include/client";

/**
 * Furia do Dragao / Dragon's Fury - ABILITY_ID::DRAGON_FURY (801)
 *
 * Active (OnUse): Charge forward 5 tiles in facing direction,
 * dealing Tactics/2 damage to all enemies in the path.
 */
class DragonFury(CombatAbility)

	function DragonFury(this)
		Super(ABILITY_ID::DRAGON_FURY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		if (who.shield)
			Message("Um verdadeiro samurai nao usaria escudo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif
		return true;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		// Play weapon animation + sound
		var cfg := ReadConfigFile(":combat:itemdesc");
		var elem := FindConfigElem(cfg, who.weapon.objtype);
		if (elem)
			PerformAction(who, elem.Anim);
		endif
		PlaySoundEffect(who, 0x520);
		Message("*Furia do Dragao!*").SendAbove(who);

		// Get facing direction
		var mod := GetModByFacing(who.facing);
		var dx := mod[1];
		var dy := mod[2];

		var dist := 5;
		var dmg := CInt(mobile.GetSkill(TACTICS) / 2);
		var alreadyDamaged := array{};
		var hostiles := mobile.GetNearbyHostiles(20, LH_FLAG_LOS);

		// KAI religion visual hue (GetReligion is NPC-safe; returns IsNPCException when absent)
		var hue := 0;
		if (mobile.GetReligion() == KAI)
			hue := 2597;
		endif

		for i := 1 to dist
			var nextx := who.x + dx;
			var nexty := who.y + dy;

			if (!CheckLosAt(who, nextx, nexty, who.z))
				break;
			endif

			// Visual effect at current position
			PlayStationaryEffectEx(who.x, who.y, who.z, who.realm, 0x6d57, 10, 20, hue, 0);

			// Damage hostiles at current position
			foreach mob in (hostiles)
				if (!(mob.serial in alreadyDamaged) &&
					mob.serial != who.serial &&
					CoordinateDistance(mob.x, mob.y, who.x, who.y) <= 1)

					alreadyDamaged.append(mob.serial);
					ApplyRawDamage(mob, dmg);
				endif
			endforeach

			// Advance the samurai
			var err := MoveObjectToLocation(who, nextx, nexty, who.z, who.realm);
			if (err == error)
				break;
			endif
			sleepms(50);
		endfor

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[DRAGON_FURY] Carga: {dist} tiles, Alvos: {alreadyDamaged.size()}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
