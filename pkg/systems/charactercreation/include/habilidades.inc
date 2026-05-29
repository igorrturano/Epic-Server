use cfgfile;

include ":attributes:attributes";
include "include/epicConstants";
include "include/epicUtils";
include ":tn:equipment";
include ":abilities:include/models/Ability";
include ":census:include/CensusController";
include ":wrappers:Objects/Model/MobileWrapper";
include "/include/math";
include ":fls_core:fls_characters";
function NivelHabilidade(habilidade, classe)

	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
	var elem := FindConfigElem( habcfg, habilidade );

	var nivel := GetConfigStringDictionary( elem, "classe" );

	return nivel[classe];

endfunction

function TemRequisito(who, habilidade, msg := 1)

	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
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

	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
	var elem := FindConfigElem( habcfg, habilidade );

	var requisitos := GetConfigStringArray(elem, "requisito");

	return requisitos;

endfunction

function TemHabilidade(who, habilidade)
	var habs := GetObjProperty(who, "myhabs");
	if (!habs)
		return 0;
	endif

	// myhabs now stores IDs - check if habilidade is in the values
	var hab_id := CInt(habilidade);
	if (hab_id)
		// habilidade is an ID (number)
		foreach key in (habs.keys())
			if (CInt(habs[key]) == hab_id)
				return 1;
			endif
		endforeach
	else
		// habilidade is a name (string) - need to find the ID first
		var habcfg := ReadConfigFile(":abilities:configs/habilidades");
		if (!habcfg)
			return 0;
		endif

		// Find ability ID by name
		foreach cfg_key in (GetConfigStringKeys(habcfg))
			var elem := FindConfigElem(habcfg, cfg_key);
			var nome := GetConfigString(elem, "nome");
			if (RemoveAcentos(Lower(nome)) == RemoveAcentos(Lower(habilidade)))
				hab_id := CInt(cfg_key);
				break;
			endif
		endforeach

		// Now check if that ID is in myhabs
		if (hab_id)
			foreach key in (habs.keys())
				if (CInt(habs[key]) == hab_id)
					return 1;
				endif
			endforeach
		endif
	endif

	return 0;
endfunction

function IsBloodied(mobile)
	if (mobile.npctemplate)
		if (AP_GetVital(mobile, HITS) < CInt(AP_GetVitalMaximumValue(mobile, HITS)/4))
			return 1;
		endif
	elseif (AP_GetVital(mobile, HITS) < (GetHP(mobile) * 0.25))
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

	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
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

// GetAbilitySubclasse maps an ability ID (from habilidades.cfg key) to the
// path-rework subclass(es) it belongs to. Encoded via the ID range — each
// class occupies a 100-block of IDs (Arqueiro 1-5, Cacador 100-104,
// Combatente 200-204, etc.). Bardo is the only non-block-aligned class
// (1708-1712), but 1708/100 still resolves cleanly to 17.
//
// Returns an ARRAY of subclass names (set semantics). Most trees belong to
// exactly one subclass (single-element array). A shared tree — one ability
// line feeding two class fantasies — returns multiple subclasses; the
// 1800-1804 shadow-dance tree is buyable by both the Ladino rogue
// (Dancarino_das_Sombras) and the Sabio Velkyn melee-caster
// (Deslocador_das_Sombras). An empty array means cross-class (Caminho) /
// unknown, which falls through the subclass filter unrestricted.
function GetAbilitySubclasse(hab_id)
	var id := CInt(hab_id);
	case (id / 100)
		0:  return array{ "Arqueiro" };
		1:  return array{ "Cacador" };
		2:  return array{ "Combatente" };
		3:  return array{ "Defensor" };
		4:  return array{ "Paladino" };
		5:  return array{ "Aklattur" };
		6:  return array{ "Barbaro" };
		7:  return array{ "Monge" };
		8:  return array{ "Samurai" };
		9:  return array{ "Mutante" };
		10: return array{ "Malandro_Arcano" };
		11: return array{ "Cacador_de_Tesouros" };
		12: return array{ "Assassino" };
		13: return array{ "Shinobi" };
		14: return array{ "Vivisseccionista" };
		15: return array{ "Esgrimista" };
		16: return array{ "Ladrao" };
		17: return array{ "Bardo" };
		18: return array{ "Dancarino_das_Sombras", "Deslocador_das_Sombras" };
		19: return array{ "Cacador_de_Demonios" };
		20: return array{ "Necromante" };
		21: return array{ "Druida" };
		22: return array{ "Magus" };
		23: return array{ "Lamina_Arcana" };
		24: return array{ "Arqueiro_Arcano" };
		25: return array{ "Granadeiro" };
		26: return array{ "Shaman" };
		27: return array{ "Engenheiro" };
		28: return array{ "Cleric" };
	endcase
	return array{};
