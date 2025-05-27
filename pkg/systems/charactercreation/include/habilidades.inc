use cfgfile;

include ":attributes:attributes";
include "include/epicConstants";
include "include/epicUtils";
include ":tn:equipment";

function NivelHabilidade(habilidade, classe)

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var elem := FindConfigElem( habcfg, habilidade );

	var nivel := GetConfigStringDictionary( elem, "classe" );

	return nivel[classe];

endfunction

function TemRequisito(who, habilidade, msg := 1)

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var elem := FindConfigElem( habcfg, habilidade );

	var requisitos := GetConfigStringArray(elem, "requisito");
	var chardata := GetObjProperty(who, "chardata");
	foreach req in (requisitos)
		if (req["kit"]) // Ver se tem Kit!
			req["kit "] := "";
			if ((!chardata).trait in (req))
				if (msg == 1)
					SendSysMessageEx(who, "Voce nao possui a Caracteristica: " + req, SSM_FAIL);
				endif
				return 0;
			endif
        elseif (req["cultura"]) // Adiciona busca por culturas
            req["cultura"] := "";
            if (!((chardata).culture in (req)))
				if (msg == 1)
					SendSysMessageEx(who, "Voce nao pertence a cultura: " + req, SSM_FAIL);
				endif
				return 0;
			endif
		elseif ( !TemHabilidade(who, req) )
			if (msg == 1)
				SendSysMessageEx(who, "Voce nao possui o pre requisito: " + req, SSM_FAIL);
			endif
			return 0;
		endif

	endforeach

	return 1;


endfunction


function retornaRequisitos( habilidade)

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var elem := FindConfigElem( habcfg, habilidade );

	var requisitos := GetConfigStringArray(elem, "requisito");

	return requisitos;

endfunction

function TemHabilidade(who, habilidade)

	var habs := GetObjProperty(who, "myhabs");

	foreach key in (habs.keys())
		if (RemoveAcentos(Lower(habs[key])) == RemoveAcentos(Lower(habilidade)))
			return 1;
		endif

	endforeach


	return 0;

endfunction

function IsBloodied(mobile)
	if (mobile.npctemplate)
		if (AP_GetVital(mobile, HITS) < CInt(AP_GetVitalMaximumValue(mobile, HITS)/4))
			return 1;
		endif
	elseif (AP_GetVital(mobile, HITS) < GetHP(mobile))
		return 1;
	endif
	return 0;
endfunction

function IsRaging(mobile)
	//Bonus da habilidde baixo limiar de furia -> Nazg Khalug
	var bonus_limiar_furia_hp := 0;
	// if (TemHabilidade(mobile, "Nazg Khalug"))
	// 	bonus_limiar_furia_hp := bonus_limiar_furia_hp + 25;
	// endif

	if (!mobile.npctemplate && AP_GetVital(mobile, HITS) <= (GetHP(mobile) + bonus_limiar_furia_hp))
		return 1;
	endif

	if (mobile.npctemplate)
		if (AP_GetVital(mobile, HITS) < ( CInt(AP_GetVitalMaximumValue(mobile, HITS)/2) + bonus_limiar_furia_hp))
			return 1;
		endif
	elseif (AP_GetVital(mobile, HITS) < GetHP(mobile) + bonus_limiar_furia_hp)
		return 1;
	endif
	return 0;
endfunction

function HabilidadesPorNivel(who, classe, lvl)

	var possiveis := array;

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var keys := getConfigStringKeys( habcfg );

	foreach key in keys
		var elem := FindConfigElem( habcfg, key );
		var d := GetConfigStringDictionary( elem, "classe" );
//		sendsysmessage(who, "elem " + elem + " classe " + classe  + " d[classe] " + d[classe]  );
		if ( cint(d[classe]) == lvl && d[classe] != error)
			possiveis.append(key);
		endif
	endforeach

	return possiveis;

endfunction

