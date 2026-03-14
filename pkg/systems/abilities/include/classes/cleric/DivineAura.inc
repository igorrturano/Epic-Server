include ":abilities:models/CombatAbility";
include ":timedscripts:timedScripts";
include ":attributes:attributes";

/**
 * Aura Divina (Divine Aura) - ABILITY_ID::AURA_DIVINA (2802)
 *
 * Active (OnUse):
 * - Religion-specific targeted buff/summon ability
 * - Scaling: theology_bonus = Theology / 5
 *
 * Religion effects:
 *   ELUITHORTH: Summons dark spiders (start_script prolesombria)
 *   BOKK_BHARAZ: Anvil jump (start_script saltobigorna)
 *   HALLEUAD: Speed buff on target
 *   VENERATIO: Stat buffs (STR/DEX/INT, capped 50%) on target
 *   KAI: Hit accuracy bonus on target
 */
class DivineAura(CombatAbility)

	function DivineAura(this)
		Super(ABILITY_ID::AURA_DIVINA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var religiao := mobile.GetReligion();
		var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 5);

		PerformAction(who, HUMAN_ANIM::CAST_DIR);

		case (religiao)
			ELUITHORTH:
				var targ := Target(who);
				if (!targ)
					Message("Voce deve selecionar um local.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				start_script(":charactercreation:habilidades/prolesombria", array{ who, targ });

			BOKK_BHARAZ:
				start_script(":charactercreation:habilidades/saltobigorna", array{ who });

			HALLEUAD:
				var targ := Target(who);
				if (!targ)
					Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				PlayObjectCenteredEffectEx(targ, FX_SPEED_BOOTS, 7, 32, 0, 1);
				PlaySoundEffect(targ, 0x20F);
				TS_StartTimer(targ, "velocidade", theology_bonus);

			VENERATIO:
				var targ := Target(who);
				if (!targ)
					Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif

				PlayObjectCenteredEffectEx(targ, FX_MORALE_UP, 7, 32);
				PlaySoundEffect(targ, 0x1F5);

				var buff_percent := CInt(theology_bonus * 2);
				if (buff_percent > 50)
					buff_percent := 50;
				endif

				var buff_duration := theology_bonus * 2;

				// STR buff
				var str_val := AP_GetStat(targ, STRENGTH);
				var str_points := CInt((str_val * buff_percent) / 100);
				TS_StartTimer(targ, "strength", buff_duration, str_points, who, 1);

				// DEX buff
				var dex_val := AP_GetStat(targ, DEXTERITY);
				var dex_points := CInt((dex_val * buff_percent) / 100);
				TS_StartTimer(targ, "agility", buff_duration, dex_points, who, 1);

				// INT buff
				var int_val := AP_GetStat(targ, INTELLIGENCE);
				var int_points := CInt((int_val * buff_percent) / 100);
				TS_StartTimer(targ, "intelligence", buff_duration, int_points, who, 1);

				Message("Voce se sente fortalecido pela aura divina!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
				Message("Seu alvo foi abencoado com forca divina!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			KAI:
				var targ := Target(who);
				if (!targ)
					Message("Voce deve selecionar um alvo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif

				PlayObjectCenteredEffect(targ, 0x373A, 10, 20);
				PlaySoundEffect(targ, 0x1F7);

				var attack_bonus := CInt(mobile.GetSkill(THEOLOGY) / 2);
				SetObjProperty(targ, "#hit_mod", attack_bonus);

				Message("Sua precisao foi aprimorada pela aura divina!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
				Message("Voce concede precisao divina ao alvo!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
		endcase

		return true;
	endfunction

endclass
