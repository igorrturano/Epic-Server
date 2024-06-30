include "include/say";
include ":attributes:attributes";
include ":itemUtils:itemdesc";
include ":gumps:yesNo";
include ":traps:traps";
include ":charactercreation:habilidades";


function examinarAlvo(who)
	SendSysMessageEx(who, "Selecione um alvo.", SSM_REQUEST);
	var targ := Target(who);
	if( !targ )
		SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
		return 0;
	elseif ( !targ.isA(POLCLASS_MOBILE) && !targ.isA(POLCLASS_CONTAINER) && !targ.isA(POLCLASS_DOOR) )
		SendSysMessageEx(who, "Alvo invalido.", SSM_FAIL);
		return 0;
	endif

	var armadilhaRevelada := 0;
	var acharamArmadilha := array;
	var found := 0;
	if(targ.isA(POLCLASS_MOBILE))
		if (GetObjProperty(targ, "disfarce") )
			if (SkillCheck(who, AWARENESS, AP_GetSkill(who, SNEAK)))
				found:=1;
				var char := GetObjProperty(targ, "chardata");
				var name := char.FirstName;
				var last_name := char.LastName;
				if (last_name)
					name := name+" "+last_name;
				endif
				sleep(1);
				SendSysMessageEx(who, "Voce descobriu disfarce de "+name+" em "+targ.name+".", SSM_INFO);
				SendSysMessageEx(who, "ATENCAO: Se voce nao conhece esse personagem, ignore o fato de saber o nome dele.", SSM_INFO);
				return 1;
			endif
			return 0;
		endif
	elseif ( GetObjProperty(targ, "TrapList") )
		sleep(1);
		if(!ReserveItem(targ)) //Apenas um personagem examina o bau por vez.
			SendSysMessageEx(who, "Alguem ja esta examinando "+targ.name+".", SSM_FAIL);
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
		if ((!armadilhaRevelada) && (who.serial in acharamArmadilha)) //Quando você achou ela mas não revelou pra todos
			SendSysMessageEx(who, "Armadilha ja foi encontrada e esta obvia para voce.", SSM_INFO);
			if (YesNo(who, "Revelar a armadilha para todos? (", "Sim.", "Nao, deixa so pra mim."))
				who.hidden := 0;
				PrintTextAbove(who, "*Aponta para armadilha em "+targ.name+"*");
				PrintTextAbove(targ, "*!*");
				TrapFound(targ, who);
			endif
			return 1;
		elseif(armadilhaRevelada)
			SendSysMessageEx(who, "Armadilha ja foi encontrada e esta obvia para todos.", SSM_INFO);
			return 1;
		else
			var diff;
			var lvl := Cint(GetObjProperty(targ, "level"));
			case (lvl)
				1: diff := 15;
				2: diff := 40;
				3: diff := 60;
				4: diff := 86;
				5: diff := 90;
				6: diff := 110;
				default: diff := 40;
			endcase
			sleep(1);
			if ( SkillCheck(who, AWARENESS, diff) > 0 )
				acharamArmadilha.Append(who.serial);
				SetObjProperty(targ, "acharamArmadilha", acharamArmadilha);
				SendSysMessageEx(who, "Armadilha encontrada em "+targ.name+".", SSM_INFO);
				if (YesNo(who, "Revelar a armadilha para todos? (", "Sim.", "Nao, deixa so pra mim."))
					who.hidden := 0;
					PrintTextAbove(who, "*Aponta para armadilha em "+targ.name+"*");
					PrintTextAbove(targ, "*!*");
					TrapFound(targ, who);
				endif
				return 1;
			else
				SendSysMessageEx(who, "Voce nao encontrou nada de diferente.", SSM_FAIL);
				return 0;
			endif
		endif
	else
		SendSysMessageEx(who, "Voce nao encontrou nada de diferente.", SSM_INFO);
		return 0;
	endif
	return 0;
endfunction

function CalculateDHDiff(who, mobile)

	var mobile_skill := CInt(AP_GetSkill(mobile, SNEAK));

	//sendsysmessage(mobile, "dificuldade de  "  + who.name + "te achar " + mobile_skill);
	if (GetObjProperty(mobile, "#overridehiding") != error)
		mobile_skill := cint(GetObjProperty(mobile, "#overridehiding"));
	endif
	var dist := Distance(who, mobile);
	var difficulty;

	if (TemHabilidade(mobile, "Furtividade Aprimorada") && !TemHabilidade(who, "Sempre Alerta"))
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

	return difficulty;

endfunction