function HabilidadesPorNivelPossiveis(who, classe, povo, lvl)
	//sendsysmessage(who, "Procurando Habs!");
	var possiveis := array;

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var keys := getConfigStringKeys( habcfg );


	foreach key in keys
		var elem := FindConfigElem( habcfg, key );
		var c := GetConfigStringDictionary( elem, "classe" );
		//sendsysmessage(who, "classe" + CInt(c[classe]));
		var p := GetConfigStringDictionary( elem, "povo");
		//sendsysmessage(who, "povo" + CInt(p[povo]) + "PovoInP " + CInt(p.Exists(povo)));
		var requisito := GetConfigString(elem, "requisito");

		//sendsysmessage(who, "elem " + elem + " classe " + classe  + " c[1] " + c[1]+" c[2] " + c[2]+ " p[povo] " + p[povo] + "requisito:"+requisito);
		if ((c[classe]!=error) && (cint(c[classe]) == lvl) && ((p[povo]==error) || (p.Exists(povo))))// && (povo in p))
			//sendsysmessage(who, "classe" + CInt(c[classe]) + "povo" + CInt(p[povo]));
			if(lvl > 2) //Se o level for maior que 4, verifique requisito
				if(TemHabilidade(who,requisito))
					possiveis.append(key);
				endif
			else
				possiveis.append(key);
			endif
		endif
	endforeach
	return possiveis;

endfunction

function HabilidadesPorNivelFaccao(who, faccao, lvl)

	var possiveis := array;

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var keys := getConfigStringKeys( habcfg );

	foreach key in keys
		var elem := FindConfigElem( habcfg, key );
		var d := GetConfigStringDictionary( elem, "faccao" );
		if ( cint(d[faccao]) == lvl && d[faccao] != error)
			possiveis.append(key);
		endif
	endforeach

	return possiveis;

endfunction


function ComprarHabGump(who, habs, habsfac)

	UnloadConfigFile(":charactercreation:config/habilidades");

	var gump := GFCreateGump();

	var habsize := habs.size();
	var sizey := (habsize*100) + 120;
	var newsizey := (habsfac.size()*100) + 120; //facção
	if (newsizey > sizey)//facção
		sizey := newsizey;
	endif

	var newsizex := 0;
	if (habsfac.size() >= 1) //facção
		newsizex := 320;
	endif

	if (habsize > 10)
		newsizex := newsizex + 600;
		sizey := 620;
	elseif (habsize > 5)
		newsizex := newsizex + 300;
		sizey := 620;
	endif

    GFDisposable(gump, 0);
	GFResizePic(gump, 0, 25, 3600, 380+newsizex, sizey+30);
	GFResizePic(gump, 15, 40, 0x13BE, 350+newsizex, sizey);
	GFTextMid(gump, 80, 50, 250, 1153, "Habilidades de Classe");
	if (habsfac.size() >= 1) //facção
		GFTextMid(gump, 80+newsizex, 50, 250, 1153, "Habilidades de Faccao");
	endif

	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");

	var x := 40;
	var y := 80;

	var i := 1;
	foreach hab in habs
		var habelem := FindConfigElem(habcfg, hab);
		var desc := GetConfigString(habelem, "desc");
		var uso := GetConfigString(habelem, "uso");
		var nome := GetConfigString(habelem, "nome");
		if (uso == "Ativa")
			var delay := GetConfigInt(habelem, "delay");
			nome := nome + " ["+delay+" s]";
			var stam := GetConfigInt(habelem, "stam");
			if (!stam)
				stam := GetConfigInt(habelem, "mana");
				nome := nome + " [" + stam + " mana]";
			else
				nome := nome + " [" + stam + " stam]";
			endif
		endif
		var nivel := GetConfigInt(habelem, nivel);
		var requisitos := getConfigstringArray(habelem, "requisito");
		//var gumpid :=  Hex(GetConfigInt(habelem, "gump"));

		GFAddButton(gump, x-20, y+4, 2103, 2104, 1, i );
		//GFGumpPic(gump, x, y+20, gumpid) ;
		GFTextLine(gump, x, y, 1153, nome + " [" + uso + "]" );
		if (requisitos.size() )
			desc := desc + "<cr> Pre-Requisitos: ";
			foreach req in requisitos
				desc := desc + req + " ";
			endforeach
		endif
		GFHTMLArea( gump, x, y+20, 300, 90, desc, 1150, 0);

		i := i + 1;
		y := y + 110;
		if (i == 6)
			x := 355;
			y := 80;
		elseif (i == 11)
			x := 670;
			y := 80;
		endif
	endforeach

	y := 80;
	x := 355;
	if (habsize > 10)
		x := 985;
		y := 80;
	elseif (habsize > 5)
		x := 670;
		y := 80;
	endif
	i := 1;

	foreach hab in habsfac
		var habelem := FindConfigElem(habcfg, hab);
		var desc := GetConfigString(habelem, "desc");
		var uso := GetConfigString(habelem, "uso");
		var nome := GetConfigString(habelem, "nome");
		if (uso == "Ativa")
			var delay := GetConfigInt(habelem, "delay");
			nome := nome + " ["+delay+" s]";
			var stam := GetConfigInt(habelem, "stam");
			if (!stam)
				stam := GetConfigInt(habelem, "mana");
				nome := nome + " [" + stam + " mana]";
			else
				nome := nome + " [" + stam + " stam]";
			endif
		endif
		var nivel := GetConfigInt(habelem, nivel);
		var requisitos := getConfigstringArray(habelem, "requisito");
		//var gumpid :=  Hex(GetConfigInt(habelem, "gump"));

		GFAddButton(gump, x-20, y+4, 2103, 2104, 1, 20+i );
		//GFGumpPic(gump, x, y+20, gumpid) ;
		GFTextLine(gump, x, y, 1153, nome + " [" + uso + "]" );
		if (requisitos.size() )
			desc := desc + "<cr> Pre-Requisitos: ";
			foreach req in requisitos
				desc := desc + req + " ";
			endforeach
		endif
		GFHTMLArea( gump, x, y+20, 300, 90, desc, 1150, 0);

		i := i + 1;
		y := y + 110;
	endforeach

	var input := GFSendGump(who, gump);
	input := input[0];


	if (input > 0)
		if (input >= 20)
			input := input-20;
			return habsfac[input];
		endif
		return habs[input];
	else
		return 0;
	endif

