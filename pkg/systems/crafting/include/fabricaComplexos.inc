include ":crafting:controladoraComplexos";

function FabricaItensComplexos(quem,profissao)
	EnviaItensComplexosGump(quem,profissao);
endfunction

function GeraItemComplexo( material, objtype, tier, isDispensavel, classe := 1)
	
	var fabricadoProp := CriaPropFabricadoGerada(objtype, material, tier, classe);

	var descriptor := GetItemDescriptor(CInt(objtype));
	descriptor.CProps.insert(PROPNAME_MATERIAL,material);
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,isDispensavel);
	descriptor.CProps.insert(PROPNAME_FABRICADO, fabricadoProp);
	descriptor.maxhp := fabricadoProp.esquema.modificadorHpTotal? descriptor.maxhp + fabricadoProp.esquema.modificadorHpTotal : descriptor.maxhp;
	descriptor.color := fabricadoProp.esquema.cor;

	return descriptor;
endfunction

function ConverteItemGenericoParaComplexoGerado( item ,material, tier, isDispensavel, classe := 1)
	var fabricadoProp := CriaPropFabricadoGerada(item.objtype, material, tier, classe);
	SetObjProperty(item,PROPNAME_MATERIAL,material);
	SetObjProperty(item,PROPNAME_DISPENSAVEL_FLAG,isDispensavel);
	SetObjProperty(item,PROPNAME_FABRICADO,fabricadoProp);
	item.maxhp := fabricadoProp.esquema.modificadorHpTotal? item.maxhp + fabricadoProp.esquema.modificadorHpTotal : item.maxhp;
	item.color := fabricadoProp.esquema.cor;
	item.weight_multiplier_mod := fabricadoProp.esquema.modificadorPesoTotal ? ((fabricadoProp.esquema.modificadorPesoTotal / 100) + 1) : 0;
endfunction

function GeraItemComplexoAleatorio( tipoMaterial, objtype , isDispensavel, tierMax, tierMin := 1, classe := 1)
	return GeraItemComplexo(PegaTipoMaterialAleatorio(tipoMaterial), objtype, RandomIntMinMax(tierMin, tierMax), isDispensavel, classe);
endfunction

function ConverteGenericoParaComplexoAleatorio(item, tipoMaterial, isDispensavel, tierMax, tierMin := 1, classe := 1)
	return ConverteItemGenericoParaComplexoGerado(item, PegaTipoMaterialAleatorio(tipoMaterial), RandomIntMinMax(tierMin, tierMax), isDispensavel, classe);
endfunction
