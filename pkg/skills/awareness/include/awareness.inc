include "include/say";
include ":attributes:attributes";
include ":itemUtils:itemdesc";
include ":gumps:yesNo";
include ":traps:traps";
include ":charactercreation:habilidades";
include ":combat:Controller/HitController";


function ExaminarMobile(who,targ)
	if (GetObjProperty(targ, "disfarce") )
		if (SkillCheck(who, AWARENESS, EffectiveSkill(who, "Stealth")))
			var char := GetObjProperty(targ, "chardata");
			var name := char.FirstName;
			var last_name := char.LastName;
			if (last_name)
				name := name+" "+last_name;
			endif
			sleep(1);
			SendSysMessageEx(who, "Você descobriu disfarce de "+name+" em "+targ.name+".", SSM_INFO);
			SendSysMessageEx(who, "ATENCAO: Se você não conhece esse personagem, ignore o fato de saber o nome dele.", SSM_INFO);
			return 1;
		endif
	endif
	SendSysMessageEx(who, "Você não encontrou nada de diferente.", SSM_FAIL);
	return 0;
endfunction

function GetTrapTypesDescription(trap_list)
	var description := "";
	foreach trap in trap_list
		var info := GetTrapInfo(trap.name);
		var label;
		if (info.errortext)
			label := trap.name;
		else
			label := info.Name;
		endif
		if (description == "")
			description := label;
		else
			description := description + ", " + label;
		endif
	endforeach
	return description;
endfunction

function ExaminarObjeto(who,targ)

	var armadilhaRevelada := 0;
	var acharamArmadilha := array;
	var trap_list := GetObjProperty(targ, "TrapList");
	if ( trap_list )
		sleep(1);
		if(!ReserveItem(targ)) //Apenas um personagem examina o bau por vez.
			SendSysMessageEx(who, "Alguém já esta examinando "+targ.name+".", SSM_FAIL);
			return 0;
		else
			if(who.hidden)
				SendSysMessageEx(who, "Examinando "+targ.name+".", SSM_INFO);
			else
				PrintTextAbove(who, "*Examinando "+targ.name+"*");
			endif
			sleep(1);
		endif
		if(GetObjProperty(targ, "trapSpoted"))
			armadilhaRevelada := GetObjProperty(targ, "trapSpoted"); //CPROP que diz quem revelou a trap pra todos
		endif
		if(GetObjProperty(targ, "acharamArmadilha"))
			acharamArmadilha := GetObjProperty(targ, "acharamArmadilha"); //Lista de pessoas que revelaram a armadilha só para si
		endif
		var trap_types_desc := GetTrapTypesDescription(trap_list);
		if ((!armadilhaRevelada) && (who.serial in acharamArmadilha)) //Quando você achou ela mas não revelou pra todos
			SendSysMessageEx(who, "Armadilha já foi encontrada e está óbvia para você. Tipo: "+trap_types_desc+".", SSM_INFO);
			if (YesNo(who, "Revelar a armadilha para todos? (", "Sim.", "Não, deixar só para mim."))
				who.hidden := 0;
				PrintTextAbove(who, "*Aponta para armadilha em "+targ.name+"*");
				PrintTextAbove(targ, "*!*");
				TrapFound(targ, who);
			endif
			return 1;
		elseif(armadilhaRevelada)
			SendSysMessageEx(who, "Armadilha já foi encontrada e está óbvia para todos. Tipo: "+trap_types_desc+".", SSM_INFO);
			return 1;
	else
		var diff;
		var lvl := Cint(GetObjProperty(targ, "level"));

		case (lvl)
			1: diff := 20;
			2: diff := 40;
			3: diff := 60;
			4: diff := 80;
			5: diff := 100;
			6: diff := 120;
			default: diff := 1000; // Telepads e afins
		endcase

		sleep(1);
		var check_result := SkillCheck(who, AWARENESS, diff);
		if ( check_result > 0 )
				acharamArmadilha.Append(who.serial);
				SetObjProperty(targ, "acharamArmadilha", acharamArmadilha);

				var trap_count := trap_list.size();

				if (trap_count > 1)
					SendSysMessageEx(who, "Armadilha encontrada em "+targ.name+". Você detectou "+trap_count+" armadilhas: "+trap_types_desc+".", SSM_INFO);
				else
					SendSysMessageEx(who, "Armadilha encontrada em "+targ.name+". Tipo: "+trap_types_desc+".", SSM_INFO);
				endif

				if (YesNo(who, "Revelar a armadilha para todos? (", "Sim.", "Não, deixar só para mim."))
					who.hidden := 0;
					PrintTextAbove(who, "*Aponta para armadilha em "+targ.name+"*");
					PrintTextAbove(targ, "*!*");
					TrapFound(targ, who);
				endif
				return 1;
			else
				SendSysMessageEx(who, "Você nao encontrou nada de diferente.", SSM_FAIL);
				return 0;
			endif
		endif
	else
		SendSysMessageEx(who, "Você não encontrou nada de incomum em "+targ.name+".", SSM_FAIL);
		return 0;
	endif
