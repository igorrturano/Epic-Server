use uo;

include "/include/epicConstants";
include "/include/epicPropNames";
include ":charactercreation:habilidades";
include ":custompizza:itemrp/include/lore_utils";
include ":megacliloc:/include/toolTipUtils";
include ":megacliloc:/include/gettersTTipUtils";
include ":equipsys:Controller/WeaponController";
include ":destroyable:include/destroyItem";
include ":crafting:/include/fabricacaoUtil";
include ":equipsys:Controller/ArmorController";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":wrappers:Objects/Model/ArmorWrapper";
include ":wrappers:Objects/Model/MobileWrapper";

Const FECHA_BASEFONT_TAG := "</BASEFONT>";

function PegaDescMagica(objeto,xDesc := "")
	if (PegaPropObjItemOrTemplate(objeto,"magic_excep"))
		xDesc := xDesc + ROXO + "Magico Excepcional<br>";
	elseif (PegaPropObjItemOrTemplate(objeto,"magic"))
		xDesc := xDesc + ROXO + "Magico<br>";
	endif
	return xDesc;
endfunction

function PegaDescComponente(objeto,xDesc := "")
	var prop := PegaPropObjItemOrTemplate(objeto,PROPNAME_TIER_COMPONENTE);
	if(prop)
		xDesc := xDesc + SOFT_PINK + "Componente<br>";
		var tierDesc := PegaComponenteTierDesc(prop).desc;
		if (tierDesc)
			xDesc := xDesc + WHITE + "Qualidade: " + tierDesc + "<br>";
		endif
		var encaixeProp := PegaPropObjItemOrTemplate(objeto, PROPNAME_ENCAIXE_COMPONENTE);
		if(encaixeProp)
			xDesc := xDesc + WHITE + "Encaixe: " + encaixeProp + "<br>";
		endif
	endif

	prop := PegaPropObjItemOrTemplate(objeto,PROPNAME_FABRICADO);
	if((prop.estado == ITEM_PRONTO || prop.estado == ITEM_GERADO) && !prop.isCompletamenteComum)
		//xDesc := xDesc + "<br>";
		if(prop.qualidade > 0)
			xDesc := xDesc + LARANJA + "Excepcional<br>";
		endif
		var larguraInicial := Len(xDesc);
		foreach componente in (prop.componentes)
			xDesc := PegaDescEfeitosComponente(componente, xDesc);
			Sleepms(2);
		endforeach
		if(larguraInicial != Len(xDesc))
			xDesc := xDesc + "<br>";
		endif
	endif
	return xDesc;
endfunction

function PegaDescProdutoMontado(objeto,xDesc := "")
	var prop := PegaPropObjItemOrTemplate(objeto,PROPNAME_FABRICADO);
	if(prop)
		var quantidadeLote := "";
		if(prop.receita.lote && prop.receita.lote > 1)
			quantidadeLote := prop.receita.lote  + "x ";
		endif
		
		var componentes;
		if(prop.estado == ITEM_MONTADO)
			xDesc := xDesc + WHITE + "Item: "+  quantidadeLote + prop.receita.desc +"<br>";
			componentes := prop.esquema.componentesEscolhidos;
		else
			componentes := prop.componentes;
		endif 

		if(componentes)
			if(componentes.size() > 0)
				xDesc := xDesc +"<br>"+ BLUE_ICE + "Componentes<br>";
				foreach componenteEscolhido in (componentes)
					Sleepms(2);
					var descComponente := " {} [{}] [{}]".format(componenteEscolhido.item.nome,
																componenteEscolhido.material.nome,
																componenteEscolhido.tier.desc); 
					xDesc := xDesc + WHITE + descComponente + " <br>";
				endforeach
			endif
		endif
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

function PegaDescComum(objeto,xDesc := "")

	/*if (objeto.quality >= 1.1)
		xDesc := xDesc + WHITE + getQuality(objeto.quality) + "<br>"; //exibe apenas dois digitos
	endif*/

	var cond := GetCondition(objeto);

	if(objeto.ISA(POLCLASS_EQUIPMENT) || objeto.isVendorTemplate)
		xDesc := xDesc + "Condição: " + cond + "<br>";
	else
		if(cond != "Boa")
			xDesc := xDesc + "Condição: " + cond + "<br>";
		endif
	endif

	return xDesc;
