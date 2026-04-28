use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":attributes:vitals";
include ":nature:nature";
include ":message:model/Message";
include "include/client";
include "include/packets";

/**
 * Ancestralidade Elemental (Elemental Ancestry) - ABILITY_ID::ELEMENTAL_ANCESTRY (2603)
 *
 * ON_USE: Channeled storm ability. Caster must stay still.
 *   - Duration: Theology/10 ticks (5s each)
 *   - Heals allies in 20-tile radius: Theology/5 per tick
 *   - If has souls: weather + lightning bolts on enemies (1d10+40 energy)
 *   - Wets enemy powder
 *   - Consumes all souls. Cancels on movement.
 */
class ElementalAncestry(CombatAbility)

	function ElementalAncestry(this)
		Super(ABILITY_ID::ELEMENTAL_ANCESTRY);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var mlore := CInt(mobile.GetSkill(THEOLOGY));
		var duration := CInt(mlore / 10);

		var shaman_souls := CInt(who.GetProp("shaman_souls"));
		if (!shaman_souls)
			shaman_souls := 0;
		endif

		var weatherType := 0; // rain
		var hasSouls := shaman_souls > 0;
		if (hasSouls)
			weatherType := 1; // storm
		endif

		var origx := who.x;
		var origy := who.y;

		var mobs := mobile.GetNearbyMobiles(20);
		var enemies := array{};

		Message("Voce deve permanecer parado para manter sua ancestralidade elemental.").SendSys(who);

		foreach mob in mobs
			if (mob.master != who && mob.serial != who.serial)
				enemies.append(mob);
			endif
		endforeach

		var i := 0;
		while (i < duration)
			PerformAction(who, HUMAN_ANIM::CAST_AREA);

			if ((origx != who.x) || (origy != who.y))
				who.SetProp("shaman_souls", 0);
				Message("Voce parou a tempestade.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
				break;
			endif

			if (hasSouls)
				_PlayWindSound(who);
			endif

			SetWeatherForPlayer(who, weatherType, 50);

			// Wet enemy powder
			var items := ListItemsNearLocation(who.x, who.y, who.z, 20, who.realm);
			foreach item in items
				if (item.objtype == 0x9092)
					PrintTextAbove(item, "*Molhada*");
					SubtractAmount(item, RandomInt(5) + 1);
				endif
			endforeach

			// Process enemies and heal allies
			foreach mob in enemies
				if (!mob.isUnderRoof())
					wetPowder(mob);
					SetWeatherForPlayer(mob, weatherType, 50);
				endif
			endforeach

			// Heal allies (non-enemies)
			foreach mob in mobs
				if (!(mob in enemies))
					applyEpicHealing(mob, CInt(mlore / 5));
				endif
			endforeach

			// Lightning on enemies
			var party_members := who.party ? who.party.members : array{};
			foreach mob in enemies
				if (!(mob in party_members) && distance(mob, who) < 20)
					SetWeatherForPlayer(mob, weatherType, 50);

					if (hasSouls && RandomInt(100) < mlore)
						PlayLightningBoltEffect(mob);
						ApplyDamageEX(mob, RandomDiceRoll("1d10") + 40, DMG_ENERGY);
						Message("Voce foi atingido por um raio!").SendSys(mob);
					endif
				endif
			endforeach

			// Random ambient bolts
			if (hasSouls)
				var k := 0;
				while (k < shaman_souls)
					if (RandomInt(100) < 15)
						DoBolt(who);
					endif
					k := k + 1;
				endwhile
			endif

			sleep(5);
			i := i + 1;
		endwhile

		// Cleanup weather for enemies
		foreach mob in enemies
			EraseObjProperty(mob, "molhado");
			SetWeatherForPlayer(mob, 0xFF, 1);
			MoveObjectToLocation(mob, mob.x, mob.y, mob.z, mob.realm);
			SetWeatherForPlayer(mob, 0xFF, 1);
			MoveObjectToLocation(mob, mob.x, mob.y, mob.z, mob.realm);
		endforeach

		who.SetProp("shaman_souls", 0);

		return true;
	endfunction

endclass

function _PlayWindSound(person)
	var wind := { 21, 22, 23 };
	if (RandomDiceRoll("1d4") == 2)
		PlaySoundEffect(person, wind[RandomInt(3) + 1]);
	endif
endfunction
