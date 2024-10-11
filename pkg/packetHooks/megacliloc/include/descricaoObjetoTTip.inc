use uo;

include "/include/epicConstants";
include ":megacliloc:/include/gettersDescTTips";
include "include/epicCombatUtils";

function MontaDescPersonagem(objeto, quem, xDesc)
	if (!objeto.npctemplate)
		if (quem.cmdlevel > 1) //Staff consegue ver o nome e raça
			if(GetObjProperty(objeto, "disfarce"))
				xDesc := xDesc + WHITE + " [disfarce] [" + objeto.acctname + "]" + "<br>";
			endif
			var culture;
			if (GetObjProperty(objeto, "chardata"))
				culture := GetObjProperty(objeto, "chardata").culture;
			endif
			if (culture)
				xDesc := xDesc + WHITE + culture + "<br>";
			endif
		endif
	endif
	return xDesc;
endfunction

function MontaDescricaoItem(objeto,who, xDesc := "")

	xDesc := PegaDescMagica(objeto,xDesc);
	xDesc := PegaDescDosesVeneno(objeto,xDesc);
	
	if (isFireWeapon(objeto))
		xDesc := PegaDescMunicao(objeto,xDesc);
	endif

	xDesc := PegaDescArtefato(objeto,who,xDesc);

	if (objeto.isA(POLCLASS_WEAPON) || objeto.isA(POLCLASS_ARMOR))
		var cfgObj := MontaObjCfgArmaArmadura(objeto,xDesc);
		
		cfgObj.desc := PegaDescAAComum(objeto,cfgObj.desc);//Pega a descricao comum entre arma e armadura.
			
		if(objeto.isA(POLCLASS_WEAPON))
			XDesc := PegaDescArmas(objeto,who,cfgObj);//o config já leva a desc
		elseif (objeto.isA(POLCLASS_ARMOR))
			xDesc := PegaDescArmaduras(objeto,cfgObj);//o config já leva a desc
		endif
	endif

	xDesc := PegaDescMagicLore(objeto,who,xDesc);
	XDesc := PegaDescGoles(objeto,xDesc);
	
	return xDesc;
endfunction