endfunction

function PegaDescArmas(objeto,who,cfg := struct)

	if(cfg == error || cfg.size() <= 0)
		cfg := MontaObjCfgArmaArmadura(objeto);
	endif

	var weapon := WeaponWrapper(objeto);
	weapon.SetCprops(objeto.Cprops);
	var character := MobileWrapper(who);
	var xDesc := cfg.desc;
	// Coloca vermelho se a força do usuario for menor, verde se for maior ou igual a força exigida
	var str_required := weapon.GetSTRRequired();
	
	if(str_required > 0)
		if (character.GetSTR() < str_required)
			xDesc := xDesc + RED + "Força exigida: " + str_required + "<br>";
		else
			xDesc := xDesc + "Força exigida: " + str_required + "<br>";
		endif
	endif

	xDesc := xDesc + "Perícia: " + StrReplace(weapon.GetAttribute(),"_"," ") + "<br>";
	xDesc := xDesc + "Dano: <br>";
	xDesc := PegaDescDanoArma(objeto,cfg,xDesc, weapon);
	XDesc := PegaDescDanoModArma(objeto,xDesc);

	xDesc := xDesc + "Velocidade: " + GetVelStringDesc2(weapon.GetSpeedTier()) + "<br>";

	var crits := GetCriticalInfo(cfg.elem);
	if (crits != "")
		xDesc := xDesc + LARANJA + crits;
	endif

	var hits :=  GetHitsInfo2(cfg.elem);
	if (hits != "")
		xDesc := xDesc + AMARELO_CLARO + hits;
	endif

	if (objeto.hp_mod > 0)
		xDesc := xDesc + ROXO + "Bônus Item PVs "+ objeto.hp_mod;
	endif

	if (PegaPropObjItemOrTemplate(objeto, "Aprimoramentos"))
		xDesc := xDesc + ROXO + "Aprimoramentos: " +  "<br>";
		foreach key in (PegaPropObjItemOrTemplate(objeto, "Aprimoramentos").keys())
				xDesc := xDesc + AMARELO_CLARO + key + "<br>";
		endforeach
	endif
		
	xDesc := PegaDescCargasArmElemental(objeto,who,xDesc);
	
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

function PegaDescDanoArma(objeto,cfg,xDesc := "", weapon := 0)
	
	if(!weapon)
		weapon := WeaponWrapper(objeto);
	endif

	var mainDamage := weapon.GetMainDamage();
	var offDamage := weapon.GetOffDamage();
	var bluntMD,slashMD,pircMD :=0;
	var bluntOD,slashOD,pircOD := "";
	var elemMainDamage := SplitWords(weapon.GetEpicMainDamage());

	slashMD := mainDamage.GetSlashDamage().GetDiceString();
	pircMD := mainDamage.GetPiercingDamage().GetDiceString();
	bluntMD := mainDamage.GetBluntDamage().GetDiceString();

	slashOD := offDamage.GetSlashDamage().GetDiceString()? FISICO_OFF + " " +  offDamage.GetSlashDamage().GetDiceString() + FECHA_BASEFONT_TAG:"";
	pircOD := offDamage.GetPiercingDamage().GetDiceString()? FISICO_OFF + " " + offDamage.GetPiercingDamage().GetDiceString() + FECHA_BASEFONT_TAG:"";
	bluntOD := offDamage.GetBluntDamage().GetDiceString()? FISICO_OFF + " " + offDamage.GetBluntDamage().GetDiceString() + FECHA_BASEFONT_TAG:"";
	
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