endfunction

function comprarHabilidade(who, item, nv:=-1)
	var itemdesc := ReadConfigFile("::itemdesc");
	var nivel;

	if (nv != -1)
		nivel := nv;
	elseif (item)
		nivel := cint(GetObjProperty(item, "nivel"));
	else
		SendSysMessage(who,"Chamada inválida da função");
		return;
	endif
	var classe := GetObjProperty(who, "classe");
	var faccao := GetObjProperty(who, "faccao"); //facção
	var chardata := GetObjProperty(who, "chardata");
	var cultura := chardata.culture;
	var raca := chardata.raca;
	var religiao := chardata.religion;
	var magia := chardata.magia;
	var habs := GetObjProperty(who, "buyhab");
	if (habs[0] == error)
		habs[0] := 1;
		SetObjProperty(who, "buyhab", habs);
	endif
	var myhabs := GetObjProperty(who, "myhabs");
	if ( (myhabs == error)  )
		myhabs := dictionary;
	endif
	//SendSysMessage(who, GetObjProperty(who, "chardata").classe +" "+ GetObjProperty(who, "chardata").povo + " " + nivel);
	var possiveis := HabilidadesPorNivelPossiveis(who, chardata.classe, chardata.culture, nivel);

	//var possiveis := HabilidadesPorNivel(who, classe, nivel);
	var possiveisfaccao := HabilidadesPorNivelFaccao(who, faccao, nivel); //facção

	//sendsysmessage(who, "Procurando Habs possiveis!");

	nivel := nivel*2;

	if (habs[nivel] != 1)
		SendSysMessageEx(who, "Voce nao pode comprar habilidade desse nivel " + nivel , SSM_FAIL);
		return 0;
	endif

	if ( possiveis.size() > 0 || possiveisfaccao.size() > 0)
		//sendsysmessage(who, "Tem alguma hab!");

		SendSysMessageEx(who, "Escolha sua habilidade.", SSM_REQUEST);
		var escolhida := ComprarHabGump(who, possiveis, possiveisfaccao);
		if (!escolhida)
			SendSysMessageEx(who, "Voce cancelou a compra da habilidade.", SSM_FAIL);
			return 0;
		endif
		//var escolhida := RadioGump(who, 300, 380, "Escolha a habilidade que quer aprender", possiveis);
		if (!temRequisito(who, escolhida))
			return 0;
		endif

        if (escolhida == "Visao verdadeira" && who.gender != 1 && cultura != HAROLOTH && religiao != ELUITHORTH)
            SendSysMessageEx(who, "Apenas drows femeas podem comprar essa habilidade.", SSM_FAIL);			
            return 0;
        endif

		if (escolhida == "Aura Mortis" && religiao != VENERATIO && cultura != AIGLANA)
			SendSysMessageEx(who, "Apenas aiglanos Veneratio podem comprar essa habilidade.", SSM_FAIL);
			return 0;
		endif

		if(escolhida == "Elo Natural" && cultura == HAROLOTH)
            SendSysMessageEx(who, "Drows Har'oloth não podem ser druidas.", SSM_FAIL);
            return 0;
		endif

		if(escolhida == "Danca das Sombras" && cultura != HAROLOTH && religiao != ELUITHORTH)
			SendSysMessageEx(who, "Apenas drows machos Har'oloth e devotos de Idrith podem comprar essa habilidade.", SSM_FAIL);
			return 0;
		endif

		if(escolhida == "Gwaenedh" && magia != "Arqueiro Arcano")
			SendSysMessageEx(who, "Apenas arqueiros arcanos podem comprar essa habilidade.", SSM_FAIL);
			return 0;
		endif

		SendSysMessageEx(who, "Voce comprou a habilidade: " + escolhida, SSM_INFO);
		if (escolhida == "Caçador Implacável")
			var hitsmod := cint(GetObjProperty(who, "hitsmod"));
			hitsmod := hitsmod + 20;
			setObjProperty(who, "hitsmod", hitsmod);
			recalcvitals(who);
		elseif (escolhida == "Acuidade com Arma")
			SendSysMessageEx(who, "Escolha qual vai ser sua arma preferida.", SSM_REQUEST);
			var targ := Target(who);
			while(!targ.isA(POLCLASS_WEAPON))
				targ := Target(who);
			endwhile
			SetObjProperty(who, "armapreferida", targ.objtype);
			SendSysMessageEx(who, "Arma escolhida.", SSM_INFO);
		elseif (escolhida == "Arma Predileta")
			SendSysMessageEx(who, "Escolha qual vai ser sua arma predileta.", SSM_REQUEST);
			var targ := Target(who);
			while(!targ.isA(POLCLASS_WEAPON))
				targ := Target(who);
			endwhile
			SetObjProperty(who, "armapredileta", targ.objtype);
			SendSysMessageEx(who, "Arma escolhida.", SSM_INFO);
		elseif (escolhida == "Foco com Arma")
			SendSysMessageEx(who, "Escolha qual vai ser sua arma para a habilidade foco com arma.", SSM_REQUEST);
			var targ := Target(who);
			while(!targ.isA(POLCLASS_WEAPON))
				targ := Target(who);
			endwhile
			SetObjProperty(who, "fococomarma", targ.objtype);
			SendSysMessageEx(who, "Arma escolhida.", SSM_INFO);
		elseif (escolhida == "Inimigo Predileto")
			var inimigo := RadioGump(who, 300, 380, "Escolha seu Inimigo Predileto", array{"Dragon","Undead","Giants","GiantAnimal","EvilHuman","Goblin", "Underdark"});
			SetObjProperty(who, "InimigoPredileto", inimigo);
		elseif (escolhida == "Inventario Ferreiro")
			var inventario := RadioGump(who, 300, 380, "Selecione a especialidade", array{"Armas","Escudos","Armaduras"});
			Start_Script(":charactercreation:habilidades/inventario", {who, "ferreiro", inventario});
		elseif (escolhida == "Inventario Carpinteiro")
			var inventario := RadioGump(who, 300, 380, "Selecione a especialidade", array{"Equipamentos","Mobilia"});
			Start_Script(":charactercreation:habilidades/inventario", {who, "carpinteiro", inventario});
		elseif (escolhida == "Inventario Costureiro")
			var inventario := RadioGump(who, 300, 380, "Selecione a especialidade", array{"Couro","Tecido"});
			Start_Script(":charactercreation:habilidades/inventario", {who, "costureiro", inventario});
		elseif (escolhida == "Elo Natural")
			setObjProperty(who, "empatiacomanimais", 1);
		elseif(escolhida == "Protecao Elemental Kitahne")
			who.resist_fire_mod  := who.resist_fire_mod + 25;
			who.resist_cold_mod  := who.resist_cold_mod + 25;
			who.resist_poison_mod  := who.resist_poison_mod + 25;
			who.resist_energy_mod  := who.resist_energy_mod + 25;
		elseif(escolhida == "Aura Mortis" || escolhida == "Dominatio Animae" || escolhida == "Colheita de Almas" || escolhida == "Campeao das Montanhas")
            var equips := EnumeratePhysicalItems(who);
            //foreach para pegar todos os equips no corpo e remover as penalidades
            foreach equip in equips
                if ((equip.isa(POLCLASS_ARMOR)) && equip.resist_physical > 0)
                    //Quando comprar a habilidade de Aura de Corrupca ou Dominatio Animae, a penalidade eh removida
                    var penalty := CInt(itemdesc[equip.objtype].MageryPenalty);
                    AP_ModifySkillMod(who, MAGERY, (-1)*penalty);
                endif
            endforeach
        elseif(escolhida == "Artilheiro Preparado")
			if (chardata.wisdom_path != "Engenheiro")
				SendSysMessageEx(who, "Apenas engenheiros podem comprar essa habilidade.", SSM_FAIL);
				return 0;
			endif
            who.carrying_capacity_mod := 50;
        elseif(escolhida == "Bolsa Infinita")
            who.carrying_capacity_mod := 120;
		elseif(escolhida == "Ainalaith")
			var damage_effect := struct{};
			damage_effect.+fire_arrow := 0;
			damage_effect.+cold_arrow := 0;
			damage_effect.+energy_arrow := 0;
			SetObjProperty(who, "#shooting_effects", damage_effect);
		elseif (escolhida == "Tiro em Movimento")
			GrantPrivilege(who, "firewhilemoving");
			who.enable("firewhilemoving");	
		endif
		myhabs[nivel] := escolhida;
		habs[nivel] := 2;
		SetObjProperty(who, "buyhab", habs);
		SetObjProperty(who, "myhabs", myhabs);
		//return 1;
	else
		SendSysMessageEx(who, "Falta comprar a habilidade level 4 ou esse nivel ainda nao tem habilidades.", SSM_FAIL);
	endif
    RecalcVitals(who);
    if(item)
		DestroyItem(item);
	endif
