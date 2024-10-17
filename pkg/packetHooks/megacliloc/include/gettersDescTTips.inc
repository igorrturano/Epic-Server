use uo;

include "/include/epicConstants";
include ":charactercreation:habilidades";
include ":custompizza:itemrp/include/lore_utils";
include ":megacliloc:/include/toolTipUtils";
include ":megacliloc:/include/gettersTTipUtils";
include ":equipsys:include/equipstats";

Const FECHA_BASEFONT_TAG := "</BASEFONT>";

function PegaDescMagica(objeto,xDesc := "")
	if (PegaPropObjItemOrTemplate(objeto,"magic_excep"))
		xDesc := xDesc + ROXO + "Magico Excepcional<br>";
	elseif (PegaPropObjItemOrTemplate(objeto,"magic"))
		xDesc := xDesc + ROXO + "Magico<br>";
	endif
	return xDesc;
endfunction

function PegaDescDosesVeneno(objeto,xDesc := "")
	if (PegaPropObjItemOrTemplate(objeto,"poisonInfo") != error )
		if (objeto.graphic == 0x0F01)
			xDesc := xDesc + SOFT_GREEN + "Doses de Veneno: " + PegaPropObjItemOrTemplate(objeto,"poisonInfo").dosage +  "<br>";
		endif
	endif
	return xDesc;
endfunction

function PegaDescMunicao(objeto,xDesc := "")
	if (!PegaPropObjItemOrTemplate(objeto, "balas"))
		xDesc := xDesc + WHITE + "Sem Munição<br>";
	else
		xDesc := xDesc + WHITE + "Munição: " + PegaPropObjItemOrTemplate(objeto, "balas") + "<br>";
	endif
	return xDesc;
endfunction

function PegaDescArtefato(objeto,who,xDesc := "")

	var description := PegaPropObjItemOrTemplate(objeto, "description");
	var lingua := PegaPropObjItemOrTemplate(objeto, "lingua");

	if (lingua != error)
		xDesc := xDesc + "<BASEFONT COLOR=#FFD700>[Artefato Arqueológico]<br>";

		if (description != error && description != "")
			xDesc := PegaDescLeitura(who,lingua,description,xDesc);
		else
			xDesc := xDesc + "<BASEFONT COLOR=#FFFFFF>*Sem inscrições visíveis*<br>";
		endif
	elseif (description != error && description != "")
		xDesc := xDesc + "<BASEFONT COLOR=#FFFFFF>" + description + "<br>";
	endif

	return xDesc;
endfunction

function PegaDescLeitura(who,lingua,description,xDesc := "")
	var tooltip := "";
	var pode_ler := SabeLerLinguagem(who,lingua);

	if (pode_ler.sabeLer)
		xDesc := xDesc + "<BASEFONT COLOR=#C0C0C0>{}<br><BASEFONT COLOR=#FFFFFF>{}<br>".format(pode_ler.fonte,description);
	else
		xDesc := xDesc + "<BASEFONT COLOR=#FFFFFF>*Inscrições em Língua Desconhecida*<br>";
		xDesc := xDesc + "<BASEFONT COLOR=#FFFFFF>*Não foi decifrado*<br>";
	endif

	return xDesc;
endfunction

function PegaDescAAComum(objeto,xDesc := "")

	if (objeto.quality >= 1.1)
		xDesc := xDesc + WHITE + getQuality(objeto.quality) + "<br>"; //exibe apenas dois digitos
	endif

	var cond := GetCondition(objeto);
	xDesc := xDesc + "Condição: " + GetCondition(objeto) + "<br>";

	return xDesc;
endfunction

function PegaDescArmas(objeto,who,cfg := struct)

	if(cfg == error || cfg.size() <= 0)
		cfg := MontaObjCfgArmaArmadura(objeto);
	endif
	var xDesc := cfg.desc;
	if(cfg.elem.strrequired > 0)
		xDesc := xDesc + "Força exigida: " + cfg.elem.strrequired + "<br>";
	endif
	xDesc := xDesc + "Perícia: " + StrReplace(cfg.elem.Attribute,"_"," ") + "<br>";
	xDesc := xDesc + "Dano: <br>";
	xDesc := PegaDescDanoArma(objeto,cfg,xDesc);
	xDesc := xDesc + "Velocidade: " + GetVelStringDesc(cfg.elem, objeto) + "<br>";
	var crits := GetCriticalInfo(cfg.elem);
	if (crits != "")
		xDesc := xDesc + LARANJA + crits;
	endif

	var hits :=  GetHitsInfo(cfg.elem);
	if (hits != "")
		xDesc := xDesc + AMARELO_CLARO + hits;
	endif

	if (objeto.hp_mod > 0)
		xDesc := xDesc + ROXO + "Bônus Item PVs "+objeto.hp_mod;
	endif
		
	//XDesc := PegaDescDanoModArma(objeto,xDesc); Comentado até ter sido revisto no novo sistema de dano.
	XDesc := PegaDescCargasArmElemental(objeto,who,xDesc);

	if (PegaPropObjItemOrTemplate(objeto, "swordmage") != error)
		xDesc := xDesc + WHITE + PegaPropObjItemOrTemplate(objeto, "swordmage") + "<br>";
	endif

	var hitsscripts := PegaPropObjItemOrTemplate(objeto, "HitScripts");
	if (hitsscripts != error)
		foreach hits in hitsscripts
		var dano_esp := GetDmgStringSpecialDmg(cfg.elem, objeto);
			if (hits == ":combat:weaponHitScripts/FireHit")
				xDesc := xDesc + ROXO + "Dano de Fogo "+dano_esp+"<br>";
			elseif ( hits == ":combat:weaponHitScripts/IceHit")
				xDesc := xDesc + ROXO + "Dano de Gelo "+dano_esp +"<br>";
			elseif ( hits == ":combat:weaponHitScripts/LightningHit")
				xDesc := xDesc + ROXO + "Dano de Energia "+dano_esp+"<br>";
			endif
		endforeach
	endif
	return xDesc;