endfunction

function HabilidadesPorNivelPossiveis(who, classe, povo, lvl)
	//sendsysmessage(who, "Procurando Habs!");
	var possiveis := array;

	// Path-rework: narrow class abilities to the player's specific subclass.
	// Legacy chars (no chardata.subclasse) fall back to the archetype-wide
	// list as before.
	var chardata := GetObjProperty(who, "chardata");
	var subclasse := "";
	if (chardata != error && chardata)
		subclasse := chardata.subclasse;
		if (subclasse == error)
			subclasse := "";
		endif
	endif

	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
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
			// Subclass narrowing: skip abilities that belong to a different
			// subclass within the same archetype. Caminho/cross-class entries
			// return an empty set and pass through. Shared trees (e.g.
			// 1800-1804 shadow-dance) list multiple subclasses; the player
			// passes if theirs is in the set.
			if (subclasse != "")
				var hab_subclasses := GetAbilitySubclasse(key);
				if (hab_subclasses.size() > 0 && !(subclasse in hab_subclasses))
					continue;
				endif
			endif

			//sendsysmessage(who, "classe" + CInt(c[classe]) + "povo" + CInt(p[povo]));
			if(lvl > 2) //Se o level for maior que 4, verifique requisito
				if(TemHabilidade(who,requisito))
					possiveis.append(key);
				endif
			else
				possiveis.append(key);
			endif
		elseif (c.Exists("Caminho"))
			// Caminho abilities available to all classes at levels 4,8,12,16,18 (nivel 2,4,6,8,9)
			var caminhoLevels := array{2, 4, 6, 8, 9};
			if (lvl in caminhoLevels)
				var isPool := CInt(GetConfigInt(elem, "pool"));
				if (isPool || !TemHabilidade(who, key))
					possiveis.append(key);
				endif
			endif
		endif
	endforeach
	return possiveis;

endfunction



