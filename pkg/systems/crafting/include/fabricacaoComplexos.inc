include ":crafting:include/controladoras/controladoraComplexos";
include ":crafting:include/util/fabricacaoConstants";

function GeraDescriptorComplexo( 
							objtype,
							tier,
							isDispensavel := 1, 
							listaDeMateriaisIgnorados := array,
							modoEscolhaMaterial := FABRICACAO_MODO_MATERIAL::MESMO_TIER,
							deveGerarOpcionais := false, 
							classe := 1
						)

	var fabricadoProp := CriaPropFabricadoGerada(objtype, tier,deveGerarOpcionais, listaDeMateriaisIgnorados,modoEscolhaMaterial, classe);

	if(!fabricadoProp) return fabricadoProp; endif

	var descriptor := GetItemDescriptor(CInt(objtype));
	descriptor.CProps.insert(PROPNAME_MATERIAL,fabricadoProp.materialPrincipal);
	descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,isDispensavel);
	descriptor.CProps.insert(PROPNAME_FABRICADO, fabricadoProp);
	descriptor.maxhp := fabricadoProp.modificadorHpTotal? descriptor.maxhp + fabricadoProp.modificadorHpTotal : descriptor.maxhp;
	descriptor.color := fabricadoProp.cor;

	return descriptor;
endfunction

function GeraItemComplexo( 
							container,
							objtype,
							quantidade,
							tier,
							isDispensavel := 1, 
							listaDeMateriaisIgnorados := array,
							modoEscolhaMaterial := FABRICACAO_MODO_MATERIAL::MESMO_TIER,
							deveGerarOpcionais := false, 
							classe := 1,
							estagio := ITEM_GERADO
						)
	var descriptor;
	if(estagio == ITEM_GERADO)
		descriptor :=  GeraDescriptorComplexo(
								objtype,
								tier,
								isDispensavel,
								listaDeMateriaisIgnorados, 
								modoEscolhaMaterial,
								deveGerarOpcionais, 
								classe);
	elseif(estagio == ITEM_MONTADO)
		descriptor := MontaProdutoMontadoAleatorio(
								objtype,
								tier,
								modoEscolhaMaterial,
								deveGerarOpcionais, 
								classe
								);
	endif

	if(!descriptor || TypeOfInt(descriptor.color) != OT_LONG || !descriptor.graphic) return error; endif

	var item := CreateItemInContainer( container, descriptor, quantidade );
	item.hp := item.maxhp;

	return item;
endfunction