function examinarArea(who)
	var range := CInt( AP_GetSkill(who, AWARENESS) / 10 ) + 1;

	if ( TemHabilidade(who, "Sempre Alerta") )
		range := range + 5;
	endif

/*	var raca := GetObjProperty(who, "raca");
	if ( raca == "elfo" )
		range := range + 10;
	elseif ( raca == "meio-elfo" || raca == "drow" )
		range := range + 5;
	endif
*/

	var diff;
	var lvl;
	var found := 0;
	var difficulty;
	foreach mobile in ListMobilesNearLocationEX(who.x, who.y, who.z, range, LISTEX_FLAG_HIDDEN, who.realm)
		if ( CheckLineOfSight(who, mobile) && abs(mobile.z - who.z ) < 19 )
			difficulty := CalculateDHDiff(who, mobile);
			if ( mobile == who ); // Do Nothing
			elseif (who.npctemplate) // NPC revela quem ele achar
				if ( SkillCheck(who, AWARENESS, difficulty) > 0 )
					found := found+1;
					mobile.hidden := 0;
					PrintText(mobile, "*Revelado por "+who.name+"*");
				endif
	        elseif (who.party && (who.party.leader.name == mobile.party.leader.name)) //procura o colega, mas não revela
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0, ADV_DISABLE) > 0 )  //Não upa
		        	exibeCharEscondido(who,mobile,258);
		        	PrintTextAbovePrivate(mobile, "!", who,_DEFAULT_TEXT_FONT,258); //Se esta na mesma party nao revela
	        	endif
	        elseif (!mobile.npctemplate && CInt(GetObjProperty(who, "IgnoreHidden"))) //mostra PJ escondidos sem revelar
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0,ADV_DISABLE) > 0 )  //Não upa
		        	exibeCharEscondido(who,mobile,36);
					PrintTextAbovePrivate(mobile, "!", who,_DEFAULT_TEXT_FONT,36);
				endif
	        elseif (mobile.npctemplate && CInt(GetObjProperty(who, "IgnoreHidden"))) //mostra NPC escondidos sem revelar
	        	if ( SkillCheck(who, AWARENESS, difficulty, 0,ADV_DISABLE) > 0 )  //Não upa
		        	exibeCharEscondido(who,mobile,37);
					PrintTextAbovePrivate(mobile, "!", who,_DEFAULT_TEXT_FONT,37);
				endif
			else //Revela todo mundo que deveria
				if ( SkillCheck(who, AWARENESS, difficulty) > 0 )
					found := found+1;
					mobile.hidden := 0;
					PrintText(mobile, "*Revelado por "+who.name+"*");
					SendSysMessageEx(who, "Voce encontrou "+mobile.name+".", SSM_INFO);
					SendSysMessageEx(mobile, who.name+" te encontrou!", SSM_FAIL);
				endif
			endif
			sleepms(1);
		endif
	endforeach

	foreach item in ListItemsNearLocation(who.x, who.y, who.z, range, who.realm)
		if ( item.invisible )
			if ( item.isTrap())
				lvl := Cint(GetObjProperty(item, "level"));
				case (lvl)
					1: diff := 15;
					2: diff := 40;
					3: diff := 60;
					4: diff := 86;
					5: diff := 90;
					6: diff := 110;
					default: diff := -1; //testa contra a propria skill
				endcase
				if ( SkillCheck(who, AWARENESS, diff) > 0 )
					found := found+1;
					item.invisible := 0;
					SendSysMessageEx(who, "Voce encontrou "+item.desc+".", SSM_INFO);
					PrintTextAbovePrivate(item, "*!*", who);
				endif
			else
				var diff := CInt(GetObjProperty(item, "FindDiff"));
				if ( diff )
					if ( SkillCheck(who, AWARENESS, diff) > 0 )
						found := found+1;
						item.invisible := 0;
						SendSysMessageEx(who, "Voce encontrou "+item.desc+".", SSM_INFO);
						PrintTextAbovePrivate(item, "*!*", who);
					endif
				endif
			endif
		endif
		sleepms(2);
	endforeach
	return found;
endfunction

function exibeCharEscondido(who, escondido, cor)
	DeleteObject(who, escondido);
	DrawObject(who, escondido, escondido.graphic, cor);
endfunction

// the following functions have been pasted from SendPacket.inc

function ConvertIntToHex(astring, alength)
	astring := Hex(astring);
	astring := CStr(astring);
	if (astring["0x"])
		astring := astring[3, len(astring)];
	endif
	if (len(astring) > alength * 2)
		return;
	endif
	while (len(astring) < alength * 2)
		astring := "0"+ astring;
	endwhile
	return astring;
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
