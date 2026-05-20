
use uo;
use cfgfile;

include ":timedScripts:timedScripts";
include ":brainai:npcUtil";
include ":attributes:attributes";
include ":fls_core:fls_chargumps_ex";
include "include/dungeons";
include "include/epicConstants";
include "include/flash";
include ":tn:cooldown";
include ":nature:Controller/LightController";
include ":hotbar:include/cooldown_packet";

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
	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
	if (who.dead)
		return 0;
	endif

	if (!temHabilidade(who, txt) && (who.cmdlevel < 2))
		SendSysMessageEx(who, "Voce nao possui esta habilidade.", SSM_FAIL);
		return 0;
	endif

	var found := 0;
    var habelem;
    var abilityId := 0;
    // Itera sobre as habilidades no arquivo de configuração para encontrar uma correspondência
    foreach elem in GetConfigStringKeys(habcfg)
        var normalized_hab := NormalizeString(elem); // Normaliza o nome da habilidade no arquivo de configuração
        if (NormalizeString(txt) == normalized_hab)
            habelem := FindConfigElem(habcfg, elem); // Usa o nome da habilidade original
            abilityId := CInt(elem);  // V2: Save ability ID for AbilityMonitor
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
			PrintTextAbove(mimic, "*copia a habilidade " + hab);
		endif
		EraseObjProperty(who, "#mimico");
	endif

	// V2: ALL abilities go through AbilityMonitor - single source of truth
	var ability := Ability(abilityId);
	AbilityMonitor(ability).ActivateOn(MobileWrapper(who));

	// Mimic support
	if (mimic)
		AbilityMonitor(ability).ActivateOn(MobileWrapper(mimic));
	endif

	TS_StartCooldown(who, hab, delay);
	sendCooldownToClientUpdate(who, delay, habelem);

	return 1;
endfunction

function sendCooldownToClientUpdate(who, delay, habelem)
	// ✅ Determine class-specific gump ID
	var gumpid := 0x87; // Default fallback
	var classes := GetConfigStringDictionary(habelem, "classe");
	var hab := GetConfigString(habelem, "nome");
	
	// Priority: Sabio > Guerreiro > Ladino > Artifice
	if (classes["Sabio"] != error)
		gumpid := 0x9bac; // 39852
	elseif (classes["Guerreiro"] != error)
		gumpid := 0x9bad; // 39853
	elseif (classes["Ladino"] != error)
		gumpid := 0x9bae; // 39854
	elseif (classes["Artifice"] != error)
		gumpid := 0x9baf; // 39855
	endif
	
	// Send cooldown message to client with class-specific gump ID
	SendCooldownPacket(who, hab, CInt(delay), gumpid, COOLDOWN_TYPE_ABILITY);
endfunction