function PegaDescDanoModArma(objeto,xDesc := "", weapon := 0)
	if(!weapon)
		weapon := WeaponWrapper(objeto);
	endif

	var damageMod := weapon.GetDamageMod();
	var fireDamage := damageMod.GetFireBaseDamage();
	var coldDamage := damageMod.GetColdBaseDamage();
	var poisonDamage := damageMod.GetPoisonBaseDamage();
	var energyDamage := damageMod.GetEnergyBaseDamage();

	if(fireDamage || coldDamage || poisonDamage || energyDamage)
		xDesc := xDesc + "<br> Dano Mágico Fixo: <br>";

		if(fireDamage)
			xDesc += "Fogo: {} +{} {} <br>".format( RED_FIRE ,fireDamage,FECHA_BASEFONT_TAG);
		endif
		if(coldDamage)
			xDesc += "Gelo: {} +{} {} <br>".format( BLUE_ICE ,coldDamage,FECHA_BASEFONT_TAG);
		endif
		if(poisonDamage)
			xDesc += "Veneno: {} +{} {} <br>".format( SOFT_GREEN ,poisonDamage,FECHA_BASEFONT_TAG);
		endif
		if(energyDamage)
			xDesc += "Energia: {} +{} {} <br>".format( ROXO ,energyDamage,FECHA_BASEFONT_TAG);
		endif
	endif
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

	var armadura := ArmorWrapper(objeto); 
	armadura.SetCprops(objeto.Cprops);

	var xDesc := cfg.desc;

	if (armadura.GetSTRRequired() > 0)
		xDesc := xDesc + "Min Str: " + armadura.GetSTRRequired() + "<br>";
	endif
	if (armadura.GetModifiers()[DEXTERITY].GetValue())
		xDesc := xDesc + "Redutor Dex : " + armadura.GetModifiers()[DEXTERITY].GetValue() + "<br>";
	endif
	if (armadura.IsShield())
		var shieldAttrs := GetShieldModsByProtection(armadura.GetTipoProtecao());
		if (!(shieldAttrs is @Exception))
			xDesc := xDesc + "Bonus Bloqueio: " + shieldAttrs.GetParryBonus() + "<br>";
			xDesc := xDesc + "Delay Bloqueio: " + shieldAttrs.GetParryDelay() + "<br>";
		endif
	endif

	xDesc := PegaDescResistArmadura(objeto,cfg,armadura,xDesc);

	return xDesc;
endfunction

function PegaDescResistArmadura(objeto,cfg,armadura, xDesc := "")
	
	var atrib := struct;
	if(!armadura)
		armadura := ArmorWrapper(objeto);
	endif

	var bluntRes,slashRes,pircRes :=0;

	slashRes := armadura.GetResists().GetSlashResist();
	pircRes := armadura.GetResists().GetSlashResist();
	bluntRes := armadura.GetResists().GetSlashResist();
	var fireResist:= armadura.GetResists().GetFireResist();
	var coldResist:= armadura.GetResists().GetColdResist();
	var poisonResist := armadura.GetResists().GetPoisonResist();
	var energyResist := armadura.GetResists().GetEnergyResist();
	
	if((slashRes + pircRes + bluntRes) + 0)
		xDesc := xDesc + "Defesas: <br>";
		if(slashRes> 0)
			xDesc := xDesc  + " Cort: " + ClassificaResistCor(armadura.GetTipoProtecao()) +  (slashRes) + FECHA_BASEFONT_TAG  + "<br>";
		endif
		if(pircRes> 0)
			xDesc := xDesc  + " Perf: " + ClassificaResistCor(armadura.GetTipoProtecao()) +  (pircRes) + FECHA_BASEFONT_TAG  + "<br>";
		endif
		if(bluntRes> 0)
			xDesc := xDesc  + " Cont: " + ClassificaResistCor(armadura.GetTipoProtecao()) + (bluntRes) + FECHA_BASEFONT_TAG + "<br>";
		endif
	endif

	
	if(fireResist || coldResist || poisonResist || energyResist)
		xDesc := xDesc + "<br> Resistência Mágica: <br>";

		if(fireResist)
			xDesc += "Fogo: {} +{} {} <br>".format( RED_FIRE ,fireResist,FECHA_BASEFONT_TAG);
		endif
		if(coldResist)
			xDesc += "Gelo: {} +{} {} <br>".format( BLUE_ICE ,coldResist,FECHA_BASEFONT_TAG);
		endif
		if(poisonResist)
			xDesc += "Veneno: {} +{} {} <br>".format( SOFT_GREEN ,poisonResist,FECHA_BASEFONT_TAG);
		endif
		if(energyResist)
			xDesc += "Energia: {} +{} {} <br>".format( ROXO ,energyResist,FECHA_BASEFONT_TAG);
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
					var title ;
					if (potencia == error)
						potencia := 0;
					endif
					case(potencia)
						0: title := "Inábil";
						10: title := "Incompetente";
						20: title := "Inexperiente";
						30: title := "Iniciante";
						40: title := "Aprendiz";
						50: title := "Habilidoso";
						60: title := "Treinado";
						70: title := "Competente";
						80: title := "Experiente";
						90: title := "Mestre";
						100: title := "Grão Mestre";
						110: title := "Lendário";
					endcase
					xDesc := xDesc + ROXO + title + "<br>";
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

