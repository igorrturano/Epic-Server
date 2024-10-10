use uo;

include "/include/epicConstants";
include ":charactercreation:habilidades";
include ":custompizza:itemrp/include/lore_utils";
include ":megacliloc:/include/toolTipUtils";
include ":megacliloc:/include/gettersTTipUtils";
include ":combat:damage";


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


	xDesc := xDesc + "Força exigida: " + cfg.elem.strrequired + "<br>";
	xDesc := xDesc + "Dano: " + GetDmgStringDesc(cfg.elem,  objeto) + "<br>";
	if (cfg.elem.OffHandDamage)
		xDesc := xDesc + "OffDano: " + GetOffHandDmgStringDesc(cfg.elem,  objeto) + "<br>";
	endif
	xDesc := xDesc + "Velocidade: " + GetVelStringDesc(cfg.elem, objeto) + "<br>";
	xDesc := xDesc + GetTipoStringDesc(cfg.elem, objeto) + "<br>";
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
		

	XDesc := PegaDescDanoModArma(objeto,xDesc);
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

function PegaDescDanoModArma(objeto,xDesc := "")
	xDesc := xDesc + WHITE + "Fisico " + (objeto.damage_physical ) + "<br>";

	if ( (objeto.damage_cold + objeto.damage_cold_mod != 0))
		xDesc := xDesc  + BLUE_ICE + "Frio " + (objeto.damage_cold ) + "<br>";
	endif
	if ( (objeto.damage_fire + objeto.damage_fire_mod != 0))
		xDesc := xDesc + RED_FIRE + "Fogo " + (objeto.damage_fire ) + "<br>";
	endif
	if ( (objeto.damage_energy + objeto.damage_mod != 0))
		xDesc := xDesc  + LILAS + "Energia " + (objeto.damage_energy ) + "<br>";
	endif
	if ( (objeto.damage_poison + objeto.damage_poison_mod != 0))
		xDesc := xDesc  + SOFT_GREEN + "Veneno " + (objeto.damage_poison ) + "<br>";
	endif

	if (objeto.dmg_mod > 0 )
		xDesc := xDesc + ROXO + "Bônus Dano "+objeto.dmg_mod+ " <br>";
	endif

	return xDesc;
endfunction

function PegaDescCargasArmElemental(objeto, who, XDesc := "")
	if ( PegaPropObjItemOrTemplate(objeto,"#elementalweapon").serial == who.serial)	
		var elWpn := PegaPropObjItemOrTemplate(objeto, "#elementalweapon");
		if (elWpn.type == DMG_COLD)
			xDesc := xDesc + BLUE_ICE + "Cargas Arcanas: "+elWpn.cargas+"<br>";
		elseif ( elWpn.type == DMG_FIRE)
			xDesc := xDesc + RED_FIRE + "Cargas Arcanas: "+elWpn.cargas +"<br>";
		elseif (elWpn.type == DMG_ENERGY)
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

	xDesc := PegaDescResistArmadura(objeto,xDesc);

	return xDesc;
endfunction

function PegaDescResistArmadura(objeto, xDesc := "")
		xDesc := xDesc + FISICO + "Fisico " + (objeto.resist_physical ) + "<br>";

	if ( CInt(objeto.resist_cold) != 0)
		xDesc := xDesc  + BLUE_ICE + "Frio " + (objeto.resist_cold ) + "<br>";
	endif
	if ( CInt(objeto.resist_fire) != 0)
		xDesc := xDesc + RED_FIRE + "Fogo " + (objeto.resist_fire ) + "<br>";
	endif
	if ( CInt(objeto.resist_energy) != 0)
		xDesc := xDesc  + LILAS + "Energia " + (objeto.resist_energy ) + "<br>";
	endif
	if ( CInt(objeto.resist_poison) != 0)
		xDesc := xDesc  + SOFT_GREEN + "Veneno " + (objeto.resist_poison ) + "<br>";
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