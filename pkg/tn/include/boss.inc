
use uo;
use vitals;
include "include/say";
include ":gumps:requestGump";
include ":gumps:yesNo";
include ":merchants:item_template";
include ":attributes:attributes";
include ":tn:tngumps";
include ":tn:questevent";
include ":gumps:yesNo";
include "include/client";

function GetBossStruct(targ)
	var spawnpoint := SystemFindObjectBySerial( GetObjProperty(targ, "spawnpoint") );
	var unique := GetObjProperty(spawnpoint, "uniquestat");
	return unique;

endfunction

function isBoss(targ)
	var spawnpoint := SystemFindObjectBySerial( GetObjProperty(targ, "spawnpoint") );
	if (spawnpoint != error)
		var unique := GetObjProperty(spawnpoint, "unique");
		if (unique != error)
			return 1;
		endif
	endif
	return 0;
endfunction

function isTrueBoss(targ)
	var spawnpoint := SystemFindObjectBySerial( GetObjProperty(targ, "spawnpoint") );
	if (spawnpoint != error)
		var unique := GetObjProperty(spawnpoint, "true_boss");
		if (unique != error)
			return 1;
		endif
	endif
	return 0;
endfunction

function CreateBossStructGump(who, byref boss)

	if (boss.resist)
		if (!boss.phresist)
			boss.+phresist := boss.resist;
		endif
	endif
	if (boss.status)
		if (!boss.str)
			boss.+str := boss.status;
			boss.+dex := boss.status;
			boss.+int := boss.status;
		endif
	endif

	var chargump := GFCreateGump();
	var input;

	var cfg := ReadConfigFile(":brainAI:npcdesc");
	var npctemplate := boss.npctemplate;
	npctemplate := npctemplate[10, len(npctemplate)-9];
	cfg := cfg[npctemplate];


	//GFClosable(chargump, 0);
	GFDisposable(chargump, 0);

	GFResizePic(chargump, 0, 25, 3600, 500, 500);
	GFResizePic(chargump, 15, 40, 0x13BE, 470, 470);

	GFTextMid(chargump, 15, 50, 300, 1153, "Boss Menu");
	GFTextLine(chargump, 50, 80, 1153,  "NpcTemplate:");
	GFTextLine(chargump, 50, 95, 1153, "Nome: ");
	GFTextLine(chargump, 50, 110, 1153, "Cor: ");
	if (cfg)
		GFTextLine(chargump, 50, 125, 1153, "Str Mod ["+ cfg.Strength+ "]:");
	else
		GFTextLine(chargump, 50, 125, 1153, "Str Mod: ");
	endif
	if (cfg)
		GFTextLine(chargump, 50, 140, 1153, "Int Mod [" + cfg.Intelligence + "]:");
	else
		GFTextLine(chargump, 50, 140, 1153, "Int Mod: ");
	endif

	if (cfg)
		GFTextLine(chargump, 50, 155, 1153, "Dex Mod [" + cfg.Dexterity + "]:");
	else
		GFTextLine(chargump, 50, 155, 1153, "Dex Mod: ");
	endif
	GFTextLine(chargump, 50, 170, 1153, "Dmg Mod(0.1 a 1.0): ");

	if (cfg)
		GFTextLine(chargump, 50, 185, 1153, "Hits Mod [" + cfg.Hits+ "]:");
	else
		GFTextLine(chargump, 50, 185, 1153, "Hits Mod: ");
	endif


  	//if (cfg.specialai != error)
	//	GFTextLine(chargump, 50, 200, 1153, "Special AI [" + cfg.specialai+ "]:"); ///////////////////////