function PegaDescDoses(objeto, xDesc)
		if (PegaPropObjItemOrTemplate(objeto, "doses"))
			xDesc := xDesc + BLUE_ICE + PegaPropObjItemOrTemplate(objeto, "doses") + " doses";
		endif
		return xDesc;
endfunction

function PegaDescEmperrado(objeto, xDesc)
	if(objeto.ISA(POLCLASS_LOCKABLE))
		if(IsEmperrado(objeto))
			xDesc := xDesc + "[Emperrado]<br>";
		endif
	endif
	return xDesc;
endfunction

function PegaDescEfeitosComponente(componente, xDesc)
	var propriedades := componente.propriedades;
	if(propriedades.valorPrincipal != 0 && TypeOfInt(propriedades.valorPrincipal) != OT_UNINIT)
		var descValor, corLinha;

		if(propriedades.valorPrincipal > 0)
			descValor := SOFT_GREEN + "+"+ propriedades.valorPrincipal + " <br>";
			corLinha := BLUE;
		else
			corLinha := SOFT_PINK;
			descValor := RED_FIRE + propriedades.valorPrincipal + " <br>";
		endif

		if(componente.tier.valor >= PegaComponenteTierDesc(COMPONENTE_TIERS::EXCEPCIONAL).valor)
			corLinha := AMARELO_CLARO;
		endif

		xDesc := xDesc + corLinha + getComponenteToAprimoramento(propriedades.caracteristica,propriedades.valorPrincipal) + " " + descValor;
	endif
	
	if(propriedades.efeitosAdicionais)
		var descValor, corLinha;
		foreach efeitoAdicional in (propriedades.efeitosAdicionais)
			Sleepms(2);
			if(efeitoAdicional != 0)
				if(efeitoAdicional[_efeitoAdicional_iter] > 0)
					descValor := _efeitoAdicional_iter + " " + SOFT_GREEN + "+"+ efeitoAdicional[_efeitoAdicional_iter] + " <br>";
					corLinha := BLUE;
				else
					corLinha := SOFT_PINK;
					descValor := _efeitoAdicional_iter + " " + RED_FIRE + efeitoAdicional[_efeitoAdicional_iter] + " <br>";
				endif
				xDesc := xDesc + corLinha + "*" + descValor;
			else
				continue;
			endif
		endforeach
	endif
	return xDesc;
endfunction

function PegaDescCargasRunicas(objeto, who, XDesc := "")
	if ( GetObjProperty(objeto, "runecharges"))	
		xDesc := xDesc + AMARELO_CLARO + "Cargas Rúnicas: "+GetObjProperty(objeto, "runecharges")+"<br>";
	endif
	return XDesc;
endfunction

function PegaDescEstudavel(objeto, xDesc := "")
	var skill_name := GetObjProperty(objeto, "estudavel");
	if (!skill_name)
		return xDesc;
	endif

	var skill_traduzida := TraduzirProfissao(skill_name);
	xDesc := xDesc + "<BASEFONT COLOR=#a8d8ea>[Item de Estudo]<br>";
	xDesc := xDesc + "<BASEFONT COLOR=#FFFFFF>Área: " + skill_traduzida + "<br>";
	xDesc := xDesc + "<BASEFONT COLOR=#C0C0C0>Limite de estudo: 52.0<br>";
	return xDesc;
endfunction