use cfgfile;

include ":commands:debugsystem";
include ":attributes:attributes";
include "include/epicCombatUtils";
include "include/facings";
include ":timedScripts:timedScripts";

function CalculateTotalBaseDamage(attacker, defender, rawdamage)
	var damage := 0;

	// Aplica bonus de dano
	damage := CalcDamageStats(attacker, damage);
	damage := CalcDamageWeapon(attacker, damage);
	damage := CalcDamageSpecial(attacker, defender, damage);
	damage := CalcDamageHabs(attacker, defender, damage);
	
	// Aplica redutores e penalidades de dano
	damage := CalcDamageReducer(attacker, damage);

	if (damage < 1)
		damage := 1;
	endif

	damage := CInt(damage);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Base: " + damage);

	return damage;
endfunction

function CalcDamageStats(attacker, damage)
	var stat;

	var cfg := ReadConfigFile(":combat:itemdesc");
	var elem := FindConfigElem(cfg, attacker.weapon.objtype);

	SkillCheck(attacker, TACTICS, -1);
	var tacticsmod := CInt(AP_GetSkill(attacker, TACTICS)/20);
	damage += tacticsmod;

	stat := STRENGTH;
	if (elem.Attribute == "Ranged")
		stat := DEXTERITY;
	endif

	var statmod;
	if (elem.Throwable)
		statmod := CInt(AP_GetStat(attacker, DEXTERITY)) + CInt(AP_GetStat(attacker, STRENGTH) / 2);
	else
		statmod := CInt(AP_GetStat(attacker, stat));
	endif

	damage += (statmod-50) / 10;
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de "+stat+": " + damage);

	return damage;
endfunction

function CalcDamageWeapon(attacker, damage)
	var weaponcfg := ReadConfigFile(":*:itemdesc");
	var weaponelem := FindConfigElem(weaponcfg, attacker.weapon.objtype);
	
	var weapondice := CStr(weaponelem.Damage);
	var damageweapon;
	if (CheckBastardSword(attacker)) // Usando espada bastarda com as duas mãos
		weapondice := "1d10";
	endif
	damageweapon := RandomDiceRoll(weapondice);
	damage += damageweapon;

	damage += Cint(GetObjProperty(attacker.weapon, "dmgbonus")) + Cint(GetObjProperty(attacker.weapon, "#dmgbonus"));

	if (GetObjProperty(attacker, "armapredileta") == attacker.weapon.objtype)
		damage += Cint(AP_GetSkill(attacker, TACTICS) / 25);
	endif

	var sharpened_weapon := GetObjProperty(attacker.weapon, "Aprimoramentos")["Arma Afiada"];
	if(sharpened_weapon)
		damage += sharpened_weapon;
	endif

	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano da arma: " + weapondice);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Rolou dado: " + damageweapon);
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano: " + CInt(damage));
	DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Arma Afiada: " + CInt(sharpened_weapon));

	return damage;
endfunction