function ComprarHabGump(who, habs)

	UnloadConfigFile(":abilities:configs/habilidades");
	var habcfg := ReadConfigFile(":abilities:configs/habilidades");

	var ITEMS_PER_PAGE := 4;
	var totalPages := CInt((habs.size() + ITEMS_PER_PAGE - 1) / ITEMS_PER_PAGE);
	if (totalPages < 1)
		totalPages := 1;
	endif

	var GUMP_W := 460;
	var GUMP_H := 570;
	var CARD_H := 115;
	var CARD_START_Y := 60;
	var NAV_Y := GUMP_H - 45;

	var gump := GFCreateGump();
	GFDisposable(gump, 0);

	// Page 0 - static background (always visible)
	GFPage(gump, 0);
	GFResizePic(gump, 0, 0, 3600, GUMP_W, GUMP_H);
	GFResizePic(gump, 15, 15, 0x13BE, GUMP_W - 30, GUMP_H - 30);
	GFTextMid(gump, 15, 25, GUMP_W - 30, 1153, "Habilidades Disponíveis");

	// Build paginated content
	var pageNum := 1;
	var itemOnPage := 0;
	GFPage(gump, pageNum);

	var i := 1;
	foreach hab in habs
		if (itemOnPage >= ITEMS_PER_PAGE)
			// Next arrow on current page
			GFAddButton(gump, GUMP_W - 70, NAV_Y, 5601, 5605, GF_PAGE_BTN, pageNum + 1);
			GFTextLine(gump, GUMP_W - 115, NAV_Y + 2, 1153, "Próxima");
			GFTextMid(gump, 15, NAV_Y + 2, GUMP_W - 30, 1153, $"Pág. {pageNum}/{totalPages}");

			pageNum := pageNum + 1;
			GFPage(gump, pageNum);

			// Prev arrow on new page
			GFAddButton(gump, 30, NAV_Y, 5603, 5607, GF_PAGE_BTN, pageNum - 1);
			GFTextLine(gump, 60, NAV_Y + 2, 1153, "Anterior");

			itemOnPage := 0;
		endif

		var habelem := FindConfigElem(habcfg, hab);
		var desc := GetConfigString(habelem, "desc");
		var uso := GetConfigString(habelem, "uso");
		var nome := GetConfigString(habelem, "nome");
		var icon := GetConfigInt(habelem, "icon");
		if (!icon || uso == "Passiva")
			icon := 2973;
		endif

		if (uso == "Ativa")
			var delay := GetConfigInt(habelem, "delay");
			nome := nome + " [" + delay + "s]";
			var stam := GetConfigInt(habelem, "stam");
			if (!stam)
				stam := GetConfigInt(habelem, "mana");
				nome := nome + " [" + stam + " mana]";
			else
				nome := nome + " [" + stam + " stam]";
			endif
		endif

		var requisitos := GetConfigStringArray(habelem, "requisito");
		if (requisitos.size())
			desc := desc + "<cr>Requisitos: ";
			foreach req in requisitos
				desc := desc + req + " ";
			endforeach
		endif

		var y := CARD_START_Y + (itemOnPage * CARD_H);
		var textX := 35;

		// Icon (if available)
		if (icon)
			GFGumpPic(gump, 30, y + 8, icon);
			textX := 85;
		endif

		// Name + type
		GFTextLine(gump, textX, y, 1153, nome + " [" + uso + "]");

		// Description
		GFHTMLArea(gump, textX, y + 20, GUMP_W - textX - 80, 75, desc, 1150, 0);

		// Buy button
		GFAddButton(gump, GUMP_W - 60, y + 30, 4005, 4007, 1, i);
		GFTextLine(gump, GUMP_W - 62, y + 12, 1153, "Obter");

		itemOnPage := itemOnPage + 1;
		i := i + 1;
	endforeach

	// Navigation for last page
	if (totalPages > 1)
		GFTextMid(gump, 15, NAV_Y + 2, GUMP_W - 30, 1153, $"Pág. {pageNum}/{totalPages}");
		if (pageNum > 1)
			GFAddButton(gump, 30, NAV_Y, 5603, 5607, GF_PAGE_BTN, pageNum - 1);
			GFTextLine(gump, 60, NAV_Y + 2, 1153, "Anterior");
		endif
	endif

	var input := GFSendGump(who, gump);
	input := input[0];

	if (input > 0)
		return habs[input];
	else
		return 0;
	endif

endfunction

