use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":tn:tngumps";
include ":charactercreation:resists";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Adaptable Form (Forma Adaptável) - ABILITY_ID::FORMA_ADAPTAVEL (900)
 *
 * Active (ON_USE): toggle. First use shows a gump to pick
 * Fortitude/Reflexos/Vontade and adds a SCIENCE/20 bonus to the matching
 * #FortitudeMod/#ReflexosMod/#VontadeMod. Second use removes exactly what
 * it added.
 *
 * State lives in two ObjProperties:
 *   formaadaptavel - active stat marker (FORTITUDE/REFLEXES/WILL), the
 *                    AF_PENDING sentinel while the gump is open, or absent
 *                    when inactive.
 *   Mutacao        - the exact bonus currently applied (also the
 *                    re-fire guard read by the NPC brain).
 *
 * The marker is committed LAST (after Mutacao and the #XxxMod bump) so a
 * crash mid-toggle leaves a pending half-state the login sweep can clear
 * without ever subtracting a bonus that was never added.
 */
const AF_PENDING := "pending";

class AdaptableForm(CombatAbility)

	function AdaptableForm(this)
		Super(ABILITY_ID::FORMA_ADAPTAVEL);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var state := GetObjProperty(who, "formaadaptavel");

		if (state == FORTITUDE)
			return _AF_Revert(who, "#FortitudeMod");
		elseif (state == REFLEXES)
			return _AF_Revert(who, "#ReflexosMod");
		elseif (state == WILL)
			return _AF_Revert(who, "#VontadeMod");
		elseif (state == AF_PENDING)
			// Another activation is still on the gump (or its client
			// dropped). Opening a second one is the double-fire that
			// used to strand the modifier — refuse instead.
			Message("Você ainda está se adaptando, aguarde.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		elseif (state != error)
			// Unknown/legacy marker we can't map. Clear it so the
			// ability isn't bricked; the login sweep does deeper repair.
			EraseObjProperty(who, "formaadaptavel");
		endif

		var bonus := CInt(AP_GetSkill(who, SCIENCE) / 20);
		var prot;
		if (mobile.IsNPC())
			prot := "Fortitude";
		else
			// Claim the slot BEFORE the blocking gump so a second use
			// lands on the AF_PENDING branch instead of applying again.
			SetObjProperty(who, "formaadaptavel", AF_PENDING);
			prot := RadioGump(who, 200, 260, "Escolha a resistencia", { "Fortitude", "Reflexos", "Vontade" });
			if (!prot)
				EraseObjProperty(who, "formaadaptavel");
				return false;
			endif
		endif

		Message($"Voce escolheu aprimorar {prot}", Color(SYSTEM_COLOR::INFO)).SendSys(who);

		if (prot == "Fortitude")
			_AF_Apply(who, FORTITUDE, "#FortitudeMod", bonus);
		elseif (prot == "Reflexos")
			_AF_Apply(who, REFLEXES, "#ReflexosMod", bonus);
		elseif (prot == "Vontade")
			_AF_Apply(who, WILL, "#VontadeMod", bonus);
		else
			EraseObjProperty(who, "formaadaptavel");
			return false;
		endif

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[FORMA_ADAPTAVEL] Resistencia {prot} +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass

// Record magnitude, bump the mod, then commit the marker last so the only
// fully-committed state is one where the bonus is provably added.
function _AF_Apply(who, statMarker, modKey, bonus)
	SetObjProperty(who, "Mutacao", bonus);
	SetObjProperty(who, modKey, bonus + CInt(GetObjProperty(who, modKey)));
	SetObjProperty(who, "formaadaptavel", statMarker);
endfunction

// Subtract exactly what we recorded — symmetric with _AF_Apply, so it
// can't go net-negative no matter what else touched the shared mod.
function _AF_Revert(who, modKey)
	var delta := CInt(GetObjProperty(who, "Mutacao"));
	SetObjProperty(who, modKey, CInt(GetObjProperty(who, modKey)) - delta);
	EraseObjProperty(who, "Mutacao");
	EraseObjProperty(who, "formaadaptavel");
	Message("Voce volta ao normal.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
	return true;
endfunction