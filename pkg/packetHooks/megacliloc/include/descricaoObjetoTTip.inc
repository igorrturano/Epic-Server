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

	var tipoObjeto := ClassificaParametro(objeto);
	var isArma := tipoObjeto == ITEM_ARMA || tipoObjeto == TEMPLATE_ARMA;
	var isArmadura := tipoObjeto == ITEM_ARMADURA || tipoObjeto == TEMPLATE_ARMADURA;

	xDesc := PegaDescMagica(objeto,xDesc);
	xDesc := PegaDescComponente(objeto,xDesc);
	xDesc := PegaDescDosesVeneno(objeto,xDesc);
	
	if (isFireWeapon(objeto))
		xDesc := PegaDescMunicao(objeto,xDesc);
	endif

	xDesc := PegaDescArtefato(objeto,who,xDesc);
	xDesc := PegaDescEmperrado(objeto,xDesc);
	xDesc := PegaDescComum(objeto,xDesc);
	
	if (isArma || isArmadura)
		
		var cfgObj := MontaObjCfgArmaArmadura(objeto,xDesc);
		if(isArma)
			XDesc := PegaDescArmas(objeto,who,cfgObj);//o config já leva a desc
		elseif (isArmadura)
			xDesc := PegaDescArmaduras(objeto,cfgObj);//o config já leva a desc
		endif
	endif

	xDesc := PegaDescMagicLore(objeto,who,xDesc);
	XDesc := PegaDescGoles(objeto,xDesc);
	
	return xDesc;
endfunction