function comprarHabilidade(who, item, nv:=-1)
	var nivel;

	if (nv != -1)
		nivel := nv;
	elseif (item)
		nivel := cint(GetObjProperty(item, "nivel"));
	else
		SendSysMessage(who,"Chamada inválida da função");
		return;
	endif
	var chardata := GetObjProperty(who, "chardata");
	var cultura := chardata.culture;
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
	var possiveis := HabilidadesPorNivelPossiveis(who, chardata.classe, chardata.culture, nivel);

	nivel := nivel*2;

	// True Mode gates ability tiers purely by character Level — even-level
	// abilities arrive as the pergaminho scroll (granted by LevelUp), and
	// odd-level stat slots are opened by character Level in
	// SetPlayerSkillLevel / RepairMigratedBuyhab. The skill-distribution
	// gate stays for Story Mode characters, who progress on the old
	// "accumulate enough primary skill points" curve.
	if (chardata.game_mode != "True Mode")
		var diference := Trunc(GetBuyHabDiference(who, nivel),1);

		if (diference > 0)
			SendSysMessageEx(who, "Você não pode comprar habilidade desse nivel, faltam {} pontos de skill distribuídas nas suas primárias. ".format(diference), SSM_FAIL);
			return 0;
		endif
	endif

	if ( possiveis.size() > 0 )
		SendSysMessageEx(who, "Escolha sua habilidade.", SSM_REQUEST);
		var escolhida := ComprarHabGump(who, possiveis);
		if (!escolhida)
			SendSysMessageEx(who, "Voce cancelou a compra da habilidade.", SSM_FAIL);
			return 0;
		endif
		if (!temRequisito(who, escolhida))
			return 0;
		endif

		// escolhida is now an ability ID - convert to int for comparisons
		var escolhida_id := CInt(escolhida);

		if (escolhida_id == ABILITY_ID::ELO_NATURAL && cultura == HAROLOTH)
			SendSysMessageEx(who, "Drows Har'oloth não podem ser druidas.", SSM_FAIL);
			return 0;
		endif

		if (escolhida_id == ABILITY_ID::DANCA_DAS_SOMBRAS && cultura != HAROLOTH && religiao != ELUITHORTH)
			SendSysMessageEx(who, "Apenas drows machos Har'oloth e devotos de Idrith podem comprar essa habilidade.", SSM_FAIL);
			return 0;
		endif

		if (escolhida_id == ABILITY_ID::ARCANE_BOW && magia != "arcane_archer")
			SendSysMessageEx(who, "Apenas arqueiros arcanos podem comprar essa habilidade.", SSM_FAIL);
			return 0;
		endif

		// Get ability name from config for display message
		var habcfg := ReadConfigFile(":abilities:configs/habilidades");
		var habelem_display := FindConfigElem(habcfg, escolhida);
		var ability_name := GetConfigString(habelem_display, "nome");
		SendSysMessageEx(who, "Voce comprou a habilidade: " + ability_name, SSM_INFO);

		if (escolhida_id == ABILITY_ID::CACADOR_IMPLACAVEL || escolhida_id == ABILITY_ID::RESISTENCIA_DO_DESERTO)
			var hitsmod := cint(GetObjProperty(who, "hitsmod"));
			hitsmod := hitsmod + 20;
			setObjProperty(who, "hitsmod", hitsmod);
			recalcvitals(who);
		elseif (escolhida_id == ABILITY_ID::ELO_NATURAL)
			setObjProperty(who, "empatiacomanimais", 1);
		elseif (escolhida_id == ABILITY_ID::EXPLOSIVE_FIST)
			if (chardata.subclasse != "Engenheiro" && chardata.wisdom_path != "Engenheiro")
				SendSysMessageEx(who, "Apenas engenheiros podem comprar essa habilidade.", SSM_FAIL);
				return 0;
			endif
		elseif (escolhida_id == ABILITY_ID::ELEMENTAL_AFFINITY)
			var damage_effect := struct{};
			damage_effect.+fire_arrow := 0;
			damage_effect.+cold_arrow := 0;
			damage_effect.+energy_arrow := 0;
			SetObjProperty(who, "#shooting_effects", damage_effect);
		elseif (escolhida_id == ABILITY_ID::TIRO_EM_MOVIMENTO)
			GrantPrivilege(who, "firewhilemoving");
			who.enable("firewhilemoving");
		elseif (escolhida_id == ABILITY_ID::NINJA_PATH)
			CreateItemInBackPack(who, 3592, 1);
		elseif (escolhida_id == ABILITY_ID::CAMPEAO_DIVINO || escolhida_id == ABILITY_ID::FORMA_ABISSAL)
			var wing_type := ChooseWings(who);
			if (wing_type)
				SetObjProperty(who, "chosen_wings", wing_type);
			endif
		elseif(escolhida_id == ABILITY_ID::AURA_DE_FE)
			ChooseFlag(who);
		endif
		myhabs[nivel] := escolhida_id;
		habs[nivel] := 2;
		SetObjProperty(who, "buyhab", habs);
		SetObjProperty(who, "myhabs", myhabs);

		// Carry-weight abilities are recomputed from scratch (single source of
		// truth in MobileWrapper) AFTER myhabs is persisted so HasAbility sees
		// the just-bought ability. Replaces the old in-place += 60 / := 120.
		if (escolhida_id == ABILITY_ID::PACK_MULE || escolhida_id == ABILITY_ID::BOLSA_INFINITA)
			MobileWrapper(who).RecalcCarryWeightMod();
		endif

		if (who.cmdlevel == 0)
			CensusTrackAbility(escolhida_id);
		endif

		// Update pool counter for pool abilities (e.g., weapon specializations)
		if (CInt(GetConfigInt(habelem_display, "pool")))
			var poolCounts := GetObjProperty(who, "myhabs_pool") ?: dictionary;
			poolCounts[escolhida_id] := CInt(poolCounts[escolhida_id]) + 1;
			SetObjProperty(who, "myhabs_pool", poolCounts);
		endif

		if(item)
			DestroyItem(item);
		endif
	else
		SendSysMessageEx(who, "Falta comprar a habilidade level 4 ou esse nivel ainda nao tem habilidades.", SSM_FAIL);
		return 0;
	endif
    RecalcVitals(who);

