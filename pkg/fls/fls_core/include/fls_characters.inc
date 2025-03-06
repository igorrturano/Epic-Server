
use uo;
use cfgfile;

include ":brainai:npcUtil";
include ":attributes:attributes";
include ":fls_core:fls_chargumps_ex";
include "include/dungeons";
include "include/epicConstants";
include ":tn:cooldown";
function EquipRaceSkin(who)

	var myrace := GetObjProperty(who, "raca");

	if ( myrace == "humano")
		if (who.gender) // female
			var paperdoll := CreateItemInBackpack(who, 0x3823);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		else //male
			var paperdoll := CreateItemInBackpack(who, 0x3822);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		endif
	elseif ( myrace == "elfo")
		if (who.gender) // female
			var paperdoll := CreateItemInBackpack(who, 0x3827);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		else //male
			var paperdoll := CreateItemInBackpack(who, 0x3826);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		endif
	elseif ( myrace == "hobbit")
		if (who.gender) // female
			var paperdoll := CreateItemInBackpack(who, 0x382D);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		else //male
			var paperdoll := CreateItemInBackpack(who, 0x382C);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		endif
	elseif ( myrace == "anao")
		if (who.gender) // female
			var paperdoll := CreateItemInBackpack(who, 0x3825);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		else //male
			var paperdoll := CreateItemInBackpack(who, 0x3824);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		endif
	elseif ( myrace == "drow")
		if (who.gender) // female
			var paperdoll := CreateItemInBackpack(who, 0x382B);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		else //male
			var paperdoll := CreateItemInBackpack(who, 0x382A);
			paperdoll.newbie := 1;
			paperdoll.color := who.color;
			EquipItem(who, paperdoll);
		endif
	endif
endfunction

function AwardClassSkill(who, diff, chance, classe)

	var myclass := GetObjProperty(who, "classe");
	if (classe != myclass)
		return;
	endif

	CheckSkillAdvance(who, BEGGING, diff, 100 );

endfunction

function IsInDungeon(player)

	if (player.x >= 3749 && player.y >= 0 && player.x <= 6141 && player.y <= 4000)
		return 1;
	endif

	return 0;

endfunction

function HaveReverseVision(who)

	if (GetObjProperty(who, "chardata").raca == DROW)
		return 1;
	elseif (cint(GetObjProperty(who, "reversevision")) == 1)
		return 1;
	elseif (GetObjProperty(who, "metamorfose")) // TODO: Ajuste paliativo pro Open Beta
		return 1;
	else
		return 0;
	endif

endfunction

function ApllyLightLevel()
	foreach player in enumerateonlinecharacters()
		if (HaveReverseVision(player))
			ReverseLight(player);
		endif
	endforeach
endfunction

function ReverseLight(player)
	var config := readconfigfile(":nature:config/daynight");
	config := config["settings"];
	var darkness := cint(config.darkness);

	var light := cint(getglobalproperty("lightlevel"));
	if (player.realm == "britannia" && !GetCooldown(player, "lightvision"))
		player.setlightlevel(20, 9000); // Is in a dungeon
	elseif(player.x >= 4390 && player.y >= 0 && player.x <= 6100 && player.y <= 4095)
		player.setlightlevel(20, 9000); // Is in a dungeon
	else
		var newlight := abs(darkness - light);
		player.setlightlevel(newlight, 9000);
	endif

endfunction

//RETIRADO por estar redundante com methods/char
//function GetClass(who)
//	var classe := GetObjProperty(who, "classe");
//	if (!classe)
//		return 0;
//	else
//		return classe;
//	endif
//endfunction

//function GetRace(who)
//	var raca := GetObjProperty(who, "raca");
//	if (!raca)
//		return 0;
//	else
//		return raca;
//	endif
//endfunction

function GetRaceCfgElem(race_name)
	var cfg := GetRaceCfg();
	return cfg[race_name];
endfunction

