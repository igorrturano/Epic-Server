include ":combat:settings";
include ":combat:Model/AttackInfo";
include ":combat:Model/CombatAbility";

include ":wrappers:Objects/Model/MobileWrapper";
// include ":wrappers:Objects/Model/WeaponWrapper";
include ":debug:Controller/DebugController";

include ":attributes:attributes";

function ProgressAttackSkills(byRef attackInfo)
	var skillUsedForCritical := TACTICS;

	if (!attackInfo.GetAttacker().IsConnected()) // Se o atacante estiver offline, não upa
		return false;
	endif

	// Testa se é possivel ganhar skill com o NPC alvo
	// var skillAttacker := AP_GetSkill(attacker, attacker.weapon.attribute);
	// var skillDefender := AP_GetSkill(defender, defender.weapon.attribute);
	var skillAttacker := attackInfo.GetAttacker().GetSkill(attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute());
	var skillDefender := attackInfo.GetDefender().GetSkill(attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetAttribute()); // TODO: Verificar se isso funciona com NPCs sem arma equipada (se pega WRESTLING)

	if (attackInfo.GetDefender().IsNPC() && attackInfo.GetAttacker().IsConnected())
		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType() == 0x13b9 && !attackInfo.GetAttacker().GetEquipBySlot(SHIELD)) // bastarda
			skillAttacker := attackInfo.GetAttacker().GetSkill(TWOHANDED);
		endif

		if (attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetObjType() == 0x13b9 && !attackInfo.GetDefender().GetEquipBySlot(SHIELD))
			skillDefender := attackInfo.GetDefender().GetSkill(TWOHANDED);
		endif

		if (attackInfo.GetAttacker().HasAbility(ABILITY_NAME::MAESTRIA_DE_DEVASTACAO) &&
			attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjRef().GetProp("swordmage") == attackInfo.GetAttacker().GetSerial())
			
			skillAttacker := attackInfo.GetAttacker().GetSkill(ONEHANDED);
		endif

		if (attackInfo.GetDefender().HasAbility(ABILITY_NAME::MAESTRIA_DE_DEVASTACAO) &&
			attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetObjRef().GetProp("swordmage") == attackInfo.GetDefender().GetSerial())
			
			skillDefender := attackInfo.GetDefender().GetSkill(ONEHANDED);
		endif
	endif

	// se a skill do atacante for maior 10 pontos a mais que a do defesor, não upa
	if (skillAttacker > (skillDefender + 10))
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_UP, Message("Skill UP bloqueado: diferença de skill maior que 10 pontos."));
		return 0;
	endif

	// Sendo o alvo um personagem jogador ou NPC com combate melhor que o atacante, permite o up de skill de combate
	var can_gain := 0;
	var next_gain := CInt(attackInfo.GetAttacker().GetObjRef().GetProp("#CS_GainTime"));
	if (ReadGameClock() >= next_gain)
		if (CS_GetSettingsCfgElem("General").PvPGains)
			can_gain := 1;
		elseif (attackInfo.GetDefender().IsNPC())
			can_gain := 1;
		endif
	endif

	if (can_gain)
		var gain_wait := CInt(CS_GetSettingsCfgElem("General").GainWait);
		attackInfo.GetAttacker().GetObjRef().SetProp("#CS_GainTime", ReadGameClock() + gain_wait);

		if (attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetObjType() == 0x13b9)
			if (!attackInfo.GetAttacker().GetEquipBySlot(SHIELD))
				SkillCheck(attackInfo.GetAttacker().GetObjRef(), TWOHANDED, -1);
			else
				SkillCheck(attackInfo.GetAttacker().GetObjRef(), ONEHANDED, -1);
			endif
		else
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), attackInfo.GetAttacker().GetEquipBySlot(WEAPON).GetAttribute(), -1);
		endif

		SleepMS(5);
		SkillCheck(attackInfo.GetAttacker().GetObjRef(), skillUsedForCritical, -1);
	endif
endfunction