endfunction


function AddNavigationBar(byref gump, current_page)
    var buttons := array{"Skills", "Habilidades"};
    
    for i := 1 to buttons.size()
        var x_offset := (i-1)*150 + 25;
        
        if (i == current_page)
            GFAddButton(gump, x_offset, 75, 2154, 2154, GF_CLOSE_BTN, i);
        else
            GFAddButton(gump, x_offset, 75, 2151, 2154, GF_CLOSE_BTN, i);
        endif
        GFTextLine(gump, x_offset + 35, 80, 2100, buttons[i]);
    endfor
endfunction

// Builds skill -> path lookup dict ("Militar" / "Civil"). Includes the new
// path-rework skill names (Stealth, Smithing, etc.) plus legacy names mapped
// to their closest path equivalent so old characters' skill lists still
// group cleanly.
function BuildSkillPathLookup()
    var lookup := dictionary;
    var s;

    var mil := MILITARY_SKILLS;
    foreach s in mil
        lookup[s] := "Militar";
    endforeach

    var civ := CIVIC_SKILLS;
    foreach s in civ
        lookup[s] := "Civil";
    endforeach

    // Legacy skill names (kept as parallel attributes until Phase 6 cleanup).
    lookup["Sneak"]         := "Militar";
    lookup["Handle_Animal"] := "Militar";
    lookup["Throwing"]      := "Militar";
    lookup["Pickpocketing"] := "Civil";
    lookup["Metal_Working"] := "Civil";
    lookup["Wood_Working"]  := "Civil";
    lookup["Awareness"]     := "Civil";
    lookup["Domestic_Work"] := "Civil";
    lookup["Gathering"]     := "Civil";

    return lookup;
endfunction

function ShortTierLabel(tier)
    if (tier == 3)
        return "Pri";
    elseif (tier == 2)
        return "Sec";
    elseif (tier == 1)
        return "Ter";
    endif
    return "";
endfunction

function RenderSkillRow(byref gump, who, byref chardata, skill, x, y)
    var valor_mod := "";
    if (AP_GetSkillMod(who, skill) > 0)
        valor_mod := " (+" + CStr(AP_GetSkillMod(who, skill)) + ")";
    elseif (AP_GetSkillMod(who, skill) < 0)
        valor_mod := " (" + CStr(AP_GetSkillMod(who, skill)) + ")";
    endif

    var tier_text := ShortTierLabel(chardata.Skills[skill]);
    var tier_suffix := "";
    if (tier_text != "")
        tier_suffix := " [" + tier_text + "]";
    endif

    GFTextLine(gump, x, y, 1153, StrReplace(skill, "_", " ") + tier_suffix);
    GFTextLine(gump, x+145, y, GetSkillColor(AP_GetSKill(who, skill)), CStr(AP_GetLevelTitle(AP_GetSKill(who, skill))) + valor_mod);
    GFTextRight(gump, x+279, y, GetSkillColor(AP_GetSKill(who, skill)), CStr(AP_GetTrueSkill(who, skill)));
