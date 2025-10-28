include ":combat:damage";
include ":IO:CFGFile";

function ApplyCriticalHit(attacker, defender, weapon, armor, basedamage, rawdamage)
	if (getObjProperty(attacker, "critdelay") > ReadGameClock())
		return 0;
	endif

	var weapon_scripts := array{};

	var pet_instinto := GetObjProperty(attacker, "Group_savage"); // animal tamado com a hab Instinto Selvagem para adicionad criticos diferentes
	var pet_critic := GetObjProperty(attacker, "Critic");
	var pet_selvagem := GetObjProperty(attacker, "Selvagem"); // verifica se é animal selvagem - nunca tamado, para dar dano critico

	var npcCfg := CFGFile("combat", "npcdesc").SetElementName(attacker.npctemplate);
	var tamingCfg := CFGFile("taming", "taming").SetElementName(pet_critic);
	var weaponCfg := CFGFile("*", "itemdesc").SetElementName(weapon.objtype);
	
	var pet_txt := tamingCfg.SetKeyName("Text").GetKeyValue();
	var tem_critico := npcCfg.SetElementName("CriticalHitScripts").GetElementValue(); // Se tem critico no npcdesc definido

	if (!attacker.ip && weapon.IsA(POLCLASS_WEAPON) && !pet_instinto && !pet_selvagem)
		if (tem_critico == {})
			weapon_scripts := weaponCfg.SetKeyName("CriticalHitScripts").GetElementValue();
		else
			weapon_scripts := tem_critico;
		endif
	elseif (pet_instinto == 1 || pet_selvagem == 1)
		weapon_scripts := tamingCfg.GetElementValue("CriticalHitScripts");
		Message(pet_txt[1]).SendAbove(attacker);
	else
		weapon_scripts := weaponCfg.SetKeyName("CriticalHitScripts").GetElementValue();
	endif
	// Possivel mudar o critico de npc's. Os npcs só tem uma arma, a Intrinsic Weapon, mesmo que alguma esteja equipada.

	var criticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_CRITICO));
	var reducaoCriticoFabricacao := CInt(PegaValorFabricacao(attacker.weapon, CARACTERISTICA_FABRICACAO_REDUCAOCRITICO));
	var can_crit := criticoFabricacao > 0 || attacker.npctemplate || weapon == FIST;

	if (weapon_scripts.size() > 0 && can_crit)
		var multiplicador := 7;
		reducaoCriticoFabricacao := reducaoCriticoFabricacao == OT_LONG ? reducaoCriticoFabricacao : 0;
		multiplicador -= (criticoFabricacao - reducaoCriticoFabricacao);
		
		var critdelay := RandomIntMinMax(15 * multiplicador, 130 + (15 * multiplicador));
		if (!GetObjProperty(weapon, "polida"))
			Message("Sua arma está embotada!").SetColor(SSM_FAIL).SendSys(attacker);
			critdelay := ReadGameClock() + RandomIntMinMax(240, 450);
		endif

		Message("*CRÍTICO*").SetColor(2118).SendAbove(attacker);
		SetObjProperty(attacker, "critdelay", ReadGameClock() + critdelay);

		var isDwarverDefender := GetObjProperty(defender, "chardata").trait == "Defensor Anão";
		if (GetCooldown(defender, "vitalidadeinabalavel") > 0 || isDwarverDefender)
			Message("Você ignora o crítico!").SetColor(SSM_INFO).SendSys(defender);
			Message("O defensor ignorou o crítico!").SetColor(SSM_FAIL).SendSys(attacker);
			return 1;
		endif
	elseif (!can_crit)
		// Message("Apenas armas elaboradas causam crítico").SetColor(SSM_FAIL).SendSys(attacker);
		return 1;
	endif

	foreach hitscript in weapon_scripts
		var script := start_script(
			hitscript,
			array{ attacker, defender, weapon, armor, basedamage, rawdamage }
		);
		if (!script || script.errortext)
			Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(attacker);
			Message("*Attacker* Weapon script error starting [" + hitscript + "] :" + script.errortext).SendSys(defender);
		endif
	endforeach
endfunction