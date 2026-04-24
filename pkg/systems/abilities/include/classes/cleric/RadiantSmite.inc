include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":attributes:attributes";
include ":wrappers:Objects/Model/WeaponWrapper";

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
				if (mobile.IsNPC())
					return false;
				endif
				var dm_targ := Target(who);
				if (!dm_targ || !dm_targ.isA(POLCLASS_MOBILE))
					Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				if (Distance(who, dm_targ) > 3)
					Message("O alvo esta muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				if (!who.weapon || who.weapon.objtype != 0x166E)
					Message("Voce precisa estar usando chicote para usar essa habilidade.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				var dm_dano := mobile.GetSkill(THEOLOGY);
				var dm_isInDarkness := (GetGlobalProperty("dia") != "dia") || (who.realm != "britannia");
				mobile.SetProperty("#GuaranteedHit", 1);
				if (dm_isInDarkness)
					PlaySoundEffect(who, 0x685);
					TS_StartTimer(dm_targ, "paralysis", theology_bonus);
				endif
				ApplyRawDamage(dm_targ, CInt(dm_dano / 5));
				ApplyRawDamage(dm_targ, CInt(dm_dano / 5));
				Message("*chicotada dominante*").SendAbove(who);

			BOKK_BHARAZ:
				if (mobile.IsNPC())
					return false;
				endif
				var mt_targ := Target(who);
				if (!mt_targ || !mt_targ.isA(POLCLASS_MOBILE))
					Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				if (Distance(who, mt_targ) > 6)
					Message("O alvo esta muito distante.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				if (!who.weapon)
					Message("Voce precisa utilizar armas contusivas.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				var mt_weapon := WeaponWrapper(who.weapon);
				if (mt_weapon.GetAttackType() != "Contusivo")
					Message("Voce precisa utilizar armas contusivas.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				PerformAction(who, 12);
				PlayMovingEffect(who, mt_targ, FX_HAMMER_THROW, 0, 40);
				sleep(2);
				PlayMovingEffect(mt_targ, who, FX_HAMMER_THROW, 0, 40);
				TS_StartTimer(mt_targ, "paralysis", theology_bonus);
				var mt_dano := CInt(mobile.GetSkill(THEOLOGY) / 2);
				ApplyRawDamage(mt_targ, mt_dano);
				var mt_lightning := {41, 42, 519};
				PlaySoundEffect(mt_targ, mt_lightning[RandomInt(3) + 1]);

			HALLEUAD:
				var targ := GetValidTarget(mobile, who);
				if (!targ)
					return false;
				endif
				PlaySoundEffect(targ, 0x1F8);
				TS_StartTimer(targ, "blinded", theology_bonus, 0, who);
				Message("*cegado pela luz divina*").SendAbove(targ);

			VENERATIO:
				var targ := GetValidTarget(mobile, who);
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
				var targ := GetValidTarget(mobile, who);
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

function GetValidTarget(mobile, who)
	var targ;
	if (mobile.IsNPC())
		targ := who.opponent;
		if (!targ)
			targ := GetRandomNearbyHostile(who);
		endif
	else
		targ := Target(who);
	endif

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