endfunction


function AddNavigationBar(byref gump, current_page)
    var buttons := array{"Skills", "Habilidades"};
    
    for i := 1 to 2
        var x_offset := (i-1)*150 + 25;
        
        if (i == current_page)
            GFAddButton(gump, x_offset, 75, 2154, 2154, GF_CLOSE_BTN, i);
        else
            GFAddButton(gump, x_offset, 75, 2151, 2154, GF_CLOSE_BTN, i);
        endif
        GFTextLine(gump, x_offset + 35, 80, 2100, buttons[i]);
    endfor
endfunction

function ShowSkillsPage(byref gump, who)
    var chardata := GetObjProperty(who, "chardata");
    
    var x := 50;
    var y := 120;
    
    foreach skill in (chardata.Skills.keys())
        var valor_mod := "";
        if(AP_GetSkillMod(who, skill) > 0)
            valor_mod := " (+" + CStr(AP_GetSkillMod(who, skill)) + ")";
        elseif (AP_GetSkillMod(who, skill) < 0)
            valor_mod := " (" + CStr(AP_GetSkillMod(who, skill)) + ")";
        endif

        GFTextLine(gump, x, y, 1153, StrReplace(skill, "_", " ") + " [" + GetSkillDescValue(chardata.Skills[skill]) +"]" );
        GFTextLine(gump, x+145, y, GetSkillColor(AP_GetSKill(who, skill)), CStr(AP_GetLevelTitle(AP_GetSKill(who, skill))) + valor_mod);
        GFTextRight(gump, x+279, y, GetSkillColor(AP_GetSKill(who, skill)), CStr(AP_GetTrueSkill(who, skill)));

        if (AP_GetSkillScript(skill) != error)
            var elem := AP_GetAttributeCfgElem(skill);
            var id := elem.SkillId;
            //TODO Desativado para investigação do bug de usar skill a partir do GUMP
            //GFAddButton(gump, x - 20, y+4, 2103, 2104, 1, id);
        endif
        
        y := y + 15;
    endforeach

    ShowCapInfo(gump, who);
