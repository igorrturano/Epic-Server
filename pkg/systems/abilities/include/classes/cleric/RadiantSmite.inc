include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":attributes:attributes";

/**
 * Golpe Radiante (Radiant Smite) - ABILITY_ID::GOLPE_RADIANTE (2801)
 *
 * Active (OnUse):
 * - Religion-specific targeted attack/debuff ability
 * - Scaling: theology_bonus = Theology / 10
 * - Range: 8 tiles, requires line of sight
 *
 * Religion effects:
 *   ELUITHORTH: Matriarchal dominance (start_script)
 *   BOKK_BHARAZ: Stormy hammer (start_script)
 *   HALLEUAD: Blinds target
 *   VENERATIO: Stat debuffs (STR/DEX/INT, capped 50%)
 *   KAI: Marks target as "exposed"
 */
class RadiantSmite(CombatAbility)

	function RadiantSmite(this)
		Super(ABILITY_ID::GOLPE_RADIANTE);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var religiao := mobile.GetReligion();
		var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 10);

		case (religiao)
			ELUITHORTH:
				start_script(":charactercreation:habilidades/dominanciamatriarcal", who);

			BOKK_BHARAZ:
				start_script(":charactercreation:habilidades/martelotempestuoso", array{ who });

			HALLEUAD:
				var targ := GetValidTarget(who);
				if (!targ)
					return false;
				endif
				PlaySoundEffect(targ, 0x1F8);
				TS_StartTimer(targ, "blinded", theology_bonus, 0, who);
				Message("*cegado pela luz divina*").SendAbove(targ);

			VENERATIO:
				var targ := GetValidTarget(who);
				if (!targ)
					return false;
				endif

				var fx := CreateItemAtLocation(targ.x, targ.y, targ.z + 10, FX_MORALE_DOWN, 1, targ.realm);
				Sleepms(800);
				DestroyItem(fx);
				PlaySoundEffect(targ, 0x58E);

				var debuff_percent := CInt(theology_bonus * 2);
				if (debuff_percent > 50)
					debuff_percent := 50;
				endif

				// DEX debuff
				var dex_val := AP_GetStat(targ, DEXTERITY);
				var dex_points := CInt((dex_val * debuff_percent) / 100);
				TS_StartTimer(targ, "agility", theology_bonus, -dex_points, who, 1);

				// STR debuff
				var str_val := AP_GetStat(targ, STRENGTH);
				var str_points := CInt((str_val * debuff_percent) / 100);
				TS_StartTimer(targ, "strength", theology_bonus, -str_points, who, 1);

				// INT debuff
				var int_val := AP_GetStat(targ, INTELLIGENCE);
				var int_points := CInt((int_val * debuff_percent) / 100);
				TS_StartTimer(targ, "intelligence", theology_bonus, -int_points, who, 1);

				Message("*enfraquecido pela ira divina*").SendAbove(targ);
				Message("Seu alvo foi enfraquecido pela ira sagrada!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			KAI:
				var targ := GetValidTarget(who);
				if (!targ)
					return false;
				endif
				PlaySoundEffect(targ, 0x51D);
				TS_StartTimer(targ, "exposed", theology_bonus, 0, who);
				Message("Seu alvo esta exposto!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				Message("Voce foi marcado! Sua defesa esta comprometida!", Color(SYSTEM_COLOR::FAIL)).SendSys(targ);
		endcase

		return true;
	endfunction

endclass

function GetValidTarget(who)
	var targ := Target(who);
	if (!targ)
		Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	if (!CheckLineOfSight(who, targ))
		Message("Voce nao consegue ver o alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	if (Distance(who, targ) > 8)
		Message("Muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
		return 0;
	endif

	return targ;
endfunction