endfunction

function ShowSkillsPage(byref gump, who)
    var chardata := GetObjProperty(who, "chardata");

    var x := 50;
    var y := 120;

    var path_lookup := BuildSkillPathLookup();

    var mil_owned   := array;
    var civ_owned   := array;
    var other_owned := array;
    var skill_keys  := chardata.Skills.keys();

    // Iterate tier-descending (Pri 3 → Sec 2 → Ter 1) so primaries appear
    // first within each section, secondaries below, tertiaries last.
    var tier;
    for (tier := 3; tier >= 1; tier := tier - 1)
        foreach skill in skill_keys
            if (chardata.Skills[skill] != tier)
                continue;
            endif
            var p := path_lookup[skill];
            if (p == "Militar")
                mil_owned.append(skill);
            elseif (p == "Civil")
                civ_owned.append(skill);
            else
                other_owned.append(skill);
            endif
        endforeach
    endfor

    if (mil_owned.size() > 0)
        GFTextLine(gump, x, y, 33, "Militar");
        y := y + 16;
        foreach skill in mil_owned
            RenderSkillRow(gump, who, chardata, skill, x, y);
            y := y + 15;
        endforeach
        y := y + 6;
    endif

    if (civ_owned.size() > 0)
        GFTextLine(gump, x, y, 33, "Civil");
        y := y + 16;
        foreach skill in civ_owned
            RenderSkillRow(gump, who, chardata, skill, x, y);
            y := y + 15;
        endforeach
        y := y + 6;
    endif

    if (other_owned.size() > 0)
        GFTextLine(gump, x, y, 33, "Outras");
        y := y + 16;
        foreach skill in other_owned
            RenderSkillRow(gump, who, chardata, skill, x, y);
            y := y + 15;
        endforeach
    endif

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
    var habcfg := ReadConfigFile(":abilities:configs/habilidades");

	var x := 30;
	var y := 120;
    var i;
    for(i := 1; i <= 20; i := i + 1)
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
                var hab_id := myhabs[i];
                var hab_elem := habcfg[hab_id];
                var hab_nome := hab_elem.nome;
                if (hab_elem.uso == "Ativa")
                    var texto := hab_nome + " ["+hab_elem.uso + "] " + "["+hab_elem.delay + " s] ";
                    if (hab_elem.stam != error)
                        texto := texto + "["+hab_elem.stam + " stam] ";
                    else
                        texto := texto + "["+hab_elem.mana + " mana] ";
                    endif
                    GFTextLine(gump, x + 90, y, 300, texto);
                else
                    GFTextLine(gump, x + 90, y, 300, hab_nome + " ["+hab_elem.uso + "]");
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
    var cappri, capsec, capter, capprimax, capsecmax, captermax;

    var primarias := getPrimarias(who);
    var pri := primarias[1];
    cappri := Cdbl(AP_GetSkillCap(who)) + Cdbl(valorinicial(who, pri));
    capprimax := 100;
    capsec := cappri - 20;
    capsecmax := capprimax - 30;
    capter := capsec - 20;
    captermax := (chardata.vantagemracial == "Sabedoria dos Antigos") ? 50 : 40;
    if (capsec > capsecmax)
        capsec := capsecmax;
    endif
    if (capter > captermax)
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
	var habcfg := ReadConfigFile(":abilities:configs/habilidades");
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

function SetPlayerSkillLevel(who)

	var habs := GetObjProperty(who, "buyhab");
	if(!habs)
		habs[0] := 1;
	endif

	var levelInfo := GetSkillLevelInfo(who);

	for level := 1 to levelInfo.currentLevel
		if (!habs[level])
			habs[level] := 1;
		endif
	endfor

	SetObjProperty(who, "buyhab", habs);
endfunction