endfunction

function CalculateDHDiff(who, mobile)

	var mobile_skill := CInt(EffectiveSkill(mobile, "Stealth"));

	//sendsysmessage(mobile, "dificuldade de  "  + who.name + "te achar " + mobile_skill);
	if (GetObjProperty(mobile, "#overridehiding") != error)
		mobile_skill := cint(GetObjProperty(mobile, "#overridehiding"));
	endif
	var dist := Distance(who, mobile);
	var difficulty;

	if (MobileWrapper(mobile).HasAbility(ABILITY_ID::SMOKE_SCREEN) && !MobileWrapper(who).HasAbility(ABILITY_ID::SEMPRE_ALERTA))
		difficulty := CInt(mobile_skill);
	else
		if (dist >=8)
			difficulty := CInt(mobile_skill);
		elseif (dist >= 7)
			difficulty := CInt(mobile_skill*0.95);
		elseif (dist >= 6)
			difficulty := CInt(mobile_skill*0.9);
		elseif ( dist >= 5 )
			difficulty := CInt(mobile_skill*0.85);
		elseif ( dist >= 4 )
			difficulty := CInt(mobile_skill * 0.8);
		elseif ( dist >= 3 )
			difficulty := CInt(mobile_skill * 0.7);
		elseif ( dist >= 2 )
			difficulty := CInt(mobile_skill * 0.6);
		elseif ( dist >= 1 )
			difficulty := CInt(mobile_skill * 0.5);
		else
			difficulty := CInt(mobile_skill * 0.25);
		endif
	endif

	var diffBeforeLight := difficulty;
	var dhLightLabel := "";
	if (!MobileWrapper(who).HasAbility(ABILITY_ID::SEMPRE_ALERTA) && InDarkness(mobile))
		difficulty := CInt(difficulty * 1.10);
		dhLightLabel := "(dark)";
	endif
	if (HasLightNearby(mobile))
		difficulty := CInt(difficulty * 0.85);
		dhLightLabel := dhLightLabel + "(light)";
	endif
	if (GetObjProperty(mobile, "#stealthExausto"))
		difficulty := difficulty - 15;
		dhLightLabel := dhLightLabel + "(exausto)";
	endif
	if (dhLightLabel != "")
		PrintTextAbove(who, "[DH] " + mobile.name + " " + diffBeforeLight + "->" + difficulty + " " + dhLightLabel);
	endif

	return difficulty;

endfunction

