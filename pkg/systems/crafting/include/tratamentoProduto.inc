use uo;
include "/include/epicPropNames";

function PegaMaximoFalhas(item)
		var propFabricado := GetObjProperty(item,PROPNAME_FABRICADO);
		if(propFabricado)
			return cint(propFabricado.receita.sucessosNecessarios * 2);
		endif
		return 0;
endfunction

