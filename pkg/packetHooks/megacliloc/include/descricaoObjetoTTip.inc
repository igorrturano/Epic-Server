use uo;
use basic;

include "/include/epicConstants";
include ":megacliloc:/include/gettersDescTTips";
include "include/epicCombatUtils";

function MontaDescPersonagem(objeto, quem, xDesc)
	if (!objeto.npctemplate)
		if (quem.cmdlevel > 1) //Staff consegue ver o nome e raça
			if (GetObjProperty(objeto, "disfarce"))
				xDesc := xDesc + WHITE + " [disfarce] [" + objeto.acctname + "]" + "<br>";
			endif
			var chardata := GetObjProperty(objeto, "chardata");
			if (chardata && chardata.culture)
				xDesc := xDesc + WHITE + chardata.culture + "<br>";
			endif
		endif

		// Entropy-based appearance description (shows for everyone)
		var entropy_desc := GetObjProperty(objeto, "descmodentropy");
		if (entropy_desc)
			var tier := GetObjProperty(objeto, "entropy_description_tier");
			// tier missing → error (falsy) → fallback to AMARELO_CLARO.
			// tier < 0 explicit numeric compare needs the not-error guard
			// because `error < 0` semantics are untested.
			var color := (tier != error && tier < 0) ? ROXO : AMARELO_CLARO;
			xDesc := xDesc + color + entropy_desc + "<br>";
		endif
	endif

	// Pet/creature description set via RURAL_LIFE (Campestre) ability
	if (objeto.npctemplate)
		var pet_desc := GetObjProperty(objeto, "description");
		if (pet_desc)
			xDesc := xDesc + WHITE + pet_desc + "<br>";
		endif
	endif

	// Descrições agregadas dos itens equipados (montado por equip/unequip.src)
	// Cada entrada já vem com "<br>" embutido pelo /setdescmod / /setdesc.
	var equip_descs := GetObjProperty(objeto, "descmod");
	if (equip_descs != error)
		if (TypeOf(equip_descs) == "Array")
			foreach entry in equip_descs
				if (entry)
					xDesc := xDesc + WHITE + entry;
				endif
			endforeach
		elseif (TypeOf(equip_descs) == "String")
			xDesc := xDesc + WHITE + equip_descs;
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