endfunction

function ShowHabilidadesPage(byref gump, who)
    var habs := GetObjProperty(who, "buyhab");
    if (!habs)
        habs := dictionary;
        var i := 0;
        for(i := 1; i <= 20; i := i + 1)
            habs[i] := 0;
        endfor
        SetObjProperty(who, "buyhab", habs);
    endif
    
    var myhabs := GetObjProperty(who, "myhabs");
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");

	var x := 30;
	var y := 120;
    var i;
    for(i := 0; i <= 20; i := i + 1)
        if (Cint(habs[i]) == 1)
            if (i%2 == 0)
                GFTextLine(gump, x + 20, y, 1150, "Habilidade: ");
            else
                GFTextLine(gump, x + 20, y, 1150, "Atributo: ");
            endif
            
            if (i%2 == 1)
                GFTextLine(gump, x + 90, y, 300, "Comprar Status");
                GFAddButton(gump, x, y+5, 2103, 2104, 1, i+1001);
            else
                if(i == 0)
                    GFTextLine(gump, x + 90, y, 300, "Escolha a Habilidade");
                    GFAddButton(gump, x, y+5, 2103, 2104, 1, i+1001);
                else
                    GFTextLine(gump, x + 90, y, 300, "Procure o Pergaminho");
                endif
            endif
        elseif (cint(habs[i]) == 2)
            if (i%2 == 0)
                GFTextLine(gump, x + 20, y, 1150, "Habilidade: ");
            else
                GFTextLine(gump, x + 20, y, 1150, "Atributo: ");
            endif
            if (i%2 == 1)
                GFTextLine(gump, x + 90, y, 300, "Status Comprado");
            else
                if (habcfg[myhabs[i]].uso == "Ativa")
                    var texto := myhabs[i] + " ["+habcfg[myhabs[i]].uso + "] " + "["+habcfg[myhabs[i]].duracao + "] " + "["+habcfg[myhabs[i]].delay + " s] ";
                    if (habcfg[myhabs[i]].stam != error)
                        texto := texto + "["+habcfg[myhabs[i]].stam + " stam] ";
                    else
                        texto := texto + "["+habcfg[myhabs[i]].mana + " mana] ";
                    endif
                    GFTextLine(gump, x + 90, y, 300, texto);
                else
                    GFTextLine(gump, x + 90, y, 300, myhabs[i] + " ["+habcfg[myhabs[i]].uso + "]");
                endif
            endif
        endif
        y += 20;
    endfor