function GetSkillLevelInfo(who)
	var characterPrimarySkills := GetCharacterSkills(who, SKILL_PRIORITY::PRIMARY);
	var periciaCombinadaAcima := characterPrimarySkills.combinedSkillsValue - characterPrimarySkills.skills.Size() * 40; //so conta os pontos que subiu a partir de 40

	var chardata := GetObjProperty(who, "chardata");
	if (chardata != error && chardata.trait == "Sabe-Tudo")
		periciaCombinadaAcima += 3; // Sabe-Tudo sobe de nivel com 43 (em vez de 46) na sua unica primaria
	endif

	var divisor := characterPrimarySkills.skills.Size() * 3;
	var levelAtual := cint(periciaCombinadaAcima/divisor);

	return struct { primaryDivisor := divisor, currentLevel := levelAtual , relevantCombinedSkills := periciaCombinadaAcima};
endfunction

function GetBuyHabDiference(who, desiredLevel)
	var skillLevelInfo := GetSkillLevelInfo(who);
	return (skillLevelInfo.relevantCombinedSkills - (desiredLevel * skillLevelInfo.primaryDivisor)) * -1;
endfunction

function GetCharacterSkills(who, prioridadePericia)
	var chardata := GetObjProperty(who, "chardata");
	var retorno := struct { skills := array, combinedSkillsValue := 0 };
	var returnList := array;

	foreach pericia in (chardata.Skills.keys())
		Sleepms(2);
		if (cint(chardata.Skills[pericia])  == prioridadePericia || prioridadePericia == 0)
			var valor := AP_GetTrueSkill(who, pericia);
			retorno.skills.append(struct {name := pericia, value := valor});
			retorno.combinedSkillsValue := retorno.combinedSkillsValue + valor;
		endif
	endforeach

	return retorno;
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

// ---------------------------------------------------------------------------
// Casting class identity — single source of truth
// ---------------------------------------------------------------------------
// Paladino, Cacador de Demonios, Malandro Arcano and Shinobi can CAST from
// level 1 purely by class identity. The level-1 *ability* sharing the name
// (Consagracao Divina / Sigilo de Protecao / Sensibilidade Arcana / Caminho
// do Ninja) is a SEPARATE level-up reward the class buys for its mechanical
// effects (willpower, advantage, lockpick, disguise, stealth) — it is NOT
// granted at creation, so the level-1 ability pick still has value. Arqueiro
// Arcano is the lone exception: ARCANE_BOW IS granted at creation (its
// abilities need the arrow), and that grant stays inline in
// epicgumpcriacao.src::applyClassStarter.
//
// SignatureAbilityFor maps a class to that ability purely so CanCastAsClass
// can honor pre-path-rework characters who earned casting by buying it.
function SignatureAbilityFor(subclasse)
	if (subclasse == "Paladino")
		return ABILITY_ID::CONSAGRACAO_DIVINA;
	elseif (subclasse == "Cacador_de_Demonios")
		return ABILITY_ID::SIGILO_DE_PROTECAO;
	elseif (subclasse == "Malandro_Arcano")
		return ABILITY_ID::SENSIBILIDADE_ARCANA;
	elseif (subclasse == "Shinobi")
		return ABILITY_ID::NINJA_PATH;
	elseif (subclasse == "Arqueiro_Arcano")
		return ABILITY_ID::ARCANE_BOW;
	endif
	return 0;
endfunction

// The single predicate every CASTING gate uses. Casting identity is the
// class (chardata.subclasse), available from level 1. HasAbility(signature)
// is the legacy fallback ONLY — pre-path-rework chars who earned casting by
// buying the ability. This deliberately grants nothing: the ability and its
// mechanical effects stay a level-up reward gated by HasAbility elsewhere.
function CanCastAsClass(who, subclasse_name)
	var cd := GetObjProperty(who, "chardata");
	if (cd != error && cd && cd.subclasse == subclasse_name)
		return 1;
	endif
	var ability_id := SignatureAbilityFor(subclasse_name);
	if (!ability_id)
		return 0;
	endif
	return MobileWrapper(who).HasAbility(ability_id);
endfunction