function ExaminarArea(who)

	var range := CInt( EffectiveSkill(who, "Perception") / 10 ) + 1;

	if ( MobileWrapper(who).HasAbility(ABILITY_ID::SEMPRE_ALERTA) )
		range := range + 5;
	endif

	var diff;
	var lvl;
	var found := 0;
	var difficulty;
	var itemdescCfg := ReadConfigFile(":*:itemdesc");
	foreach mobile in ListMobilesNearLocationEX(who.x, who.y, who.z, range, LISTEX_FLAG_HIDDEN, who.realm)
		if ( CheckLineOfSight(who, mobile) && abs(mobile.z - who.z ) < 19 )
			difficulty := CalculateDHDiff(who, mobile);
			if ( mobile == who ); // Do Nothing
			elseif (who.npctemplate) // NPC revela quem ele achar
				if ( SkillCheck(who, AWARENESS, difficulty) > 0 )
					found := found+1;
					mobile.hidden := 0;
					PrintTextAbove(mobile, "*Revelado por "+who.name+"*");
				endif
	        elseif (who.party && (who.party.leader.name == mobile.party.leader.name)) //procura o colega, mas não revela
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0, ADV_DISABLE) > 0 )  //Não upa
		        	exibeCharEscondido(who,mobile,258);
		        	PrintTextPrivate(mobile, "!", who,_DEFAULT_TEXT_FONT,258); //Se esta na mesma party nao revela
	        	endif
	        elseif (!mobile.npctemplate && CInt(GetObjProperty(who, "IgnoreHidden"))) //mostra PJ escondidos sem revelar
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0,ADV_DISABLE) > 0 )  //Não upa
		        	exibeCharEscondido(who,mobile,36);
					PrintTextPrivate(mobile, "!", who,_DEFAULT_TEXT_FONT,36);
				endif
	        elseif (mobile.npctemplate && CInt(GetObjProperty(who, "IgnoreHidden"))) //modo passivo: NPC hostil sempre revela
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0,ADV_DISABLE) > 0 )  //Não upa
		        	found := found+1;
					mobile.hidden := 0;
					PrintTextAbove(mobile, "*Revelado por "+who.name+"*");
				endif
			else //Revela todo mundo que deveria
				if ( SkillCheck(who, AWARENESS, difficulty) > 0 )
					found := found+1;
					mobile.hidden := 0;
					PrintTextAbove(mobile, "*Revelado por "+who.name+"*");
					SendSysMessageEx(who, "Você encontrou "+mobile.name+".", SSM_INFO);
					SendSysMessageEx(mobile, who.name+" te encontrou!", SSM_FAIL);
				endif
			endif
			sleepms(1);
		endif
	endforeach

	foreach item in ListItemsNearLocation(who.x, who.y, who.z, range, who.realm)
		// Ignore invisible items without a level (ambience/system items that should never be revealed)
		lvl := Cint(GetObjProperty(item, "level"));
		if (item.invisible && lvl == error)
			continue;
		endif

		// Process discoverable hidden items (invisible with level)
		if ( item.invisible && lvl != error)
			// Items permanently invisible by itemdesc should never be revealed
			var elemDesc := FindConfigElem(itemdescCfg, item.objtype);
			if (elemDesc.Invisible)
				continue;
			endif
			case (lvl)
				1: diff := 20;
				2: diff := 40;
				3: diff := 60;
				4: diff := 80;
				5: diff := 100;
				6: diff := 120;
				default: diff := -1; //testa contra a propria skill
			endcase
			if ( SkillCheck(who, AWARENESS, diff) > 0 )
				found := found+1;
				item.invisible := 0;
				SendSysMessageEx(who, "Voce encontrou "+item.desc+".", SSM_INFO);
				PrintTextPrivate(item, "*!*", who);
			endif
		endif

		sleepms(2);
	endforeach

	// Check for secret doors
	var secret_doors := CheckForSecretDoors(who);
	found := found + secret_doors;

	return found;
endfunction

function CheckForSecretDoors(who)
	// Load secret doors config
	var secretDoorCfg := ReadConfigFile(":megacliloc:config/secretDoors");
	if (!secretDoorCfg)
		return 0;
	endif

	var awareness_skill := CInt(EffectiveSkill(who, "Perception"));
	var range := CInt(awareness_skill / 10) + 1;

	if (MobileWrapper(who).HasAbility(ABILITY_ID::SEMPRE_ALERTA))
		range := range + 5;
	endif

	// Reduce range for secret doors (harder to spot)
	var search_range := CInt(range / 2);
	if (search_range < 1)
		search_range := 1;
	endif

	var doors_found := 0;
	var nearby_items := ListItemsNearLocation(who.x, who.y, who.z, search_range, who.realm);

	foreach item in nearby_items
		// Check if this item is a secret door
		if (FindConfigElem(secretDoorCfg, item.objtype))
			// Roll against skill to notice it
			if (RandomInt(100) < awareness_skill)
				doors_found := doors_found + 1;
				Message("*!*").SendAbove(item);
				Message("Voce notou algo estranho...").SetColor(SSM_INFO).SendSys(who);
				sleepms(50);
			endif
		endif
	endforeach

	return doors_found;
endfunction

function exibeCharEscondido(who, escondido, cor)
	DeleteObject(who, escondido);
	DrawObject(who, escondido, escondido.graphic, cor);
endfunction

// the following functions have been pasted from SendPacket.inc

function ConvertIntToHex(astring, alength)
    if (astring < 0)
        astring := (1 << (alength * 8)) + astring;  // Convert to two's complement for negative values
    endif

    astring := Hex(astring);
    astring := CStr(astring);
    if (astring["0x"])
        astring := astring[3, len(astring)];
    endif
    if (len(astring) > alength * 2)
        return "";
    endif
    while (len(astring) < alength * 2)
        astring := "0" + astring;
    endwhile
    return astring;
endfunction