function CalcDamageHabs(attacker, defender, damage)
	if (GetObjProperty(attacker, "#tiropenetrante")) // TODO: Pensar o que fazer aqui, já que perfura armadura, talvez jogar pra função que aplica o dano das habs, ao invés de entrar no cálculo aqui
		EraseObjProperty(attacker, "#tiropenetrante");
        var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		// DamageFLS(defender, cint(damage-rawdamage*2), DMG_FORCED, attacker);
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Tiro Penetrante]: " + CInt(damagemod));
	endif

	if (TemHabilidade(attacker, "Brigador de Taverna") && attacker.weapon.objtype == 0x1F020)
        var damagemod := 2;
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Brigador de Taverna]: " + CInt(damagemod));
    endif

	if (getCooldown(attacker, "faroporsangue"))
		var bonus := AP_GetVital(defender, HITS) / AP_GetVitalMaximumValue(defender, HITS);
        
		if (bonus <= 0.25)
			bonus := "1d12";
		elseif (bonus <= 0.5)
			bonus := "1d10";
        elseif (bonus <= 0.75)
            bonus := "1d8";
		endif

		var damagemod := RandomDiceRoll(bonus);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Faro por Sangue]: " + CInt(damagemod));
	endif

	if (TemHabilidade(attacker, "Taticas Arcanas"))
		var manaatual := AP_GetVital(attacker, "MANA");
		var manatotal := AP_GetVitalMaximumValue(attacker, "MANA");
		if (manaatual >= manatotal / 2)
			var damagemod := CInt(manaatual / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Faro por Sangue]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(attacker, "lobosolitario"))
		if (ListHostiles(attacker, 3, LH_FLAG_LOS) > 2)
			var damagemod := CInt(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Lobo Solitário]: " + CInt(damagemod));
		endif
	endif

	if (TemHabilidade(attacker, "Precisao Cirurgica"))
		var damagemod := CInt(AP_GetSkill(attacker, ALCHEMY) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Precisão Cirurgica]: " + CInt(damagemod));
	endif

    if (TemHabilidade(attacker, "Arsenal Sombrio"))
        var damagemod := CInt(AP_GetSkill(attacker, SNEAK) / 20);
        if (isAssassinWeapon(attacker))
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
        endif
    endif

	if (TemHabilidade(attacker, "Estilo Mao Livre"))
		if (attacker.weapon.attribute == "One_Handed" && !attacker.shield)
			var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 25);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Estilo Mao Livre]: " + CInt(damagemod));
		endif
	endif

	if (TemHabilidade(attacker, "Ainalaith"))
		var damagemod := 0;
		var arrow_effects := GetObjProperty(attacker, "#shooting_effects");
		if (!arrow_effects)
			// return 0; // No effects recorded, exit early
		endif

		var arrow_types := array{"fire_arrow", "cold_arrow", "energy_arrow"};
		foreach arrow_type in arrow_types
			if (arrow_effects[arrow_type] >= 3)
				case (arrow_type)
					"fire_arrow":
						TS_StartTimer(defender, "queimar", 10);
						break;

					"cold_arrow":
						TS_StartTimer(defender, "lentidao", 10);
						break;

					"energy_arrow":
						damagemod += Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
						break;
				endcase

				arrow_effects[arrow_type] := 0;
				SetObjProperty(attacker, "#shooting_effects", arrow_effects);
			endif
		endforeach

		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Ainalaith]: " + CInt(damagemod));
	endif

	if (TemHabilidade(attacker, "Inimigo Predileto"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(attacker, "InimigoPredileto");

		if (cfg[npctemplate.template].Category == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Inimigo Predileto]: " + CInt(damagemod));
		endif
    elseif (TemHabilidade(attacker, "Exitium Maleficorum"))
        var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := array{"Undead", "Demons"};

		if (cfg[npctemplate.template].Category in inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Exitium Maleficorum]: " + CInt(damagemod));
		endif
	elseif (TemHabilidade(attacker, "Oleo do Cacador"))
		var npctemplate := NPC_ParseTemplateName(defender.npctemplate);
		var cfg := ReadConfigFile(":brainai:npcdesc");
		var inimigo := GetObjProperty(attacker, "Óleo");
		if (cfg[npctemplate.template].Category == inimigo)
			var damagemod := Cint(AP_GetSkill(attacker, ALCHEMY) / 20);
			damage := damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Oleo do Cacador]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(defender, "prey"))
		var damagemod := Cint(AP_GetSkill(attacker, SURVIVAL) / 20);
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Presa]: " + CInt(damagemod));
	endif

	if (temHabilidade(attacker, "Instinto de Sobreviência") && AP_GetVital(attacker, HITS) <= (AP_GetSkill(attacker, MAGICLORE) / 2))
		if (GetObjProperty(attacker, "metamorfose") != error)
			var damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Arsenal Sombrio]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
	endif

	if (IsRaging(attacker))
		var damagemod;
		if (temHabilidade(attacker, "Nazg Khalug") || GetObjProperty(attacker, "NpcFuria"))
			damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Nazg Khalug]: " + CInt(damagemod));
			PrintTextAbove(attacker, "*enfurecido*");
		endif
		if (temHabilidade(attacker, "Furia Primitiva"))
			if (GetObjProperty(attacker, "metamorfose") != error)
				damagemod := Cint(AP_GetSkill(attacker, MAGICLORE) / 20);
				damage += damagemod;
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Primitiva]: " + CInt(damagemod));
			endif
		endif
		if (GetCooldown(attacker, "FuriaPrimordial"))
			var inimigos := ListHostiles(attacker, 3, LH_FLAG_LOS);
			inimigos := inimigos.size();
			var damagemod := Cint(inimigos.size());
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Primordial]: " + CInt(damagemod));
		endif
		if (GetCooldown(attacker, "furiamortal"))
			var damagemod := Cint((AP_GetVitalMaximumValue(attacker, HITS) - AP_GetVital(attacker, HITS)) / 10);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia Mortal]: " + CInt(damagemod));
		endif
	endif

	if (TemHabilidade(attacker, "Budōka"))
		var monk_weapons := array{ 0x0df0, 0x13f9, 0x0e8a, 0x0e81, 0x1F020, 0x27AD, 0x27AE, 0x27A7};
        var isArmor := array{"ArmaduraPesada", "ArmaduraMedia", "ArmaduraLeve"};
		var damagemod := Cint(AP_GetSkill(attacker, TACTICS) / 20);
        var i_config := ReadConfigFile(":*:itemdesc");
        foreach item in ListEquippedItems(attacker)
            var this_item := i_config[item.objtype];
            if (this_item.TipoProtecao in isArmor)
                SendSysMessageEx(attacker, "Você perde mobilidade por estar de armadura e consequentemente, dano.", SSM_INFO);
                damagemod := 0;
                break;
            endif
        endforeach
    	
		if (damagemod)
			if (attacker.weapon.objtype in monk_weapons)
				damage += damagemod;
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Budoka]: " + CInt(damagemod));
			endif
		endif
	endif

	if (isBloodied(defender))
		if (temHabilidade(attacker, "Burz"))
			var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Burz]: " + CInt(damagemod));
		endif
	endif

	if (GetCooldown(attacker, "furia"))
		var damagemod := CInt(AP_GetSkill(attacker, TACTICS) / 20);
		if (TemHabilidade(attacker, "Destruidor"))
			damagemod := damagemod * 2;
		endif
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Furia]: " + CInt(damagemod));
	endif

	if(TemHabilidade(attacker, "Flanquear"))
		if(IsBehind(defender, attacker.x, attacker.y))
			var damagemod := RandomDiceRoll("1d4");
			damage += damagemod;
			DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Flanquear]: " + CInt(damagemod));
		endif
	endif

	if (CInt(GetObjProperty(attacker, "#mataroumorrer")) > 0)
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano de Habilidade[Matar ou Morrer]: " + CInt(damagemod));
	endif

	return damage;
