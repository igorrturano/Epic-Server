include ":combat:settings";
include ":combat:Model/AttackInfo";
include ":abilities:models/Ability";

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
	var skillAttacker := attackInfo.GetAttacker().GetSkillBase(attackInfo.GetWeapon().GetAttribute());
	var skillDefender;
	if (attackInfo.GetDefender().IsNPC())
		skillDefender := attackInfo.GetDefender().GetSkillBase(WRESTLING);
	else
		skillDefender := attackInfo.GetDefender().GetSkillBase(attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetAttribute());
	endif

	if (attackInfo.GetDefender().IsNPC() && attackInfo.GetAttacker().IsConnected())
		if (attackInfo.GetWeapon().GetObjType() == 0x13b9 && !attackInfo.GetAttacker().GetEquipBySlot(SHIELD)) // bastarda
			skillAttacker := attackInfo.GetAttacker().GetSkillBase(TWOHANDED);
		endif

		if (attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetObjType() == 0x13b9 && !attackInfo.GetDefender().GetEquipBySlot(SHIELD))
			skillDefender := attackInfo.GetDefender().GetSkillBase(TWOHANDED);
		endif

		if (attackInfo.GetWeapon().GetObjRef().GetProp("swordmage") == attackInfo.GetAttacker().GetSerial())
			skillAttacker := attackInfo.GetAttacker().GetSkillBase(ONEHANDED);
		endif

		if (attackInfo.GetDefender().GetEquipBySlot(WEAPON).GetObjRef().GetProp("swordmage") == attackInfo.GetDefender().GetSerial())
			skillDefender := attackInfo.GetDefender().GetSkillBase(ONEHANDED);
		endif
	endif

	// se a skill do atacante for maior 10 pontos a mais que a do defesor, não upa
	if (skillAttacker > (skillDefender + 10))
		DebugMsg(attackInfo.GetAttacker().GetObjRef(), DEBUG_UP, Message("Skill UP bloqueado: Voce tem 10 de skill a mais que seu oponente"));
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
		var attackerWeapon := attackInfo.GetWeapon();
		var shouldGainArcana := attackerWeapon.GetCategory() == WEAPON_CATEGORY::MAGICBOW
			|| attackerWeapon.GetCategory() == WEAPON_CATEGORY::SPIRITUALBLADE;

		var weaponSkill;
		if (attackInfo.GetWeapon().GetObjType() == 0x13b9)
			if (!attackInfo.GetAttacker().GetEquipBySlot(SHIELD))
				weaponSkill := TWOHANDED;
			else
				weaponSkill := ONEHANDED;
			endif
		else
			weaponSkill := attackInfo.GetWeapon().GetAttribute();
		endif

		// Randomize order so weapon skill and tactics level up ~50/50
		if (RandomInt(2) == 0)
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), weaponSkill, -1);
			SleepMS(5);
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), skillUsedForCritical, -1);
		else
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), skillUsedForCritical, -1);
			SleepMS(5);
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), weaponSkill, -1);
		endif

		if (shouldGainArcana)
			SkillCheck(attackInfo.GetAttacker().GetObjRef(), ARCANA, -1);
		endif
	endif
endfunction
