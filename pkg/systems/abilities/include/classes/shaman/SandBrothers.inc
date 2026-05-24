use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include "include/client";
include "include/epicCombatUtils";

/**
 * Irmaos de Areia (Sand Brothers) - ABILITY_ID::SAND_BROTHERS (2602)
 *
 * ON_USE: 4s cast animation, then:
 *   - Allies (party + pets) within 8 tiles get guaranteed next critical hit
 *   - Enemies within 8 tiles get "apodrecer" debuff (hit penalty) for duration
 *   - Duration: (Nature/10) * shaman_souls
 *   - Consumes all souls
 */
class SandBrothers(CombatAbility)

	function SandBrothers(this)
		Super(ABILITY_ID::SAND_BROTHERS);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();

		var shaman_souls := CInt(who.GetProp("shaman_souls"));
		if (!shaman_souls || shaman_souls < 1)
			Message("Voce nao possui almas coletadas.").SendSys(who);
			return false;
		endif

		// 4s cast animation
		who.frozen := 1;
		var i := 0;
		while (i < 4)
			PerformAction(who, HUMAN_ANIM::CAST_DIR);
			sleep(1);
			i++;
		endwhile
		who.frozen := 0;

		var duration := CInt(AP_GetSkill(who, NATURE) / 10) * shaman_souls;
		who.EraseProp("shaman_souls");

		// Get enemies
		var enemies := listAllMobilesNearbyButPetsAndParty(who, 8);

		// Build allies list (party members + pets)
		var allies := array{};
		if (who.party)
			foreach member in (who.party.members)
				var memberObj := SystemFindObjectBySerial(member.serial);
				if (memberObj)
					allies.append(memberObj);
				endif
			endforeach
		endif

		var ownPets := ListAllPetsAndSummonedPetsNear(who, 8);
		foreach pet in ownPets
			allies.append(pet);
		endforeach

		// Apply debuff to enemies
		foreach enemy in enemies
			PlayObjectCenteredEffectEx(enemy, FX_CURSE_SPELL, 10, 20, 0, 0, 10);
			TS_StartTimer(enemy, "apodrecer", duration);
		endforeach

		// Apply next critical to allies
		foreach ally in allies
			if (ally && ally.connected)
				ally.SetProp("#nextcritical", 1);
				PlayObjectCenteredEffectEx(ally, FX_BLOOD_DROP, 10, 20, 0, 0, 10);
			endif
		endforeach

		PlaySoundEffect(who, SFX_SPELL_SUMMON_ELEMENTAL);
		Message("Xali'Gijak. PELA HORDA!").SendSys(who);

		return true;
	endfunction

endclass
