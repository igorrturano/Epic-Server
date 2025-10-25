include ":crafting:controladoraComplexos";
include ":crafting:include/util/fabricacaoConstants";

function FabricaItensComplexos(quem,profissao, modoGm := false)
	EnviaItensComplexosGump(quem,profissao, modoGm);
endfunction

function GeraDescriptorComplexo( 
							objtype,
							tier,
							isDispensavel := 1, 
							modoEscolhaMaterial := FABRICACAO_MODO_MATERIAL::MESMO_TIER,
							deveGerarOpcionais := false, 
							classe := 1
						)

	var fabricadoProp := CriaPropFabricadoGerada(objtype, tier,deveGerarOpcionais,modoEscolhaMaterial, classe);

	var descriptor := GetItemDescriptor(CInt(objtype));
	descriptor.CProps.insert(PROPNAME_MATERIAL,fabricadoProp.materialPrincipal);
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,isDispensavel);
	descriptor.CProps.insert(PROPNAME_FABRICADO, fabricadoProp);
	descriptor.maxhp := fabricadoProp.modificadorHpTotal? descriptor.maxhp + fabricadoProp.modificadorHpTotal : descriptor.maxhp;
	descriptor.color := fabricadoProp.cor;

	return descriptor;
endfunction

function GeraItemComplexo( 
							quem,
							objtype,
							quantidade,
							tier,
							isDispensavel := 1, 
							modoEscolhaMaterial := FABRICACAO_MODO_MATERIAL::MESMO_TIER,
							deveGerarOpcionais := false, 
							classe := 1
						)

	var descriptor :=  GeraDescriptorComplexo(
							objtype,
							tier,
							isDispensavel, 
							modoEscolhaMaterial,
							deveGerarOpcionais, 
							classe);

	var item := CreateItemInContainer( quem.backpack, descriptor, quantidade );
	item.hp := item.maxhp;

	return descriptor;
endfunction

function ConverteItemGenericoParaComplexoGerado( item ,material, tier, isDispensavel, classe := 1)
	var fabricadoProp := CriaPropFabricadoGerada(item.objtype, material, tier, classe);
	SetObjProperty(item,PROPNAME_MATERIAL,material);
	SetObjProperty(item,PROPNAME_DISPENSAVEL_FLAG,isDispensavel);
	SetObjProperty(item,PROPNAME_FABRICADO,fabricadoProp);
	item.maxhp := fabricadoProp.esquema.modificadorHpTotal? item.maxhp + fabricadoProp.esquema.modificadorHpTotal : item.maxhp;
	item.hp := item.maxhp;
	item.color := fabricadoProp.esquema.cor;
	item.weight_multiplier_mod := fabricadoProp.esquema.modificadorPesoTotal ? ((fabricadoProp.esquema.modificadorPesoTotal / 100) + 1) : 0;
endfunction