endfunction

function PegaDescDanoArma(objeto,cfg,xDesc := "")
	
	var atrib := PegaPropObjItemOrTemplate(objeto, "atributos");
	var bluntMD,slashMD,pircMD :=0;
	var bluntOD,slashOD,pircOD := "";
	var elemMainDamage := SplitWords(cfg.elem.EpicMainDamage);
	
	if(!atrib)
		var elemOffDamage := SplitWords(cfg.elem.EpicOffDamage);
		atrib.+SlashMainDamage := getDamageByTier(elemMainDamage[1]);
		atrib.+PiercingMainDamage := getDamageByTier(elemMainDamage[2]);
		atrib.+BluntMainDamage := getDamageByTier(elemMainDamage[3]);

		atrib.+SlashOffDamage :=getDamageByTier(elemOffDamage[1]);
		atrib.+PiercingOffDamage :=getDamageByTier(elemOffDamage[2]);
		atrib.+BluntOffDamage :=getDamageByTier(elemOffDamage[3]);
	endif

	slashMD := atrib.SlashMainDamage;
	pircMD := atrib.PiercingMainDamage;
	bluntMD := atrib.BluntMainDamage;

	slashOD := atrib.SlashOffDamage? FISICO_OFF + " " + atrib.SlashOffDamage + FECHA_BASEFONT_TAG:"";
	pircOD := atrib.PiercingOffDamage? FISICO_OFF + " " + atrib.PiercingOffDamage + FECHA_BASEFONT_TAG:"";
	bluntOD := atrib.BluntOffDamage? FISICO_OFF + " " + atrib.BluntOffDamage + FECHA_BASEFONT_TAG:"";
	
	if(slashMD !=  0)
		xDesc := xDesc  + " Cort: " + ClassificaDanoCor(elemMainDamage[1])  + (slashMD) + FECHA_BASEFONT_TAG + slashOD + "<br>";
	endif
	if(pircMD !=  0)
		xDesc := xDesc  + " Perf: " + ClassificaDanoCor(elemMainDamage[2]) + (pircMD) + FECHA_BASEFONT_TAG + pircOD + "<br>";
	endif
	if(bluntMD !=  0)
		xDesc := xDesc  + " Cont: " + ClassificaDanoCor(elemMainDamage[3]) + (bluntMD) + FECHA_BASEFONT_TAG + bluntOD + "<br>";
	endif

	return xDesc;
endfunction

function PegaDescDanoModArma(objeto,xDesc := "")
	//TODO alterar uma vez que o sistema de combate estiver terminado.

	return xDesc;
endfunction

function PegaDescCargasArmElemental(objeto, who, XDesc := "")
	if ( PegaPropObjItemOrTemplate(objeto,"#elementalweapon").serial == who.serial)	
		var elWpn := PegaPropObjItemOrTemplate(objeto, "#elementalweapon");
		if (elWpn.type == "Cold")
			xDesc := xDesc + BLUE_ICE + "Cargas Arcanas: "+elWpn.cargas+"<br>";
		elseif ( elWpn.type == "Fire")
			xDesc := xDesc + RED_FIRE + "Cargas Arcanas: "+elWpn.cargas +"<br>";
		elseif (elWpn.type == "Energy")
			xDesc := xDesc + ROXO + "Cargas Arcanas "+elWpn.cargas+"<br>";
		endif
	endif
	return XDesc;
endfunction

function PegaDescArmaduras(objeto,cfg := struct)

	if(cfg == error || cfg.size() <= 0)
		cfg := MontaObjCfgArmaArmadura(objeto);
	endif

	var xDesc := cfg.desc;
	if (cint(cfg.elem.strrequired) > 0)
		xDesc := xDesc + "Min Str: " + cfg.elem.strrequired + "<br>";
	endif
	if (cfg.elem.DexPenalty || PegaPropObjItemOrTemplate(objeto, "penaltybonus"))
		xDesc := xDesc + "Redutor Dex : " + getDexPenalty(cfg.elem, objeto) + "<br>";
	endif
	if (cfg.elem.parrybonus || PegaPropObjItemOrTemplate(objeto, "parrybonus"))
		xDesc := xDesc + "Block Bonus: " + getParryBonus(cfg.elem, objeto) + "<br>";
	endif
	if (cfg.elem.parrydelay || PegaPropObjItemOrTemplate(objeto, "parrydelay_mod"))
		xDesc := xDesc + "Delay Bonus: " + getParryDelay(cfg.elem, objeto) + "<br>";
	endif

	xDesc := PegaDescResistArmadura(objeto,cfg,xDesc);

	return xDesc;