endfunction

function ShowEmTreinamentoPage(byref gump, who)
    GFResizePic(gump, 15, 80, INTERNAL_BACKGROUND, GUMP_WIDTH - 30, GUMP_HEIGHT - 100);
    
    GFTextLine(gump, 30, 90, 300, "Perícias em Treinamento");
    
    var trainingSkills := GetObjProperty(who, "trainingSkills");
    if (trainingSkills == error)
        trainingSkills := array;
    endif
    
    var x := 60;
    var y := 120;
    var i;
    var selectedCount := 0;

    for (i := 1; i <= skillList.size(); i := i + 1)
        var skillAux := skillList[i];
        var elem := AP_GetAttributeCfgElem(skillAux);
        var displayName := FormatSkillName(skillAux);
        
        GFTextLine(gump, x, y, 1153, displayName);
        var check := 0;
        if (skillAux in trainingSkills)
            check := 1;
            selectedCount := selectedCount + 1;
        endif
        if (selectedCount < 3 || check)
            GFCheckBox(gump, x-20, y+3, 2360, 2361, check, i);
        else
            GFGumpPic(gump, x-20, y+3, 2360);
        endif
        y := y + 15;
        sleepms(2);
    endfor

    GFAddButton(gump, 300, 480, 4502, 4502, 1, 900);
    GFHTMLArea(gump, 240, 495, 100, 20, "<BASEFONT SIZE=5>Atualizar</BASEFONT>", 0, 0);
endfunction

function ShowCapInfo(byref gump, who)
    var chardata := GetObjProperty(who, "chardata");
    var primarias := getPrimarias(who);
    var pri := primarias[1];
    var cappri := Cdbl(AP_GetSkillCap(who)) + Cdbl(valorinicial(who, pri));
    var capprimax := 100;

    var secundarias := getSecundarias(who);
    var capsec := cappri - 20;
    var capsecmax := capprimax - 30;

    var terciarias := getTerciarias(who);
    var capter := capsec - 20;
    var captermax := (chardata.vantagemracial == "Sabedoria dos Antigos") ? 50 : 40;

    if(capsec > capsecmax)
        capsec := capsecmax;
    endif

    if(capter > captermax)
        capter := captermax;
    endif

    var y_pos := GUMP_HEIGHT - 60;
    GFTextMid(gump, 55, y_pos, 300, 53, "Cap/MaxCap:");
    GFTextLine(gump, 50, y_pos + 20, 300, "Pri.: " + cappri + "/" + capprimax);
    GFTextLine(gump, 160, y_pos + 20, 300, "Sec.: " + capsec + "/" + capsecmax);
    GFTextLine(gump, 270, y_pos + 20, 300, "Ter.: " + capter + "/" + captermax);
endfunction

