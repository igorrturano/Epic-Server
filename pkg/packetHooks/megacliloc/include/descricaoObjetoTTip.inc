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

		// Entropy-based appearance description (shows for everyone)
		var entropy_desc := GetObjProperty(objeto, "descmodentropy");
		if (entropy_desc)
			var tier := GetObjProperty(objeto, "entropy_description_tier");
			var color := WHITE;

			// Color coding: purple for corrupted, golden for radiant
			if (tier < 0)
				color := ROXO;  // Purple for dark/corrupted
			else
				color := AMARELO_CLARO;  // Golden for light/virtuous
			endif

			xDesc := xDesc + color + entropy_desc + "<br>";
		endif
	endif
	return xDesc;
endfunction

function MontaDescricaoItem(objeto,who, xDesc := "")

	var tipoObjeto := ClassificaParametro(objeto);
	var isArma := tipoObjeto == ITEM_ARMA || tipoObjeto == TEMPLATE_ARMA;
	var isArmadura := tipoObjeto == ITEM_ARMADURA || tipoObjeto == TEMPLATE_ARMADURA;

	xDesc := PegaDescEstudavel(objeto, xDesc);
	xDesc := PegaDescMagica(objeto,xDesc);
	xDesc := PegaDescComponente(objeto,xDesc);
	xDesc := PegaDescDosesVeneno(objeto,xDesc);

	xDesc := PegaDescArtefato(objeto,who,xDesc);
	xDesc := PegaDescEmperrado(objeto,xDesc);
	xDesc := PegaDescComum(objeto,xDesc);

	xDesc := PegaDescCargasRunicas(objeto,who,xDesc);
	
	if (isArma || isArmadura)
		
		var cfgObj := MontaObjCfgArmaArmadura(objeto,xDesc);
		if(isArma)
			XDesc := PegaDescArmas(objeto,who,cfgObj);//o config já leva a desc
		elseif (isArmadura)
			xDesc := PegaDescArmaduras(objeto,cfgObj);//o config já leva a desc
		endif
	endif

	xDesc := PegaDescMagicLore(objeto,who,xDesc);
	XDesc := PegaDescDoses(objeto,xDesc);
	xDesc := PegaDescProdutoMontado(objeto,xDesc);
	
	return xDesc;
endfunction




