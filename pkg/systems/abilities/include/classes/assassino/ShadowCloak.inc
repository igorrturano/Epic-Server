use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":attributes:skillCheck";

/**
 * Manto das Sombras (Shadow Cloak) - ABILITY_ID::MANTO_DAS_SOMBRAS (1201)
 *
 * Passive (AfterDamage): After landing a backstab, tests SNEAK vs
 * defender's AWARENESS. If successful, re-enters stealth.
 */
class ShadowCloak(CombatAbility)

	function ShadowCloak(this)
		Super(ABILITY_ID::MANTO_DAS_SOMBRAS);
	endfunction

	function OnPassiveAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (!who.hidden)
			return false;
		endif

		// Sneak vs Awareness contest
		var defender := attackInfo.GetDefender();
		var awarenessSkill := CInt(AP_GetSkill(defender.GetObjRef(), AWARENESS));

		if (SkillCheck(who, SNEAK, awarenessSkill, 0, ADV_DISABLE) <= 0)
			DebugMsg(who, DEBUG_HAB,
				Message("[MANTO_DAS_SOMBRAS] Falhou no teste de furtividade.", Color(SYSTEM_COLOR::FAIL))
			);
			return false;
		endif

		// Re-enter stealth
		var sneakSkill := CInt(AP_GetSkill(who, SNEAK));
		var steps := CInt(sneakSkill / 10);
		if (steps < 3)
			steps := 3;
		endif

		who.hidden := 1;
		who.stealthsteps := steps;

		Message("*desaparece nas sombras*").SendEmote(who);
		DebugMsg(who, DEBUG_HAB,
			Message($"[MANTO_DAS_SOMBRAS] Re-stealth! Steps: {steps}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
