use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include "include/client";

/**
 * Cortina de Fumaca (Smoke Screen) - ABILITY_ID::SMOKE_SCREEN (1302)
 *
 * Passive: Doubles stealth steps, removes distance penalty for detection.
 * Active (OnUse): Creates a smoke cloud that hides all party members.
 *   Only the ninja can attack without revealing while inside the smoke.
 *   Area = Tactics/20, Duration = Tactics/5 seconds.
 */
class SmokeScreen(CombatAbility)

	function SmokeScreen(this)
		Super(ABILITY_ID::SMOKE_SCREEN);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var tactics := CInt(mobile.GetSkill(TACTICS));

		var area := CInt(tactics / 20);
		if (area < 1)
			area := 1;
		endif

		var duration := CInt(tactics / 5);
		if (duration < 3)
			duration := 3;
		endif

		// Animation
		PerformAction(who, HUMAN_ANIM::CAST_AREA);
		PlaySoundEffect(who, 0x228);

		// Create smoke visual effects
		var smoke_tiles := array{};
		for dx := -area to area
			for dy := -area to area
				if ((dx * dx + dy * dy) <= (area * area))
					var sx := who.x + dx;
					var sy := who.y + dy;
					var smoke_item := CreateItemAtLocation(sx, sy, who.z, 0x3728, 1, who.realm);
					if (smoke_item)
						smoke_item.movable := 0;
						SetObjProperty(smoke_item, "Timer", ReadGameClock() + duration);
						smoke_tiles.append(smoke_item);
					endif
				endif
			endfor
		endfor

		// Play smoke particle effects
		foreach tile in smoke_tiles
			PlayObjectCenteredEffect(tile, 0x3728, 10, duration * 10);
			sleepms(2);
		endforeach

		// Set smoke protection on the ninja (only the ninja attacks without revealing)
		var smoke_expires := ReadGameClock() + duration;
		SetObjProperty(who, "#smoke_protected", smoke_expires);

		// Hide the ninja
		who.hidden := 1;
		Message("*Lanca uma bomba de fumaca!*").SendEmote(who);

		// Hide party members in range (they only get hidden, reveal normally on attack)
		if (who.party)
			foreach member in (who.party.members)
				if (member.serial != who.serial && Distance(who, member) <= area)
					member.hidden := 1;
					Message("*Escondido pela fumaca*").SendAbove(member);
				endif
			endforeach
		endif

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[SMOKE_SCREEN] Area: {area}, Duracao: {duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