function Habilidades(who, nivel)
	if (GetObjProperty(who, "#gumphab") != error)
		SetObjProperty(who, "#gumphab", 1);
		return;
	endif
	var myhabs := GetObjProperty(who, "myhabs");
	var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
	var habs := GetObjProperty(who, "buyhab");
	var chardata := GetObjProperty(who, "chardata");

	var gumpstatus := GFECreateGump("Comprar Status", 150, 160);
	var x := 30;
	var y := 80;
	GFTextLine(gumpstatus, x+20, y, 1153, "+2 Str");
	GFTextLine(gumpstatus, x+20, y+20, 1153, "+2 Dex");
	GFTextLine(gumpstatus, x+20, y+40, 1153, "+2 Int");

	GFAddButton(gumpstatus, x, y+3, 2117, 2118, GF_CLOSE_BTN, 10);
	GFAddButton(gumpstatus, x, y+23, 2117, 2118, GF_CLOSE_BTN, 20);
	GFAddButton(gumpstatus, x, y+43, 2117, 2118, GF_CLOSE_BTN, 30);
	var inputstatus := GFSendGump(who, gumpstatus);
	if (habs[nivel] == 1)
		if (inputstatus[0] == 10)
			habs[nivel] := 2;
			SetObjProperty(who, "buyhab", habs);
			AP_SetTrueStat(who, STRENGTH, AP_GetTrueStat(who, STRENGTH)+2);
		elseif (inputstatus[0] == 20)
			habs[nivel] := 2;
			SetObjProperty(who, "buyhab", habs);
			AP_SetTrueStat(who, DEXTERITY, AP_GetTrueStat(who, DEXTERITY)+2);
		elseif (inputstatus[0] == 30)
			habs[nivel] := 2;
			SetObjProperty(who, "buyhab", habs);
			AP_SetTrueStat(who, INTELLIGENCE, AP_GetTrueStat(who, INTELLIGENCE)+2);
		endif
	else
		SendSysMessageEx(who, "Voce nao pode comprar esse status.");
	endif

	EraseObjProperty(who, "#gumphab");
	return 1;
endfunction

function CheckPlayerLevel(who)
	var chardata := GetObjProperty(who, "chardata");
	var habs := GetObjProperty(who, "buyhab");
	var myhabs := GetObjProperty(who, "myhabs");
	var primSum := 0;
	var primarias := 0;

	if (!myhabs)
		myhabs := dictionary;
	endif

	if (!habs[0])
		habs[0] := 1;
	endif

	foreach skill in (chardata.Skills.keys())
		if (chardata.Skills[skill]  == 3)
			primarias := primarias + 1;
			primSum := primSum + AP_GetTrueSkill(who, skill); //Sobe de nivel com base apenas no valor base da skill
		endif
	endforeach
	primSum := primSum - primarias*40; //so conta os pontos que subiu a partir de 40
	primarias := primarias*3;
	primSum := cint(primSum/primarias);

	var i;
	for(i := 1; i <= primSum; i := i + 1)
		if (!habs[i])
			habs[i] := 1;
		endif
	endfor
	SetObjProperty(who, "buyhab", habs);
endfunction

function GetSkillColor(skill)
    var temp := cint(skill/10);
    skill := Cint(skill - temp*10);
    case (skill)
        0: return 338;
        1: return 343;
        2: return 348;
        3: return 353;
        4: return 358;
        5: return 363;
        6: return 368;
        7: return 378;
        8: return 393;
        9: return 398;
    endcase
endfunction


function SelecttrainingSkills(who)
    var charGump := GFCreateGump();
    var input;

    GFClosable(charGump, 1);
    GFDisposable(charGump, 0);
    GFResizePic(charGump, 0, 25, 3600, 400, 430);
    GFResizePic(charGump, 15, 40, 0x13BE, 370, 400);
    GFTextMid(charGump, 20, 50, 300, 1153, "Escolha ate 3 pericias para treinamento.");

    var trainingSkills := GetObjProperty(who, "trainingSkills");
    if (trainingSkills == error)
        trainingSkills := array;
    endif

    var x := 60;
    var y := 100;
    var i;
    GFSetRadioGroup(charGump, 1);

    for (i := 1; i <= skillList.size(); i := i + 1)
        var skillAux := skillList[i];
        var elem := AP_GetAttributeCfgElem(skillAux);
        var stat := GetConfigString(elem, "Stat");
        if (stat == error)
            stat := "---";
        endif
        var nstat := stat[1]+stat[2]+stat[3];
        GFTextLine(charGump, x, y, 1153, skillList[i] + " [" + lower(nstat)+"]");
        var check := 0;
        if (skillAux in trainingSkills)
            check := 1;
        endif
        GFCheckBox(charGump, x-20, y+3, 2360, 2361, check, i);
        y := y + 15;
        if ( i == 21 )
            x := x + 140;
            y := 100;
        endif
        sleepms(2);
    endfor

    GFAddButton(charGump, 310, 400, 4502, 4502, 1, 100 );
    GFTextLine(charGump, 270, 410, 1153, "Pronto");
    input := GFSendGump(who, charGump);
    var selection := array;
    if (input[0] == 100)
        foreach key in (input.keys)
            if ( (key != 100) && (key != 0) )
                selection.append(key);
            endif
        endforeach
    else
        SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
        return;
    endif
    if (selection.size() > 3)
        SendSysMessageEx(who, "Voce pode escolher no maximo três pericias.", SSM_FAIL);
        return;
    endif
    trainingSkills := array;
    foreach selected in selection
        var selectedSkill := skillList[selected];
        trainingSkills.append(selectedSkill);
    endforeach

    if (trainingSkills[1])
        SetObjProperty(who, "trainingSkills", trainingSkills);
    else
        EraseObjProperty(who, "trainingSkills");
    endif
    SendSysMessageEx(who, "Suas pericias em treinamento foram atualizadas com sucesso.", SSM_INFO);