function GetRaceCfg()
	var cfg := ReadConfigFile(":fls_core:config/racas");
	if ( cfg.errortext )
		SysLog("Error - Unable to open [:fls_core:racas.cfg] ->"+cfg.errortext);
	endif

	return cfg;
endfunction


function GetTitleRace(racename)

	if (racename == "humano")
		return "Humano";
	elseif (racename == "orc")
		return "Uruk";
	elseif (racename == "hobbit")
		return "Hobbit";
	elseif (racename == "elfo")
		return "Quende";
	elseif (racename == "anao")
		return "Khuzd";
	elseif (racename == "drow")
		return "Ilythiiri";
	endif

endfunction

function IsPoisonImune(who, poison)

	if (who.npctemplate)
		var npcelem := NPC_GetNPCConfig(who.npctemplate);
		var imuneList := splitwords(npcelem.PoisonImune);
		print(" " + imuneList);
		foreach poisonName in imuneList
			if (poisonName == poison)
				return 1;
			elseif ( lower(poisonName) == "all")
				return 1;
			endif
		endforeach
		return 0;
	else
		var imuneList := splitwords(GetObjProperty(who, "PoisonImune"));
		foreach poisonName in imuneList
			if (poisonName == poison)
				return 1;
			elseif ( lower(poisonName) == "all")
				return 1;
			endif
		endforeach
		return 0;
	endif

endfunction

function FLS_GetSkillCap(who, skillname)

	if (!GetObjProperty(who, "chardata"))
		return 100.0;
	endif

	if (skillname == "classskill")
		return 100;
	endif
	if (skillname == "Begging")
		return 100;
	endif

	var primarias := GetSkills(who, "primarias");
	var secundarias := GetSkills(who, "secundarias");
	var terciarias := GetSkills(who, "terciarias");

	foreach skill in secundarias
		if (skillname == skill)
			return 70.0;
		endif
	endforeach

	foreach skill in terciarias
		if (skillname == skill)
			return 40.0;
		endif
	endforeach

	foreach skill in primarias
		if (skillname == skill)
			return 100.0;
		endif
	endforeach

	return 0.0;

endfunction

function FLS_GetSkills(who, category)

	var grad := 0;
	if (category == "primarias")
		grad := 3;
	elseif (category == "secundarias")
		grad := 2;
	elseif (category == "terciarias")
		grad := 1;
	endif

	if (!who.npctemplate)

		var chardata := GetObjProperty(who, "chardata");
		if ( (!chardata) || (chardata == error))
			return 0;
		else
			var skills := chardata.Skills;
			var skillsName := skills.keys();
			var primarias := array;
			foreach skill in skillsName
				if (skills[skill] == grad)
					primarias.append(GetTrueSkillName(skill));
				endif
			endforeach
			return primarias;
		endif
	endif

	return 0;
endfunction

function GetSkillTitle(who, skillname)

	if (skillname == "BowCraft")
		skillname := "BowcraftFletching";
	endif

	var value := AP_GetSkill(who, skillname);

	if (value >= 100 )
		return "Grao Mestre";
	elseif (value >= 80)
		return "Mestre";
	elseif (value >= 60)
		return "Competente";
	elseif (value >= 40)
		return "Treinado";
	elseif (value >= 20)
		return "Aprendiz";
	else
		return "Inexperiente";
	endif

endfunction


function GetSkillNameByID(input)

	var cfg := AP_GetAttributesCfgFile();
	var entries := GetConfigStringKeys(cfg);

	foreach entry in entries
		var elem := FindConfigElem(cfg, entry);
		if (elem.SkillId == input)
			return entry;
		endif
	endforeach

	return "";

endfunction

function destroyIllyraOutsideDungeons(who)
    var hasIllyra := 0;
    if (InDungeon(who))
        return 0;
    endif

    foreach item in EnumerateItemsInContainer(who.backpack)
        if (item.color == 2278)
            DestroyItem(item);
            hasIllyra := 1;
        endif
    endforeach
    
    foreach item in ListEquippedItems(who)
        if (item.color == 2278)
            DestroyItem(item);
            hasIllyra := 1;
        endif
    endforeach

    if (hasIllyra)
        SendSysMessage(who, "Voce foi para a superficie e todos seus itens de Illyra se tornaram po");
        var illyraDust := CreateItemInBackPack(who, 0x9092, 2);
        illyraDust.name := "Po de Illyra";
        illyraDust.color := 2278;
    endif

    return 1;
