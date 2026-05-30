use uo;

include ":abilities:models/CombatAbility";
include ":attributes:attributes";
include ":tn:tngumps";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Mutagenic (Mutagênicos) - ABILITY_ID::MUTAGENICO (904)
 *
 * Active (ON_USE): Toggle. First use: gump to choose STR/DEX/INT.
 * Grants +25 stat mod + +25 vital mod (hitsmod/stammod/manamod) + description.
 * Second use removes all bonuses.
 */
class Mutagenic(CombatAbility)

	function Mutagenic(this)
		Super(ABILITY_ID::MUTAGENICO);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var mutante := GetObjProperty(who, "mutagenico");

		recalcvitals(who);

		if (mutante == error)
			var opts := array{ "Forca", "Destreza", "Inteligencia" };
			var prot;
			if (mobile.IsNPC())
				prot := opts[RandomIntMinMax(1, opts.size())];
			else
				prot := RadioGump(who, 200, 260, "Escolha o mutagenico", opts);
			endif

			if (!prot)
				return false;
			endif

			Message($"Voce escolheu mutagenico de {prot}", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			if (prot == "Forca")
				SetObjProperty(who, "mutagenico", 1);
				AP_SetStatMod(who, STRENGTH, 25);
				var hitsmod := CInt(GetObjProperty(who, "hitsmod"));
				hitsmod := hitsmod + 25;
				SetObjProperty(who, "hitsmod", hitsmod);
				SetObjProperty(who, "description", "Veias saltam para fora da pele <br>");
			elseif (prot == "Destreza")
				SetObjProperty(who, "mutagenico", 2);
				AP_SetStatMod(who, DEXTERITY, 25);
				var stammod := CInt(GetObjProperty(who, "stammod"));
				stammod := stammod + 25;
				SetObjProperty(who, "stammod", stammod);
				SetObjProperty(who, "description", "Pupilas contraidas como as de um felino <br>");
			elseif (prot == "Inteligencia")
				SetObjProperty(who, "mutagenico", 3);
				AP_SetStatMod(who, INTELLIGENCE, 25);
				var manamod := CInt(GetObjProperty(who, "manamod"));
				manamod := manamod + 25;
				SetObjProperty(who, "manamod", manamod);
				SetObjProperty(who, "description", "Olhos Brancos <br>");
			else
				return false;
			endif

			recalcvitals(who);
			DebugMsg(
				who,
				DEBUG_HAB,
				Message($"[MUTAGENICO] Tipo: {prot}", Color(SYSTEM_COLOR::SUCCESS))
			);

		elseif (mutante == 1)
			EraseObjProperty(who, "mutagenico");
			AP_SetStatMod(who, STRENGTH, 0);
			var hitsmod := CInt(GetObjProperty(who, "hitsmod"));
			hitsmod := hitsmod - 25;
			SetObjProperty(who, "hitsmod", hitsmod);
			EraseObjProperty(who, "description");
			recalcvitals(who);
			Message("Voce se livra dos efeitos do mutagenico.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);

		elseif (mutante == 2)
			EraseObjProperty(who, "mutagenico");
			AP_SetStatMod(who, DEXTERITY, 0);
			var stammod := CInt(GetObjProperty(who, "stammod"));
			stammod := stammod - 25;
			SetObjProperty(who, "stammod", stammod);
			EraseObjProperty(who, "description");
			recalcvitals(who);
			Message("Voce se livra dos efeitos do mutagenico.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);

		elseif (mutante == 3)
			EraseObjProperty(who, "mutagenico");
			AP_SetStatMod(who, INTELLIGENCE, 0);
			var manamod := CInt(GetObjProperty(who, "manamod"));
			manamod := manamod - 25;
			SetObjProperty(who, "manamod", manamod);
			EraseObjProperty(who, "description");
			recalcvitals(who);
			Message("Voce se livra dos efeitos do mutagenico.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		endif

		return true;
	endfunction

endclass