endfunction

function CalcDamageSpecial(attacker, defender, damage)
	if (GetCooldown(attacker.weapon, "blessweapon"))
		var categoriaDefender := NPC_GetNPCConfig(defender.npctemplate).Category;
		var cursed_creatures := {"undead", "demon"};
		if (defender.npctemplate)
			if (lower(categoriaDefender) in cursed_creatures)
				var damagemod := CInt(GetObjProperty(attacker.weapon, "blessedbonus"));
				damage += damagemod;
				DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Especial[Bless Weapon]: " + CInt(damagemod));
			endif
		endif
	endif

	var backStab := CDbl(GetObjProperty(attacker, "#backstab"));
	if (backStab)
		var damagemod := 1;
		damage += damagemod;
		EraseObjProperty(attacker, "#backstab");
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Especial[Backstab]: " + CInt(damagemod));
	endif

	if(GetObjProperty(defender, "apodrecer"))
		var damagemod := RandomDiceRoll("1d4");
		damage += damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Dano Especial[Apodrecer]: " + CInt(damagemod));
	endif

	return damage;
endfunction

function CalcDamageReducer(attacker, damage)
	damage += CInt(GetObjProperty(attacker, "dmgbonus") + GetObjProperty(attacker, "#dmgbonus"));

	var damagemod := CDbl(GetObjProperty(attacker, "dmgreduction"));
	if (damagemod > 0)
		damage := damage*damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Redutor de Dano: {}%".format(CDbl((1-damagemod)*100)));
	endif

	var polida := GetObjProperty(attacker.weapon, "polida");
	if (!polida)
		var penalty := 0.8;
		var damagemod := damage * (1-penalty);
		damage -= damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Penalidade polidez (20%): {}".format(damagemod));
	else
		polida -= 1;
		if (polida < 1)
			EraseObjProperty(attacker.weapon, "polida");
			SendSysMessageEX(attacker, "Sua arma perdeu a polidez.", SSM_INFO);
		else
			SetObjProperty(attacker.weapon, "polida", polida);
		endif
	endif

	if (GetObjProperty(attacker, "chardata").trait == "Defensor Anão")
		var penalty := 0.7;
		var damagemod := damage * (1-penalty);
		damage -= damagemod;
		DebugMsg(attacker, DEBUG_NAME_DAMAGE, "Penalidade Defensor Anão (30%): {}".format(damagemod));
	endif

	return damage;
endfunction





function CheckBastardSword(who)
	if (who.weapon.objtype == 0x13b9 && !who.shield)
		return 1;
	endif
	return 0;
endfunction