endfunction


function SkillEraser(who)
    SendSysMessageEx(who, "Para interromper a descida de uma pericia escolhida anteriormente, selecione e coloque 0.", SSM_INFO);

    var charGump := GFCreateGump();
    var input;

    GFClosable(charGump, 1);
    GFDisposable(charGump, 0);
    GFResizePic(charGump, 0, 25, 3600, 400, 430);
    GFResizePic(charGump, 15, 40, 0x13BE, 370, 400);
    GFTextMid(charGump, 20, 50, 300, 1153, "Escolha as pericias para esquecer.");

    var x := 60;
    var y := 100;
    var i;
    GFSetRadioGroup(charGump, 1);

    var skillsToDelete := GetObjProperty(who, "skillsToDelete");
    if (skillsToDelete == error)
        skillsToDelete := dictionary;
    endif

    for (  i :=  1; i <= skillList.size(); i := i + 1)
        var skillAux := skillList[i];
        if (skillAux == "ItemLore")
            skillAux := "ArmsLore";
        elseif ( skillAux == "Survival")
            skillAux := "Camping";
        elseif (skillAux == "Arcana")
            skillAux := "Inscription";
        endif
        var elem := AP_GetAttributeCfgElem(skillAux);
        var stat := GetConfigString(elem, "Stat");
        if (stat == error)
            stat := "---";
        endif
        var nstat := stat[1]+stat[2]+stat[3];
        var color := 1153;
        var note := "";
        if (skillsToDelete[skillAux])
            note := " -" + cdbl(skillsToDelete[skillAux]);
            color := 1608;
        endif
        GFTextLine(charGump, x, y, color, skillList[i] +  "" + note );
        var check := 0;
        if (skillAux in skillsToDelete.keys())
            check := 1;
        endif
        GFCheckBox(charGump, x-20, y+3, 2360, 2361, check, i);

        y := y + 15;

        if ( i == 21 )
            x := x + 140;
            y := 100;
        endif
        sleepms(2);
    endfor

    GFAddButton(charGump, 310, 400, 4502, 4502, 1, 100 );
    GFTextLine(charGump, 270, 410, 1153, "Pronto");

    input := GFSendGump(who, charGump);
    var selection := array;
    if (input[0] == 100)
        foreach key in (input.keys)
            if ( (key != 100) && (key != 0) )
                selection.append(key);
            endif
        endforeach
    else
        SendSysMessageEx(who, "Cancelado.", SSM_FAIL);
        return;
    endif

    foreach selected in selection
        var selectedSkill := skillList[selected];
        if (selectedSkill == "Survival")
            selectedSkill := "Camping";
        elseif (selectedSkill == "ItemLore")
            selectedSkill := "ArmsLore";
        elseif ( selectedSkill == "EvidanceGathering")
            selectedSkill := "EvaluatingIntelligence";
        elseif ( selectedSkill == "Arcana")
            selectedSkill := "Inscription";
        endif
        var value := cdbl(RequestGump(who, "Quantos pontos vai abaixar da pericia " + selectedSkill + " ? " ));
        skillsToDelete[selectedSkill] := value;
    endforeach

    SetObjProperty(who, "skillsToDelete", skillsToDelete);
    SendSysMessageEx(who, "Suas pericias selecionadas serao esquecidas com o tempo.", SSM_INFO);

endfunction
