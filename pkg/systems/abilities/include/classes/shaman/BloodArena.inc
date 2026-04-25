use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":tn:cooldown";
include ":combat:Controller/DamageController_legacy";
include ":message:model/Message";
include "include/client";

/**
 * Arena de Sangue (Blood Arena) - ABILITY_ID::BLOOD_ARENA (2604)
 *
 * ON_USE: Target an enemy to:
 *   - Paralyze for Theology/10 seconds
 *   - Apply "apodrecer" debuff (hit penalty) for shaman_souls + duration seconds
 *   - Deal Theology + (shaman_souls * 5) poison damage
 *   - Consumes all souls
 */
class BloodArena(CombatAbility)

	function BloodArena(this)
		Super(ABILITY_ID::BLOOD_ARENA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var defender;
		if (mobile.IsNPC())
			defender := who.opponent;
			if (!defender)
				defender := GetRandomNearbyHostile(who);
			endif
		else
			defender := Target(who);
		endif

		if (!defender || !defender.isA(POLCLASS_MOBILE))
			Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		var duration := CInt(mobile.GetSkill(THEOLOGY) / 10);
		var shaman_souls := CInt(who.GetProp("shaman_souls"));
		if (!shaman_souls)
			shaman_souls := 0;
		endif

		if (shaman_souls > 0)
			PerformAction(who, HUMAN_ANIM::CAST_DIR);

			// Paralysis
			TS_StartTimer(defender, "paralysis", duration);

			// Terror (apodrecer debuff - applies hit chance penalty)
			TS_StartTimer(defender, "apodrecer", shaman_souls + duration);

			PlaySoundEffect(defender, 0x654);
			PlayObjectCenteredEffectHuefx(defender, 0x6c91, 1, 30, 2731, 0);
			Message("Voce esta aterrorizado com visoes de morte.").SendSys(defender);
		endif

		// Poison damage
		var dano_mlore := CInt(mobile.GetSkill(THEOLOGY) + (shaman_souls * 5));
		DamageFLS(defender, dano_mlore, DMG_POISON, who);

		who.SetProp("shaman_souls", 0);

		return true;
	endfunction

endclass
