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
 * Active (ON_USE): Toggle. Shows gump to choose Fortitude/Reflexes/Will.
 * Adds ALCHEMY/20 bonus to the chosen resistance via #FortitudeMod/#ReflexosMod/#VontadeMod.
 * Second use removes the bonus.
 */
class AdaptableForm(CombatAbility)

	function AdaptableForm(this)
		Super(ABILITY_ID::FORMA_ADAPTAVEL);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var resist := GetObjProperty(who, "formaadaptavel");
		var bonus := CInt(AP_GetSkill(who, ALCHEMY) / 20);

		if (resist == error)
			var prot;
			if (mobile.IsNPC())
				prot := "Fortitude";
			else
				prot := RadioGump(who, 200, 260, "Escolha a resistencia", { "Fortitude", "Reflexos", "Vontade" });
			endif

			if (!prot)
				return false;
			endif

			Message($"Voce escolheu aprimorar {prot}", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			if (prot == "Fortitude")
				SetObjProperty(who, "formaadaptavel", FORTITUDE);
				SetObjProperty(who, "Mutacao", bonus);
				SetObjProperty(who, "#FortitudeMod", bonus + CInt(GetObjProperty(who, "#FortitudeMod")));
			elseif (prot == "Reflexos")
				SetObjProperty(who, "formaadaptavel", REFLEXES);
				SetObjProperty(who, "Mutacao", bonus);
				SetObjProperty(who, "#ReflexosMod", bonus + CInt(GetObjProperty(who, "#ReflexosMod")));
			elseif (prot == "Vontade")
				SetObjProperty(who, "formaadaptavel", WILL);
				SetObjProperty(who, "Mutacao", bonus);
				SetObjProperty(who, "#VontadeMod", bonus + CInt(GetObjProperty(who, "#VontadeMod")));
			else
				return false;
			endif

			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[FORMA_ADAPTAVEL] Resistencia {prot} +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
			);
		elseif (resist == FORTITUDE)
			EraseObjProperty(who, "formaadaptavel");
			SetObjProperty(who, "#FortitudeMod", CInt(GetObjProperty(who, "#FortitudeMod")) - CInt(GetObjProperty(who, "Mutacao")));
			EraseObjProperty(who, "Mutacao");
			Message("Voce volta ao normal.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		elseif (resist == REFLEXES)
			EraseObjProperty(who, "formaadaptavel");
			SetObjProperty(who, "#ReflexosMod", CInt(GetObjProperty(who, "#ReflexosMod")) - CInt(GetObjProperty(who, "Mutacao")));
			EraseObjProperty(who, "Mutacao");
			Message("Voce volta ao normal.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		elseif (resist == WILL)
			EraseObjProperty(who, "formaadaptavel");
			SetObjProperty(who, "#VontadeMod", CInt(GetObjProperty(who, "#VontadeMod")) - CInt(GetObjProperty(who, "Mutacao")));
			EraseObjProperty(who, "Mutacao");
			Message("Voce volta ao normal.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		endif

		return true;
	endfunction

endclass