endfunction

function ProccessHab(who, txt)
	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var found := 0;
    var habelem;

    // Itera sobre as habilidades no arquivo de configuração para encontrar uma correspondência
    foreach elem in GetConfigStringKeys(habcfg)
        var normalized_hab := NormalizeString(elem); // Normaliza o nome da habilidade no arquivo de configuração
        if (NormalizeString(txt) == normalized_hab)
            habelem := FindConfigElem(habcfg, elem); // Usa o nome da habilidade original
            found := 1;
            break;
        endif
    endforeach

    if (!found)
        SendSysMessageEx(who, "Habilidade não encontrada.", SSM_FAIL);
        return 0;
    endif

	var uso := GetConfigString(habelem, "uso");
	var hab := GetConfigString(habelem, "nome");

	if (uso != "Ativa")
		SendSysMessageEx(who, "Esta habilidade nao pode ser ativada.");
		return 0;
	endif


//	sendsysmessage(who, " " + TS_GetCooldown(who, hab) );
	if ( TS_GetCooldown(who, hab) != error && who.cmdlevel < 3)
		SendSysMessageEx(who, "Voce nao pode usar essa habilidade agora.", SSM_FAIL);
		return 0;
	endif

	var stam := cint(GetConfigInt(habelem, "stam"));
	if (stam != error)
		if (!AP_ConsumeVital(who, STAMINA, stam))
			SendSysMessageEx(who, "Voce esta muito cansado para usar esta habilidade.", SSM_FAIL);
			HealStamina(who, stam);
			return 0;
		endif
	endif

	var mana := cint(GetConfigInt(habelem, "mana"));
	if (mana != error)
		if (!AP_ConsumeVital(who, MANA, mana))
			SendSysMessageEx(who, "Voce esta muito cansado para usar esta habilidade.", SSM_FAIL);
			HealMana(who, mana);
			return 0;
		endif
	endif

	var script := GetConfigString(habelem, "script");
	var duracao := GetConfigString(habelem, "duracao");

	var delay := Cint(GetConfigInt(habelem, "delay"));

	var mimic := 0;
	if (GetObjProperty(who, "#mimico"))
		mimic := SystemFindObjectBySerial(GetObjProperty(who, "#mimico"));
		if (mimic && (Distance(mimic, who) < 12))
			PrintText(mimic, "*copia a habilidade " + hab);
		endif
		EraseObjProperty(who, "#mimico");
	endif

	if (duracao == "1 ataque")
		if (!script)
			SendSysMessageEx(who, "Nao foi encontrado o script da habilidade: " + hab, SSM_FAIL);
			return 0;
		endif
        if (hab == "lamina da escuridao")
            PlayObjectCenteredEffectHuefx(who, 0x5487, 10, 20, 2296, 1);
            PlaySoundEffect(who, 0x56e);
        endif

		SendSysMessageEx(who, "Se voce acertar o proximo ataque, vai executar o movimento: " + hab, SSM_INFO);
		SetObjProperty(who, "movement",  script);
		SetObjProperty(who, "#movtime", ReadGameClock());
		if (mimic)
			SendSysMessageEx(mimic, "Se voce acertar o proximo ataque, vai executar o movimento: " + hab, SSM_INFO);
			SetObjProperty(mimic, "movement",  script);
			SetObjProperty(mimic, "#movtime", ReadGameClock());
		endif
	else
		var e := start_script(script, who );
		if (mimic)
			start_script(script, mimic );
		endif
		//sendsysmessage(who, " " + e );
	endif

	TS_StartCooldown(who, hab, delay);

	return 1;
endfunction

