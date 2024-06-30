use cfgfile;

include ":attributes:attributes";

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

	foreach req in (requisitos)
		if (req["kit"]) // Ver se tem Kit!
			req["kit "] := "";
			if ((!GetObjProperty(who, "chardata")).trait in (req))
				if (msg == 1)
					SendSysMessageEx(who, "Voce nao possui a Caracteristica: " + req, SSM_FAIL);
				endif
				return 0;
			endif
        elseif (req["povo"]) // Adiciona busca por povos
            req["povo"] := "";
            if ((!GetObjProperty(who, "chardata")).povo in (req))
				if (msg == 1)
					SendSysMessageEx(who, "Voce nao pertence ao povo: " + req, SSM_FAIL);
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
		if (Lower(habs[key]) == Lower(habilidade))
			return 1;
		endif

	endforeach


	return 0;

endfunction

function IsBloodied(mobile)
	if (mobile.npctemplate)
		if (AP_GetVital(mobile, HITS) < CInt(AP_GetVitalMaximumValue(mobile, HITS)/2))
			return 1;
		endif
	elseif (AP_GetVital(mobile, HITS) < GetHP(mobile))
		return 1;
	endif
	return 0;
endfunction

function IsRaging(mobile)
	//Bonus da habilidde baixo limiar de furia
	var bonus_limiar_furia_hp := 0;
	if (TemHabilidade(mobile, "Baixo Limiar de Furia"))
		bonus_limiar_furia_hp := bonus_limiar_furia_hp + 100;
	endif

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

function EnumeratePhysicalItemsInMobile(mobile)
	var EquipList := array;

	var SkipLayers := {11, 16, 21, 25};
	var i;

	for (i:=0; i<=25; i:=i+1)
		if (!(i in SkipLayers))
			var layer := GetEquipmentByLayer(mobile, i);
			if (layer)
				EquipList.append(layer);
			endif
		endif
	endfor

	return EquipList;
endfunction

function comprarHabilidade(who, item, nv:=-1)
	//SendSysMessage(who,"[Interno] Tentando comprar Hab Level "+nv);
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
	var possiveis := HabilidadesPorNivelPossiveis(who, GetObjProperty(who, "chardata").classe, GetObjProperty(who, "chardata").povo, nivel);

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
        if (escolhida == "Visao verdadeira" && who.graphic != 1831 && GetObjProperty(who, "chardata").povo != "Drow")
            SendSysMessageEx(who, "Apenas drows femeas podem comprar essa habilidade.", SSM_FAIL);			
            return 0;
        endif

		SendSysMessageEx(who, "Voce comprou a habilidade: " + escolhida, SSM_INFO);
		if (escolhida == "Grande Fortitude")
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
		elseif (escolhida == "Senso da Natureza")
			setObjProperty(who, "empatiacomanimais", 1);
		elseif(escolhida == "Protecao Elemental Kitahne")
			who.resist_fire_mod  := who.resist_fire_mod + 25;
			who.resist_cold_mod  := who.resist_cold_mod + 25;
			who.resist_poison_mod  := who.resist_poison_mod + 25;
			who.resist_energy_mod  := who.resist_energy_mod + 25;
		elseif(escolhida == "Senhor do Medo" || escolhida == "Devocao" || escolhida == "Colheita de Almas" || escolhida == "Campeao das Montanhas")
            var equips := EnumeratePhysicalItemsInMobile(who);
            //foreach para pegar todos os equips no corpo e remover as penalidades
            foreach equip in equips
                if ((equip.isa(POLCLASS_ARMOR)) && equip.resist_physical > 0)
                    //Quando comprar a habilidade de Aura de Corrupca ou Devocao, a penalidade eh removida
                    var penalty := CInt(itemdesc[equip.objtype].MageryPenalty);
                    AP_ModifySkillMod(who, MAGERY, (-1)*penalty);
                endif
            endforeach
        elseif(escolhida == "Artilheiro Preparado")
            who.carrying_capacity_mod := 50;
        elseif(escolhida == "Bolsa Infinita")
            who.carrying_capacity_mod := 120;
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
