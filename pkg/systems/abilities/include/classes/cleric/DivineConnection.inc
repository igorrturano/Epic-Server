include ":abilities:models/CombatAbility";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Conexao Divina (Divine Connection) - ABILITY_ID::CONEXAO_DIVINA (2800)
 *
 * Active (OnUse):
 * - Connects with divine energy, producing religion-specific effects
 * - Scaling: theology_bonus = Theology / 5
 *
 * Religion effects:
 *   KAI: Target gets "vitalidadeinabalavel" cooldown
 *   ELUITHORTH: Activates truesight (start_script)
 *   VENERATIO: Silences target + forced damage (1d4 + INT/10 + Theology/10)
 *   BOKK_BHARAZ: Activates shield barrier (start_script)
 *   HALLEUAD: Tactics buff (day) or Awareness buff (night)
 */
class DivineConnection(CombatAbility)

	function DivineConnection(this)
		Super(ABILITY_ID::CONEXAO_DIVINA);
		this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	endfunction

	function ExecuteOn(this, mobile)
		var who := mobile.GetObjRef();
		var religiao := mobile.GetReligion();
		var theology_bonus := CInt(mobile.GetSkill(THEOLOGY) / 5);
		var diaounoite := GetGlobalProperty("dia");

		Message("*Conecta com energia divina*").SendEmote(who);
		PlayObjectCenteredEffect(who, 0x376A, 1, 32);

		case (religiao)
			KAI:
				var targ := Target(who);
				if (!targ)
					Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				PlayObjectCenteredEffect(targ, 0x37B9, 0x0A, 0x1E);
				PlaySoundEffect(targ, 0x51D);
				SetCooldown(targ, "vitalidadeinabalavel", theology_bonus);
				Message("Voce sente vitalidade inabalavel fluindo por seu corpo!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);

			ELUITHORTH:
				start_script(":charactercreation:habilidades/truesight", who);
				Message("Sua visao verdadeira se ativa!", Color(SYSTEM_COLOR::INFO)).SendSys(who);

			VENERATIO:
				var targ := Target(who);
				if (!targ)
					Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif
				PlaySoundEffect(who, SFX_MAGIC_ARROW);
				PlayMovingEffectXYZHued(who, targ, FX_MAGIC_ARROW, 10, 1643, 0);
				SetCooldown(targ, "silence", theology_bonus);
				Message("Voce foi silenciado pela conexao divina!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);

				var dmg := RandomInt(4) + 1;
				dmg += CInt((mobile.GetInt() - 50) / 10) + CInt(mobile.GetSkill(THEOLOGY) / 10);
				if (dmg < 1)
					dmg := 1;
				endif
				ApplyRawDamage(targ, CInt(dmg));

			BOKK_BHARAZ:
				start_script(":charactercreation:habilidades/barreiradeescudos", who);
				Message("Voce canaliza a protecao divina!", Color(SYSTEM_COLOR::INFO)).SendSys(who);
				Message("*Parede de Escudos*").SendAbove(who);

			HALLEUAD:
				var targ := Target(who);
				if (!targ)
					Message("Cancelado.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
					return false;
				endif

				var skill := TACTICS;
				if (diaounoite == "dia")
					AP_ModifySkillMod(targ, skill, theology_bonus);
					Message("A luz do dia fortalece suas taticas de combate!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
					PlayObjectCenteredEffect(targ, 0x373A, 10, 20);
					PlaySoundEffect(targ, 0x1F5);
					sleep(theology_bonus);
				else
					skill := AWARENESS;
					AP_ModifySkillMod(targ, skill, theology_bonus);
					Message("A escuridao aguca seus sentidos!", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
					PlayObjectCenteredEffect(targ, 0x375A, 10, 20);
					PlaySoundEffect(targ, 0x1F8);
				endif
				sleep(theology_bonus);
				AP_ModifySkillMod(targ, skill, -theology_bonus);
		endcase

		return true;
	endfunction

endclass