//	else
		//GFTextLine(chargump, 50, 200, 1153, "Special AI: ");
	//endif

	GFTextLine(chargump, 250, 80, 1153,  "Resistencias");
	if (cfg)
		GFTextLine(chargump, 250, 95, 1153, "Physical [" + cfg.PhysicalResist + "]:");
	else
		GFTextLine(chargump, 250, 95, 1153, "Physical: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 110, 1153, "Fire [" + cfg.FireResist + "]:");
	else
		GFTextLine(chargump, 250, 110, 1153, "Fire: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 125, 1153, "Cold [" + cfg.ColdResist + "]:");
	else
		GFTextLine(chargump, 250, 125, 1153, "Cold: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 140, 1153, "Energy [" + cfg.EnergyResist + "]:");
	else
		GFTextLine(chargump, 250, 140, 1153, "Energy: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 155, 1153, "Poison [" + cfg.PoisonResist + "]:");
	else
		GFTextLine(chargump, 250, 155, 1153, "Poison: ");
	endif

	GFTextLine(chargump, 250, 185, 1153, "Skills ");
	if (cfg)
		GFTextLine(chargump, 250, 200, 1153, "Wrestling Mod [" + cint(cfg.Wrestling)+ "]:");
	else
		GFTextLine(chargump, 250, 200, 1153, "Wrestling Mod: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 215, 1153, "Tactics Mod [" + cint(cfg.Tactics) + "]:");
	else
		GFTextLine(chargump, 250, 215, 1153, "Tactics Mod: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 230, 1153, "Awareness Mod [" + cint(cfg.Awareness) + "]:");
	else
		GFTextLine(chargump, 250, 230, 1153, "Awareness Mod: ");
	endif

	if (cfg)
		GFTextLine(chargump, 250, 245, 1153, "Magery Mod [" + cint(cfg.Magery) + "]:");
	else
		GFTextLine(chargump, 250, 245, 1153, "Magery Mod: ");
	endif

//	if (cfg)
//		GFTextLine(chargump, 250, 260, 1153, "R. Spells Mod [" + cint(cfg.ResistingSpells) + "]:");
//	else
//		GFTextLine(chargump, 250, 260, 1153, "R. Spells Mod: ");
//	endif

	var template, nome, cor, statmod, resistmod, dmgmod, hitsmod, skillsmod, specialai, descmod;
	var firesistmod, coresistmod, enresistmod, poresistmod, phresistmod;
	var str, dex, int;
	var wrestling, tactics, magery, awareness;


	if ( boss.npctemplate )
		template := GFTextEntry( chargump, 140, 80, 150, 20, 1150, boss.npctemplate);
	else
		template := GFTextEntry( chargump, 140, 80, 150, 20, 1150, ":brainai:");
	endif

	if(boss.name)
		nome := GFTextEntry( chargump, 140, 95, 100, 20, 1150, boss.name);
	else
		nome := GFTextEntry( chargump, 140, 95, 100, 20, 1150, "-nome-");
	endif

	if (boss.color)
		cor := GFTextEntry( chargump, 180, 110, 100, 20, 1150, boss.color+" ");
	else
		cor := GFTextEntry( chargump, 180, 110, 100, 20, 1150, "0");
	endif

	if (!boss.str) //boss.status boss.resist
		str := GFTextEntry( chargump, 180, 125, 100, 20, 1150, "0");
	else
		str := GFTextEntry( chargump, 180, 125, 100, 20, 1150, boss.str + "");
	endif

	if (!boss.int)
		int := GFTextEntry( chargump, 180, 140, 100, 20, 1150, "0");
	else
		int := GFTextEntry( chargump, 180, 140, 100, 20, 1150, boss.int + "");
	endif

	if (!boss.dex)
		dex := GFTextEntry( chargump, 180, 155, 100, 20, 1150, "0");
	else
		dex := GFTextEntry( chargump, 180, 155, 100, 20, 1150, boss.dex + "");
	endif

	if (!boss.dmg)
		dmgmod := GFTextEntry( chargump, 180, 170, 100, 20, 1150, "0");
	else
		dmgmod := GFTextEntry( chargump, 180, 170, 100, 20, 1150,  boss.dmg +"");
	endif

	if (!boss.hits)
		hitsmod := GFTextEntry( chargump, 180, 185, 100, 20, 1150, "0");
	else
		hitsmod := GFTextEntry( chargump, 180, 185, 100, 20, 1150, boss.hits+"");
	endif


	if (!boss.phresist)
		phresistmod := GFTextEntry( chargump, 350, 95, 100, 20, 1150, "0");
	else
		phresistmod := GFTextEntry( chargump, 350, 95, 100, 20, 1150, boss.phresist+"");
	endif

	if (!boss.firesist)
		firesistmod := GFTextEntry( chargump, 350, 110, 100, 20, 1150, "0");
	else
		firesistmod := GFTextEntry( chargump, 350, 110, 100, 20, 1150, boss.firesist+"");
	endif

	if (!boss.coresist)
		coresistmod := GFTextEntry( chargump, 350, 125, 100, 20, 1150, "0");
	else
		coresistmod := GFTextEntry( chargump, 350, 125, 100, 20, 1150, boss.coresist+"");
	endif

	if (!boss.enresist)
		enresistmod := GFTextEntry( chargump, 350, 140, 100, 20, 1150, "0");
	else
		enresistmod := GFTextEntry( chargump, 350, 140, 100, 20, 1150, boss.enresist+"");
	endif

	if (!boss.poresist)
		poresistmod := GFTextEntry( chargump, 350, 155, 100, 20, 1150, "0");
	else
		poresistmod := GFTextEntry( chargump, 350, 155, 100, 20, 1150, boss.poresist+"");
	endif

	if (!boss.wrestling)
		wrestling := GFTextEntry( chargump, 380, 200, 100, 20, 1150, "0");
	else
		wrestling := GFTextEntry( chargump, 380, 200, 100, 20, 1150, boss.wrestling+"");
	endif

	if (!boss.tactics)
		tactics := GFTextEntry( chargump, 380, 215, 100, 20, 1150, "0");
	else
		tactics := GFTextEntry( chargump, 380, 215, 100, 20, 1150, boss.tactics+"");
	endif

	if (!boss.awareness)
		awareness := GFTextEntry( chargump, 380, 230, 100, 20, 1150, "0");
	else
		awareness := GFTextEntry( chargump, 380, 230, 100, 20, 1150, boss.awareness+"");
	endif


	if (!boss.magery)
		magery := GFTextEntry( chargump, 380, 245, 100, 20, 1150, "0");
	else
		magery := GFTextEntry( chargump, 380, 245, 100, 20, 1150, boss.magery+"");
	endif

    GFAddButton(chargump, 110, 485, 2103, 2104, 1, 666);
    GFTextLine(chargump, 50, 480, 1153, "TrueBoss");

	GFTextLine(chargump, 50, 205, 1153, "Desc: ");
	GFAddButton( chargump, 35, 210, 2103, 2104, 1, 500);
		if (boss.descmod)
			GFHTMLArea(chargump, 90, 210, 160, 60, "[" + boss.descmod+ "]");
		endif

	GFTextLine(chargump, 50, 240, 1153, "Loot: ");
	GFAddButton( chargump, 50, 263, 2103, 2104, 1, 10);


	if (boss.loot)
		var y := 0;
		var i := 0;
		foreach lootinfo in (boss.loot)
			GFAddButton( chargump, 50, 283+y, 2103, 2104, 1, 11+i);
			GFTextLine(chargump, 60, 260+y, 1150, " " + lootinfo.item.desc + " [" + lootinfo.chance + "%]");
			y := y + 15;
			i := i + 1;
		endforeach
	endif

	GFTextLine(chargump, 250, 340, 1153, "Death Events: ");
	GFAddButton( chargump, 250, 363, 2103, 2104, 1, 20);

	if (boss.deathevent)
		var y := 0;
		var i := 0;
		foreach item in (boss.deathevent)
			GFAddButton( chargump, 250, 383+y, 2103, 2104, 1, 21+i);
			GFTextLine(chargump, 260, 360+y, 1150, " " + item.type);
			y := y + 15;
			i := i + 1;
		endforeach
	endif

	GFTextLine(chargump, 250, 260, 1153, "Special: ");
	GFAddButton( chargump, 250, 283, 2103, 2104, 1, 30);
	if (boss.special)
		var y := 0;
		var i := 0;
		foreach item in (boss.special)
			GFAddButton( chargump, 250, 303+y, 2103, 2104, 1, 31+i);
			GFTextLine(chargump, 260, 280+y, 1150, " " + item.name);
			y := y + 15;
			i := i + 1;
		endforeach
	endif


    GFTextLine(chargump, 50, 420, 1153, "Combat Events: ");
	GFAddButton( chargump, 50, 443, 2103, 2104, 1, 40);
	if (boss.combatevent)
		var y := 0;
		var i := 0;
		foreach item in (boss.combatevent)
			GFAddButton( chargump, 50, 463+y, 2103, 2104, 1, 41+i);
			GFTextLine(chargump, 60, 440+y, 1150, " " + item.name);
			y := y + 15;
			i := i + 1;
		endforeach
	endif

	GFAddButton(chargump, 380, 467, 4502, 4502, 1, 1 );
	GFTextLine(chargump, 340, 480, 1153, "Fim");

	input := GFSendGump(who, chargump);


	if (boss.npctemplate == error)
		sendsysmessage(who, "criando");
		boss.+npctemplate := Cstr(GFExtractData(input, template));
		boss.+name := Cstr(GFExtractData(input, nome));
		boss.+color := Cint(GFExtractData(input, cor));
		boss.+str := Cint(GFExtractData(input, str));
		boss.+dex := Cint(GFExtractData(input, dex));
		boss.+int := Cint(GFExtractData(input, int));
		boss.+resist := Cint(GFExtractData(input, resistmod));
		boss.+hits := Cint(GFExtractData(input, hitsmod));
		boss.+dmg := Cdbl(GFExtractData(input, dmgmod));
		boss.+firesist := Cint(GFExtractData(input, firesistmod));
		boss.+coresist := Cint(GFExtractData(input, coresistmod));
		boss.+enresist := Cint(GFExtractData(input, enresistmod));
		boss.+poresist := Cint(GFExtractData(input, poresistmod));
		boss.+phresist := Cint(GFExtractData(input, phresistmod));
		boss.+wrestling := Cint(GFExtractData(input, wrestling));
		boss.+tactics := Cint(GFExtractData(input, tactics));
		boss.+awareness := Cint(GFExtractData(input, awareness));
		boss.+magery := Cint(GFExtractData(input, magery));
	else
		sendsysmessage(who, "atualizando");
		boss.npctemplate := Cstr(GFExtractData(input, template));
		boss.name := Cstr(GFExtractData(input, nome));
		boss.color := Cint(GFExtractData(input, cor));
		boss.str := Cint(GFExtractData(input, str));
		boss.dex := Cint(GFExtractData(input, dex));
		boss.int := Cint(GFExtractData(input, int));
		boss.skills := Cint(GFExtractData(input, skillsmod));
		boss.resist := Cint(GFExtractData(input, resistmod));
		boss.hits := Cint(GFExtractData(input, hitsmod));
    //boss.specialai := Cint(GFExtractData(input, specialai));
		boss.dmg := Cdbl(GFExtractData(input, dmgmod));
		boss.firesist := Cint(GFExtractData(input, firesistmod));
		boss.coresist := Cint(GFExtractData(input, coresistmod));
		boss.enresist := Cint(GFExtractData(input, enresistmod));
		boss.poresist := Cint(GFExtractData(input, poresistmod));
		boss.phresist := Cint(GFExtractData(input, phresistmod));
		boss.wrestling := Cint(GFExtractData(input, wrestling));
		boss.tactics := Cint(GFExtractData(input, tactics));
		boss.awareness :=  Cint(GFExtractData(input, awareness));
		boss.magery := Cint(GFExtractData(input, magery));
	endif

		//GFTextLine(chargump, 50, 205, 1153, "Desc [" + cfg.Desc+ "]:");
	if (input[0] == 1 )
		//SendSysMessage(who, " " + boss + " " );
		//SendSysMessage(who, " " + boss + " " + Cstr(GFExtractData(input, template)) );
		return;
	elseif (input[0] == 500)
	var str1 := SmallRequestGump(who, "Descreva o item", 0);
			boss.descmod := str1;
			CreateBossStructGump(who, boss);

	elseif (input[0] == 10 || input[0] == 11 || input[0] == 12 || input[0] == 13 || input[0] == 14 || input[0] == 15 || input[0] == 16 || input[0] == 17 || input[0] == 18 || input[0] == 19)

		var index := input[0] - 9;
		//sendsysmessage(who, " " + input[0] + " " + boss.loot[index]);

		if ( boss.loot[index] != error )
			var opt := RadioGump(who, 180, 220, "Editar item", array{"Criar", "Deletar", "Alterar Chance"});
			if (opt == "Deletar")
				boss.loot.erase(index);
				SendSysMessageEx(who, "O item foi removido.", SSM_INFO);
			elseif ( opt == "Criar" )
				var retitem := CreateItemInBackpackFromTemplate(who, boss.loot[index].item);
				if (!retitem)
					SendSysMessageEx(who, "erro: " + retitem);
				endif
				SendSysMessageEx(who, "Voce criou o item na sua mochila.");
			elseif (opt == "Alterar Chance")
				boss.loot[index].chance := Cint(RequestGump(who, "Digite a chance do item aparecer. 1 a 100:", "", "", 1));
			endif

		else

			SendSysMessage(who, "Escolha o item do loot.");
			var targ := Target(who);
			if (targ)
				if (!boss.loot)
					boss.+loot := array;
				endif
				var lootinfo := struct;
				lootinfo.+chance := Cint(RequestGump(who, "Digite a chance do item aparecer. 1 a 100:", "", "", 1));
				lootinfo.+item := CreateItemTemplate(targ);
				(boss.loot).append(lootinfo);
			endif
		endif

		CreateBossStructGump(who, boss);

	elseif ( input[0] == 20 || input[0] == 21 || input[0] == 22 || input[0] == 23 || input[0] == 24|| input[0] == 25)

		var index := input[0] - 19;
		if (boss.deathevent[index] != error )
			var yes := YesNo(who, "Deletar o evento?");
			if (yes == 1)
				boss.deathevent.erase(index);
				SendSysMessageEx(who, "Evento deletado.", SSM_INFO);
			else
				SendSysMessageEx(who, "Cancelado.", SSM_INFO);
			endif
		else
				var nevent := ConfigureEvent(who);
				if (nevent)
					if (!boss.deathevent)
						boss.+deathevent := array;
					endif
					(boss.deathevent).append(nevent);
				endif
		endif
		CreateBossStructGump(who, boss);
	elseif ( input[0] == 40 || input[0] == 41 || input[0] == 42 || input[0] == 43 || input[0] == 44 || input[0] == 45)

		var index := input[0] - 39;
		if (boss.combatevent[index] != error )
			var yes := YesNo(who, "Editar ou Deletar Evento?", "Editar", "Deletar");
			if (yes == 1)
				var nevent := ConfigureCombatEvent(who, boss.combatevent[index] );
				(boss.combatevent).erase(index);
				if (nevent)
					(boss.combatevent).append(nevent);
				endif
			else
				boss.combatevent.erase(index);
				SendSysMessageEx(who, "Evento deletado.", SSM_INFO);
			endif
		else
				var nevent := ConfigureCombatEvent(who);
				if (nevent)
					if (!boss.combatevent)
						boss.+combatevent := array;
					endif
					(boss.combatevent).append(nevent);
				endif
		endif
		CreateBossStructGump(who, boss);

    elseif ( input[0] == 666 )
        var yes := YesNo(who, "TrueBoss?");
        if (yes == 1)
            if (!boss.trueboss)
                boss.+trueboss := 1;
            endif
        endif
        CreateBossStructGump(who, boss);

	elseif ( input[0] == 30 || input[0] == 31 || input[0] == 32 || input[0] == 33 || input[0] == 34 || input[0] == 35)
		var index := input[0] - 29;

		if (boss.special[index] != error)
			var yes := YesNo(who, "Deletar a habilidade?");
			if (yes == 1)
				boss.special.erase(index);
				SendSysMessageEx(who, "Habilidade deletada.", SSM_INFO);
			else
				SendSysMessageEx(who, "Cancelado.", SSM_INFO);
			endif

		else
			var opt := RadioGump(who, 250, 260, "Editar item", array{"Ice Hit", "Fire Hit", "Energy Hit", "Life Drain", "Dmg Return", "Custom Hitscript", "Special AI"});
			var spec := struct;
			spec.+name := opt;
			if (!boss.special)
				boss.+special := array;
			endif
			if ( opt == "Dmg Return")
				spec.+amt := Cdbl(RequestGump(who, "Digite quando sera retornado. 0.1 a 1.0:", "", "", 1));
     			elseif (opt == "Custom Hitscript")
       	 			spec.+hitopt := configurehitscript(who);
      			elseif (opt == "Special AI")
         			var ainame := Cstr(RequestGump(who, "Digite o nome da IA", "", "", 1));
        			spec.+ai := ainame;
			endif
			boss.special.append(spec);
		endif

		CreateBossStructGump(who, boss);
	endif

	//return 0;
endfunction

function showHurtDescription(npc)
    var currentHealth := AP_GetVital(npc, HITS) / CInt(AP_GetVitalMaximumValue(npc, HITS));
    
    if (currentHealth >= 0.75 && currentHealth < 1.0)
        return "<BASEFONT color=#3366FF>Levemente Ferido";
    elseif (currentHealth >= 0.50 && currentHealth < 0.75)
        return "<BASEFONT color=#007A23>Ferido";
    elseif (currentHealth >= 0.25 && currentHealth < 0.50)
        return "<BASEFONT color=#FFE700>Gravemente Ferido";
    elseif (currentHealth > 0.0 && currentHealth < 0.25 )
        return "<BASEFONT color=#CC0000>Próximo da morte";
    else
        return "";
    endif
endfunction

  //transforma um npc em boss
function SetBoss(npc, boss)

	SetName(npc, boss.name);
	npc.color := boss.color;
	npc.truecolor := boss.color;
	SetObjProperty(npc, "descmodboss", Cstr(boss.descmod));
	SetObjProperty(npc, "dmgbonus", Cdbl(boss.dmg));

	if (boss.trueboss)
        Setobjproperty(npc, "true_boss", 1);
    endif

	if (boss.resist)
		npc.resist_physical_mod := Cint(boss.resist);
	else
		npc.resist_physical_mod := Cint(boss.phresist);
		npc.resist_fire_mod := Cint(boss.firesist);
		npc.resist_cold_mod := Cint(boss.coresist);
		npc.resist_energy_mod := Cint(boss.enresist);
		npc.resist_poison_mod := Cint(boss.poresist);
	endif


	if (boss.status)
		AP_SetTrueStat(npc, STRENGTH, Cint(boss.status) + AP_GetStat(npc, STRENGTH));
		AP_SetTrueStat(npc, DEXTERITY, Cint(boss.status) + AP_GetStat(npc, DEXTERITY));
		AP_SetTrueStat(npc, INTELLIGENCE, Cint(boss.status) + AP_GetStat(npc, INTELLIGENCE));
	else
		AP_SetTrueStat(npc, STRENGTH, Cint(boss.str) + AP_GetStat(npc, STRENGTH));
		AP_SetTrueStat(npc, DEXTERITY, Cint(boss.dex) + AP_GetStat(npc, DEXTERITY));
		AP_SetTrueStat(npc, INTELLIGENCE, Cint(boss.int) + AP_GetStat(npc, INTELLIGENCE));
	endif

	if(boss.skills)
		AP_SetTrueSkill(npc, TACTICS, Cint(boss.skills) + AP_GetSkill(npc, TACTICS) );
		AP_SetTrueSkill(npc, WRESTLING, Cint(boss.skills) + AP_GetSkill(npc, WRESTLING));
		AP_SetTrueSkill(npc, MAGERY, Cint(boss.skills) + AP_GetSkill(npc, MAGERY) );
		AP_SetTrueSkill(npc, AWARENESS, Cint(boss.skills) + AP_GetSkill(npc, AWARENESS) );
	else
		AP_SetTrueSkill(npc, TACTICS, Cint(boss.tactics) + AP_GetSkill(npc, TACTICS) );
		AP_SetTrueSkill(npc, WRESTLING, Cint(boss.wrestling) + AP_GetSkill(npc, WRESTLING));
		AP_SetTrueSkill(npc, MAGERY, Cint(boss.magery) + AP_GetSkill(npc, MAGERY) );
		AP_SetTrueSkill(npc, AWARENESS, Cint(boss.awareness) + AP_GetSkill(npc, AWARENESS) );
	endif

  	SetObjProperty(npc, "hitsmod", cint(boss.hits));
  	RecalcVitals(npc);
	AP_RefreshVitals(npc);

	if (!npc.backpack)
		var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z,  2482,  1, npc.realm);
		EquipItem(npc, backpack);
	endif

	foreach iteminfo in (boss.loot)
    		var amt := iteminfo.item.Amount;
    		if (!amt)
      			amt := 1;
    		endif
		//printtextabove(npc, " " + iteminfo);
		if (RandomInt(100) < iteminfo.chance)
			var theitem := CreateItemInBackpackFromTemplate(npc, iteminfo.item);
			//printtextabove(npc, " " + theitem);
    			theitem.movale := 1;
    			if (theitem.amount == 1)
        			Setname(theitem, iteminfo.item.desc);
	    		endif
		endif
	endforeach

//printtextabove(npc, " " + boss.special);
	if (boss.deathevent)
		var lwprop := array;
		foreach nevent in (boss.deathevent)
			if (nevent.type == "Ultimas Palavras")
				lwprop.append(nevent.text);
			else
				PrepareEvent(nevent);
			endif
		endforeach
		if (lwprop.size() > 0)
			SetObjProperty(npc, "Ultimas Palavras", lwprop);
		endif
	endif

  if (boss.customai)
    npc.script := boss.customai;
    restartscript(npc);
  endif

	if (boss.special)
		foreach spec in (boss.special)
   //   printtextabove(npc,  " " + spec.name);
			if (spec.name == "Ice Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/IceHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Fire Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/FireHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Energy Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/LightningHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Life Drain")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/draintouch");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Dmg Return")
				var hit_scripts := GetObjProperty(npc, "ArmorHitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:armorHitScripts/dmgreturn");
				SetObjProperty(npc, "ArmorHitScripts", hit_scripts);
				SetObjProperty(npc, "dmgreturn",  spec.amt);
      elseif (spec.name == "Custom Hitscript")
          SetObjProperty(npc, "generichit", spec.hitopt);
          var hit_scripts := GetObjProperty(npc, "HitScripts");
          if ( !hit_scripts )
            hit_scripts := array{};
          endif
          hit_scripts.Append(":combat:weaponHitScripts/generic");
          SetObjProperty(npc, "HitScripts", hit_scripts);
        elseif (spec.name == "Special AI")
            npc.script := spec.ai;
      //      printtextabove(npc, " " + spec.ai);
            restartscript(npc);
			endif
		endforeach
	endif

	return npc;

endfunction

//cria um boss
function CreateBoss(boss, x, y, z, realm)

	var npc := CreateNpcFromTemplate(boss.npctemplate, x, y, z, 0 , realm);
	if (npc == error)
    //print("createboss " + npc + " " + boss.npctemplate);
		return error{"errortext":="Nao foi possivel criar o boss em x " + x + " y " + y + " z  " + z};
	endif

	SetName(npc, boss.name);
	npc.color := boss.color;
	npc.truecolor := boss.color;

    if (boss.trueboss)
        Setobjproperty(npc, "true_boss", 1);
    endif

	SetObjProperty(npc, "dmgbonus", Cdbl(boss.dmg));
	if (boss.resist)
		npc.resist_physical_mod := Cint(boss.resist);
	else
		npc.resist_physical_mod := Cint(boss.phresist);
		npc.resist_fire_mod := Cint(boss.firesist);
		npc.resist_cold_mod := Cint(boss.coresist);
		npc.resist_energy_mod := Cint(boss.enresist);
		npc.resist_poison_mod := Cint(boss.poresist);
	endif
	if (boss.status)
		AP_SetStatMod(npc, STRENGTH, Cint(boss.status));
		AP_SetStatMod(npc, DEXTERITY, Cint(boss.status));
		AP_SetStatMod(npc, INTELLIGENCE, Cint(boss.status));
	else
		AP_SetStatMod(npc, STRENGTH, Cint(boss.str));
		AP_SetStatMod(npc, DEXTERITY, Cint(boss.dex));
		AP_SetStatMod(npc, INTELLIGENCE, Cint(boss.int));
	endif

	if(boss.skills)
		AP_SetSkillMod(npc, TACTICS, Cint(boss.skills));
		AP_SetSkillMod(npc, WRESTLING, Cint(boss.skills));
		AP_SetSkillMod(npc, MAGERY, Cint(boss.skills));
		AP_SetSkillMod(npc, AWARENESS, Cint(boss.skills));
	else
		AP_SetSkillMod(npc, TACTICS, Cint(boss.tactics));
		AP_SetSkillMod(npc, WRESTLING, Cint(boss.wrestling));
		AP_SetSkillMod(npc, MAGERY, Cint(boss.magery));
		AP_SetSkillMod(npc, AWARENESS, Cint(boss.awareness));
	endif

	  SetObjProperty(npc, "hitsmod", cint(boss.hits));
	  RecalcVitals(npc);
	AP_RefreshVitals(npc);

	if (!npc.backpack)
		var backpack := CreateItemAtLocation(npc.x, npc.y, npc.z,  2482,  1, npc.realm);
		EquipItem(npc, backpack);
	endif

	foreach iteminfo in (boss.loot)
    		var amt := iteminfo.item.Amount;
    		if (!amt)
      			amt := 1;
    		endif
		if (RandomInt(100) < iteminfo.chance)
			var theitem := CreateItemInBackpackFromTemplate(npc, iteminfo.item);
    			theitem.movale := 1;
    			if (theitem.amount == 1)
        			Setname(theitem, iteminfo.item.desc);
	    		endif
		endif
	endforeach


	if (boss.deathevent)
		var lwprop := array;
		foreach nevent in (boss.deathevent)
			if (nevent.type == "Ultimas Palavras")
				lwprop.append(nevent.text);
			else
				PrepareEvent(nevent);
			endif
		endforeach
		if (lwprop.size() > 0)
			SetObjProperty(npc, "Ultimas Palavras", lwprop);
		endif
	endif

  //printtextabove(npc, " " + boss.special);
	if (boss.special)
		foreach spec in (boss.special)
    //printtextabove(npc,  " " + spec.name);
			if (spec.name == "Ice Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/IceHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Fire Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/FireHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Energy Hit")
				var hit_scripts := GetObjProperty(npc, "HitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:weaponHitScripts/LightningHit");
				SetObjProperty(npc, "HitScripts", hit_scripts);
			elseif (spec.name == "Life Drain")
				var hit_scripts := GetObjProperty(npc, "ArmorHitScripts");
				if ( !hit_scripts )
					hit_scripts := array{};
				endif
				hit_scripts.append(":combat:armorHitScripts/dmgreturn");
				SetObjProperty(npc, "ArmorHitScripts", hit_scripts);
				SetObjProperty(npc, "dmgreturn",  spec.amt);
			elseif (spec.name == "Dmg Return")
      elseif (spec.name == "Custom Hitscript")
          SetObjProperty(npc, "generichit", spec.hitopt);
          var hit_scripts := GetObjProperty(npc, "HitScripts");
          if ( !hit_scripts )
            hit_scripts := array{};
          endif
          hit_scripts.Append(":combat:weaponHitScripts/generic");
          SetObjProperty(npc, "HitScripts", hit_scripts);
        elseif (spec.name == "Special AI")
            npc.script := spec.ai;
     //       printtextabove(npc,  " " + spec.ai);
            restartscript(npc);
			endif
		endforeach
	endif


	return npc;
endfunction

function ConfigureCombatEvent(who, event := 0)
	if (!event)
		event := struct;
		event.+name := RequestGump(who, "Digite o nome desse evento de combate", "Ex.: Ataque Multiplo", "", 1);
		event.+cd := cint(RequestGump(who, "De quanto em quanto tempo o npc realiza esse evento?", "", "", 1));
		event.+onlybloodied := cint(YesNo(who, "O NPC so faz esse evento se tiver bloodied?"));
		event.+dist := cint(RequestGump(who, "Qual a distancia que o npc precisa estar do alvo?", "0 para nao checar distancia", "", 1));

		event.+steps := array;
	endif
	var continua := 1;

	while (continua)
	var gumpcombat := GFCreateGump();

	GFDisposable(gumpcombat, 0);
	GFResizePic(gumpcombat, 0, 25, 3600, 250, 430);
	GFResizePic(gumpcombat, 15, 40, 0x13BE, 220, 400);
	GFTextMid(gumpcombat, 20, 50, 250, 1153, "Combat Events");

	var y := 64;
	GFTextLine(gumpcombat, 35, y, 1153, "Adicionar Novo ");
	GFAddButton(gumpcombat, 20, y+2, 2117, 2118, 1, 100);

	var i := 1;
	foreach state in (event.steps)
		y := y+ 15;
		GFTextLine(gumpcombat, 35, y, 1153, ""+state.type);
		GFAddButton(gumpcombat, 20, y+2, 2117, 2118, 1, 400+i);
		i := i + 1;
	endforeach

	GFAddButton(gumpcombat, 180, 387, 4502, 4502, 1, 1000 );
	GFTextLine(gumpcombat, 140, 400, 1153, "Fim");

	var input := GFSendGump(who, gumpcombat);

	if (input[0] == 100)
		var state := CombatEventsOptions(who, struct{});
		event.steps.append(state);
	elseif (input[0] == 1000)
		continua := 0;
	elseif (input[0] >= 400)
		input := input[0] - 400;
		event.steps.Erase(input);
		if (YesNo(who, "Deseja Alterar ou Remover?","Alterar","Remover") )
			var state := CombatEventsOptions(who, struct{});
			event.steps.Insert(input, state);
		else
			SendSysMessage(who, "Evento Removido");
		endif
	endif
	endwhile



	return event;

endfunction

function CombatEventsOptions(who, state)
		var opt := RadioGump(who, 180, 370, "Adicionar Estado", array{"Move", "Teleport", "TimedScript", "Perform Action", "Text", "Play Sound", "Heal", "Dmg", "Graphic Effect",  "Create Items", "Summon Npcs", "Custom HitScript", "Delay"});

		state.type := opt;
		if (opt == "Move")
			SendSysMessageEx(who, "Para onde o npc deve andar?");
			var loc := TargetCoordinates(who);
			state.+x := loc.x;
			state.+y := loc.y;
			state.+z := loc.z;
		elseif ( opt == "Teleport")
			state.+target := RadioGump(who, 210, 200, "Quem vai sofrer o teleporte?", array{"Self", "Opponent", "All Opponents"});
			var loc := TargetCoordinates(who);
			state.+x := loc.x;
			state.+y := loc.y;
			state.+z := loc.z;
		elseif ( opt == "TimedScript")
			state.+target := RadioGump(who, 200, 180, "Quem vai realizar a action?", array{"Self", "Opponent", "All Opponents"});
			var timername := RadioGump(who, 270, 580, "Escolha o timedscript", array{"defaultPoison", "hallucination", "paralysis", "clumsy", "feeblemind", "weaken", "atrasarmagias", "fugademana", "staminaPoison", "manaPoison", "bleed", "darkborer", "agility", "strength", "invisibilidade", "desviarprojetil", "dmgreduction", "evasao", "precisao", "phmod", "fimod", "comod", "pomod", "enmod"});
			state.+timername := timername;
			state.+timerduration := cint(RequestGump(who, "Qual a duracao do timer?", "Nunca mais de 120s", "", 1));
			state.+level := cint(RequestGump(who, "Qual o level do timer?", "", "", 1));

		elseif ( opt == "Perform Action" )
			state.+target := RadioGump(who, 200, 180, "Quem vai realizar a action?", array{"Self", "Opponent", "All Opponents"});
			state.+id := cint(RequestGump(who, "Qual o ID da animacao?", "", "", 1));
		elseif (opt == "Text")
			state.+target := RadioGump(who, 200, 180, "Quem vai falar?", array{"Self", "Opponent", "All Opponents"});
			state.+text := RequestGump(who, "Qual o texto ?", "", "", 1);
			state.+color := cint(RequestGump(who, "Qual a cor do texto?", "", "1000", 1));
		elseif (opt == "Play Sound")
			state.+id := cint(RequestGump(who, "Qual o ID do som?", "", "", 1));
		elseif (opt == "Delay")
			state.+delay := cint(RequestGump(who, "Quanto tempo de delay?", "", "", 1));
		elseif (opt == "Heal")
			state.healamt := cint(RequestGump(who, "Quanto vai healar?", "", "", 1));
		elseif (opt == "Graphic Effect")
			state.+target := RadioGump(who, 250, 200, "Onde vai aparecer o efeito? (fixed loc deve ser Moving)", array{"Self", "Opponent", "All Opponents", "Fixed Loc"});
			state.+graphictype := RadioGump(who, 180, 150, "Qual o tipo de efeito?", array{"Stationary", "Moving"});
			if (state.graphictype == "Stationary")
				var fxid :=  RadioGump(who, 230, 300, "Qual o efeito?", array{"Lightning", "Red Sparkles", "Blue Sparkles", "Blue and Swirl", "Swirl Sparkles", "Blue Sparkles on Ground", "Fire Column", "Outro"});
				if ( fxid == "Lightning" )
					state.+fxid := fxid;
				elseif ( fxid == "Red Sparkles")
					state.+fxid := GFX_RED_SPARKLES;
					state.+speed := 7;
					state.+loops := 7;
				elseif ( fxid == "Blue Sparkles")
					state.+fxid := GFX_BLUE_SPARKLES;
					state.+speed := 10;
					state.+loops := 10;
				elseif (fxid == "Blue and Swirl")
					state.+fxid := GFX_BLUE_SPARKLE_SWIRL;
					state.+speed := 7;
					state.+loops := 16;
				elseif (fxid == "Swirl Sparkles")
					state.+fxid := GFX_BLUE_SPARKLE_BORDER;
					state.+speed := 7;
					state.+loops := 0;
				elseif (fxid == "Blue Sparkles on Ground")
					state.+fxid := GFX_BLUE_SPARKLE_GROUND;
					state.+speed := 7;
					state.+loops := 10;
				elseif (fxid == "Fire Column");
					state.+fxid := GFX_FIRE_COLUMN;
					state.+speed := 7;
					state.+loops := 0;
				elseif (fxid == "Outro")
					state.+fxid := hex(RequestGump(who, "Qual o ID do efeito?", "", "", 1));
					state.+speed := 7;
					state.+loops := 16;
				endif

			elseif (state.graphictype == "Moving")
				var fxid := RadioGump(who, 180, 280, "Qual o efeito?", array{"Small Fireball", "Large Fireball", "Wavy Fireball", "Other"});
				state.+direction := RadioGump(who, 200, 180, "Qual vai ser a direcao do efeito?", array{"Mob->Player", "Player->Mob"});
				if (fxid == "Small Fireball")
					state.fxid := GFX_SMALL_FIREBALL;
					state.+speed := 5;
					state.+loops := 0;
				elseif (fxid == "Large Fireball")
					state.fxid := GFX_LARGE_FIREBALL;
					state.+speed := 5;
					state.+loops := 0;
				elseif (fxid == "Wavy Fireball")
					state.fxid := GFX_WAVY_FIREBALL;
					state.+speed := 5;
					state.+loops := 0;
				elseif (fxid == "Other")
					state.+fxid := cint(RequestGump(who, "Qual o ID do efeito?", "", "", 1));
					state.+speed := 5;
					state.+loops := 0;
				endif
				if (state.target == "Fixed Loc")
					var loc := RequestGump(who, "Digite o x y z onde o efeito vai aparecer:", "", "", 1);
	                		loc := splitwords(loc);
        	        		state.+startx :=  loc[1];
         	       			state.+starty :=  loc[2];
               	 			state.+startz :=  loc[3];
 					var radius :=   RequestGump(who, "Qual o raio de onde o efeito vai aparecer:", "0 se for sempre no mesmo lugar", "0", 1);
					state.startradius := radius;

					if (state.graphictype == "Moving")
						var loc2 := RequestGump(who, "Digite o x y z da posicao final do efeito:", "", "", 1);
	                			loc2 := splitwords(loc);
        	        			state.+endx :=  loc2[1];
         	       				state.+endy :=  loc2[2];
               	 				state.+endz :=  loc2[3];
 						var radius :=   RequestGump(who, "Qual o raio de onde o efeito vai terminar:", "0 se for sempre no mesmo lugar", "0", 1);
						state.endradius := radius;

					endif

				endif
			endif
		elseif ( opt == "Dmg")
			state.+target := RadioGump(who, 200, 180, "Quem vai falar?", array{"Self", "Opponent", "All Opponents"});
			var dmg := cint(RequestGump(who, "Quanto de dano vai causar", "", "", 1));
			var dmgtype := RequestGump(who, "Digite qual o tipo de dano: Physical, Fire, Cold, Poison, Energy ", "", "", 1);
			state.+dmg := dmg;
			state.+dmgtype := dmgtype;
			var vitals := CheckBoxGump(who, 230, 180, "Qual tipo de vital sera afetado pelo dano?", array{"Stam", "Hits", "Mana"});
			state.+vitals := vitals;
		elseif ( opt == "Create Items")
			SendSysMessage(who, "Nao implementado");
		elseif (opt == "Summon Npcs")
        	        state.+amount := Cint(RequestGump(who, "Quantos npcs vao ser spawnados:", "", "", 1));
	                var loc := RequestGump(who, "Digite o x y z onde eles vao aparecer:", "Escreva 6 6 6 se for para aparecer onde o boss esta", "", 1);
	                loc := splitwords(loc);
        	        state.+x :=  loc[1];
         	       	state.+y :=  loc[2];
               	 	state.+z :=  loc[3];
               	 	state.+range := Cint(RequestGump(who, "Digite o raio de spawn:", "", "", 1));
                	state.+npctemplate := RequestGump(who, "Digite o npctemplate:", "", "", 1);
					state.+tempo := Cint(RequestGump(who, "Coloque o tempo de vida do monstro(2 minutos default):", "", "", 1));

		elseif (opt == "Custom HitScript")
			SendSysMessage(who, "Nao implementado");
		endif
		return state;
endfunction