endfunction

function PegaDescResistArmadura(objeto,cfg, xDesc := "")
	
	var atrib := PegaPropObjItemOrTemplate(objeto, "atributos");

	var bluntRes,slashRes,pircRes :=0;
	if(!atrib)
		atrib := getResistByProtection(cfg.elem.TipoProtecao);
	endif
	slashRes := atrib.SlashResist;
	pircRes := atrib.PiercingResist;
	bluntRes := atrib.BluntResist;
	
	if((slashRes + pircRes + bluntRes) + 0)
		xDesc := xDesc + "Defesas: <br>";
		if(slashRes> 0)
			xDesc := xDesc  + " Cort:" + ClassificaResistCor(cfg.elem.TipoProtecao) + (slashRes) + FECHA_BASEFONT_TAG  + "<br>";
		endif
		if(pircRes> 0)
			xDesc := xDesc  + " Perf:" + ClassificaResistCor(cfg.elem.TipoProtecao) + (pircRes) + FECHA_BASEFONT_TAG  + "<br>";
		endif
		if(bluntRes> 0)
			xDesc := xDesc  + " Cont:" + ClassificaResistCor(cfg.elem.TipoProtecao) + (bluntRes) + FECHA_BASEFONT_TAG + "<br>";
		endif
	endif
	return xDesc;
endfunction

function PegaDescMagicLore(objeto, who , xDesc := "")
	var pplmage := PegaPropObjItemOrTemplate(objeto, "magicwho"); //magic lore
		if ( pplmage != error)
			if (who.serial in pplmage)
				if (PegaPropObjItemOrTemplate(objeto, "pocaoname"))
					xDesc := xDesc + WHITE + PegaPropObjItemOrTemplate(objeto, "pocaoname") + "<br>";
					var potencia := PegaPropObjItemOrTemplate(objeto, "crafterskill");
					case(potencia)
						0: potencia := "Inábil";
						10: potencia := "Incompetente";
						20: potencia := "Inexperiente";
						30: potencia := "Iniciante";
						40: potencia := "Aprendiz";
						50: potencia := "Habilidoso";
						60: potencia := "Treinado";
						70: potencia := "Competente";
						80: potencia := "Experiente";
						90: potencia := "Mestre";
						100: potencia := "Grão Mestre";
						110: potencia := "Lendário";
					endcase
					xDesc := xDesc + ROXO + potencia + "<br>";
				endif
				var mods := PegaPropObjItemOrTemplate(objeto, "equip");
				if (mods != error)
					foreach mod in (mods.keys())
						if (mod == "strmod")
							xDesc := xDesc + ROXO + "Bônus For "+mods[mod]+"<br>";
						elseif (mod == "dexmod")
							xDesc := xDesc + ROXO + "Bônus Des "+mods[mod]+"<br>";
						elseif (mod == "intmod")
							xDesc := xDesc + ROXO + "Bônus Int "+mods[mod]+"<br>";
						elseif (mod == "hitsmod")
							xDesc := xDesc + ROXO + "Bônus PVs "+mods[mod]+"<br>";
						elseif (mod == "manamod")
							xDesc := xDesc + ROXO + "Bônus Mana "+mods[mod]+"<br>";
						elseif (mod == "stammod")
							xDesc := xDesc + ROXO + "Bônus Stam "+mods[mod]+"<br>";
						elseif (mod == "chancetocritic")
							xDesc := xDesc + ROXO + "Chande de Crítico +"+mods[mod]+"%<br>";
						elseif (mod == "PhysicalResistMod")
							xDesc := xDesc + ROXO + "Resistência Física +"+mods[mod]+"<br>";
						elseif (mod == "ColdResistMod")
							xDesc := xDesc + ROXO  + "Resistência Frio +"+mods[mod]+"<br>";
						elseif (mod == "FireResistMod")
							xDesc := xDesc + ROXO + "Resistência Fogo +"+mods[mod]+"<br>";
						elseif (mod == "EnergyResistMod")
							xDesc := xDesc + ROXO  + "Resistência Energia +"+mods[mod]+"<br>";
						elseif (mod == "PoisonResistMod")
							xDesc := xDesc + ROXO  + "Resistência Veneno +"+mods[mod]+"<br>";
						else
							if ((mod != "dmg_mod") && (mod != "hitscript"))
							xDesc := xDesc + ROXO + "Bonus " + mod + "<br>";
							endif
						endif
					endforeach
				endif
			endif
		endif
	return xDesc;
endfunction

function PegaDescGoles(objeto, xDesc)
		if (getobjproperty(objeto, "goles"))
			xDesc := xDesc + BLUE_ICE + GetObjProperty(objeto, "goles") + " doses";
		endif
		return xDesc;
endfunction