function AtivaDesativaPassivo(who)
	var ignore := GetObjProperty(who, "IgnoreHidden");
	if (ignore)
		SendSysMessageEx(who, "Modo Passivo Ligado. Procura e revela passivamente!", SSM_INFO);
		EraseObjProperty(who, "IgnoreHidden");
	else
		SendSysMessageEx(who, "Modo Passivo Desligado. Percebe, mas não revela passivamente!", SSM_FAIL);
		SetObjProperty(who, "IgnoreHidden", 1);
	endif
endfunction

function VerificarArtefato(who, item)
	var prop_stealable := CInt(GetObjProperty(item, "PropStealable"));
	var skill          := CInt(EffectiveSkill(who, "Perception"));

	if (prop_stealable)
		SendSysMessage(who, "Seus olhos percorrem o objeto... parece ter sido deixado aqui de propósito, à espera de mãos habilidosas.", 3, 44);

		var trap_list := GetObjProperty(item, "TrapList");
		if (trap_list && trap_list.size() > 0)
			var item_level := CInt(GetObjProperty(item, "level"));
			SendSysMessage(who, GetTrapRPPhrase(trap_list[1].name, item_level, skill), 3, 44);
		endif
	else
		SendSysMessage(who, "Você examina o objeto com atenção, mas nada de incomum chama sua atenção.", 3, 44);
	endif
endfunction

function GetTrapRPPhrase(trap_name, level, skill)
	// skill é armazenado ×10 por AP_GetSkill (skill 30.0 = raw 300)
	if (skill < 300)
		return "Uma sensação inquietante emana do objeto. Algo não está certo, mas você não consegue precisar o quê.";
	endif

	var type_phrase;
	case (trap_name)
		"Needle":      type_phrase := "pequenos orifícios quase imperceptíveis na superfície";
		"PoisonGas":   type_phrase := "um leve odor acre e adocicado que emana do interior";
		"Explosion":   type_phrase := "marcas de enxofre e um mecanismo tenso na base";
		"Magic":       type_phrase := "uma aura perturbadora e fora do natural ao redor do objeto";
		"SteveDonald": type_phrase := "uma energia sombria e desgastante que emana do objeto";
		default:       type_phrase := "sinais sutis de um mecanismo oculto";
	endcase

	if (skill < 600)
		return "Você nota " + type_phrase + ". O objeto parece conter alguma proteção.";
	endif

	var intensity_phrase;
	if (level <= 2)
		intensity_phrase := "modesta";
	elseif (level <= 4)
		intensity_phrase := "considerável";
	else
		intensity_phrase := "altamente perigosa";
	endif

	if (skill < 800)
		return "Você nota " + type_phrase + ". A ameaça parece ser " + intensity_phrase + ".";
	endif

	var precise_trap;
	case (trap_name)
		"Needle":      precise_trap := "agulhas envenenadas";
		"PoisonGas":   precise_trap := "gás tóxico";
		"Explosion":   precise_trap := "carga explosiva";
		"Magic":       precise_trap := "encantamento protetor de origem arcana";
		"SteveDonald": precise_trap := "feitiço de envelhecimento";
		default:       precise_trap := "um mecanismo de natureza desconhecida";
	endcase

	return "Com olhos treinados, você decifra a proteção: " + precise_trap + " de intensidade " + intensity_phrase + ". Proceda com cautela.";
endfunction

function DrawObject(ktowhom, kobj, kgraphic, kcolor)
	var PacketHexStringPart1 := "78";
	var PacketHexStringPart2 := ConvertIntToHex(kobj.serial, 4) + ConvertIntToHex(kgraphic, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kobj.x, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kobj.y, 2);
	PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kobj.z, 1);
	PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kobj.facing, 1);
	PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kcolor, 2);
	PacketHexStringPart2 := PacketHexStringPart2 +"0000";
	foreach item in ListEquippedItems(kobj)
		PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(item.serial, 4);
		PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(item.graphic, 2);
		PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(item.layer, 1);
		PacketHexStringPart2 := PacketHexStringPart2 + ConvertIntToHex(kcolor, 2);
	endforeach
	PacketHexStringPart2 := PacketHexStringPart2 +"00000000";
	var FullPacketHexStringLength := (len(PacketHexStringPart1) + len(PacketHexStringPart2)) / 2 + 2;
	var FullPacketHexString := PacketHexStringPart1 + ConvertIntToHex(FullPacketHexStringLength, 2) + PacketHexStringPart2;
	SendPacket(ktowhom, FullPacketHexString);
endfunction

function DeleteObject(who, lobj)
	var PacketHexString := "1D"+ ConvertIntToHex(lobj.serial, 4);
	SendPacket(who, PacketHexString);
endfunction
