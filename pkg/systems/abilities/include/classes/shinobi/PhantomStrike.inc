use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include "include/client";

/**
 * Golpe Fantasma (Phantom Strike) - ABILITY_ID::PHANTOM_STRIKE (1304)
 *
 * Active (OnUse): Select up to Tactics/20 nearby opponents, teleport behind
 *   each one attacking and paralyzing them. Re-hides after execution.
 */
class PhantomStrike(CombatAbility)

	function PhantomStrike(this)
		Super(ABILITY_ID::PHANTOM_STRIKE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var sneak := CInt(mobile.GetSkill(SNEAK));

		var maxTargets := CInt(sneak / 20);
		if (maxTargets < 1)
			maxTargets := 1;
		endif

		var paralyze_duration := CInt(sneak / 10);
		if (paralyze_duration < 1)
			paralyze_duration := 1;
		endif

		var range := CInt(sneak / 10);
		if (range < 5)
			range := 5;
		endif

		// Get hostile targets in range
		var hostiles := mobile.GetNearbyHostiles(range, LH_FLAG_LOS);
		if (!hostiles || len(hostiles) == 0)
			Message("Nenhum inimigo proximo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		// Limit to max targets
		var targets := array{};
		var count := 0;
		foreach mob in hostiles
			if (count >= maxTargets)
				break;
			endif
			targets.append(mob);
			count := count + 1;
		endforeach

		// Animation start
		Message("*Golpe Fantasma!*").SendEmote(who);
		PlaySoundEffect(who, 0x51C);

		// Strike each target
		foreach targ in targets
			if (targ && !targ.dead)
				// Teleport behind target
				PlaySoundEffect(who, 0x513);
				mobile.TeleportBehind(MobileWrapper(targ));

				// Guaranteed hit + attack
				SetObjProperty(who, "#GuaranteedHit", 1);
				who.attack_once(targ);

				// Paralyze
				TS_StartTimer(targ, "paralysis", paralyze_duration);
				Message("*paralisado*").SendAbove(targ);

				PlaySoundEffect(targ, 0x204);
				sleepms(200);
			endif
		endforeach

		// Re-hide after execution
		who.hidden := 1;

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[PHANTOM_STRIKE] Targets: {len(targets)}, Paralyze: {paralyze_duration}s", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
