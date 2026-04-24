use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":abilities:helpers/shinobiHelpers";
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
		if (!this)
			return false;
		endif
		var who := mobile.GetObjRef();
		var sneak := CInt(mobile.GetSkill(SNEAK));

		var area := CInt(sneak / 20);
		if (area < 1)
			area := 1;
		endif

		var duration := CInt(sneak / 5);
		if (duration < 1)
			duration := 1;
		endif

		var smoke_expires := ReadGameClock() + duration;

		// Animation
		PerformAction(who, HUMAN_ANIM::ATTACK_2HAND_BLUNT);
		PlaySoundEffect(who, 0x533);
		Sleep(1);
		// Set smoke protection on the ninja (only inside smoke area)
		SetObjProperty(
			who,
			"#smoke_state",
			struct{
				"expires" := smoke_expires,
				"x" := who.x,
				"y" := who.y,
				"radius" := area
			}
		);

		GrantPrivilege(who, "hiddenattack");
		who.enable("hiddenattack");

		// Hide the ninja
		who.concealed := 1;
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

		// Smoke visual loop - replay stationary effects each tick
		var cx := who.x;
		var cy := who.y;
		while (ReadGameClock() <= smoke_expires)
			if (!IsInShinobiSmokeZone(who))
				return true;
			endif
			for dx := -area to area
				for dy := -area to area
					if ((dx * dx + dy * dy) <= (area * area))
						PlayStationaryEffect(cx + dx, cy + dy, who.z, 0x3728, 10, 10, 0, who.realm);
					endif
				endfor
			endfor
			sleep(1);
		endwhile

		ClearSmokeProtection(who);

		return true;
	endfunction

endclass
