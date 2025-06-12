include ":equipsys:utils";

print("INSTALLING: Methods for Armor -> Equipment System... OK!");

/**
 * getSlot(item)
 * 
 * Obtém o slot de equipamento (Coverage/Cobertura) de um item através de seu arquivo de configuração.
 * 
 * Parâmetros
 * item: objRef do item para verificar o slot
 * 
 * Valor de retorno
 * Retorna "Shield" se o item for um escudo, caso contrário retorna o valor de Coverage do arquivo de configuração
 * ou um erro se Coverage não estiver definido
 */
exported function getSlot(item) //PEGA O SLOT DO ITEM NO .CFG
	var elem := getItemCfgElem(item);

	if (isShield(item))
		return "Shield";
	endif

	return elem.Coverage ? elem.Coverage : error{"errortext" := "The item have no Coverage!"};
endfunction

/**
 * isShield(item) 
 *
 * Verifica se um item é um escudo examinando seu tipo de equipamento
 *
 * Parâmetros
 * item: objRef do item para verificar se é escudo
 *
 * Valor de retorno
 * Retorna verdadeiro se o item for um escudo ("escudo"), falso caso contrário
 */
exported function isShield(item)
    var tipoequip := item.getType();
    var escudo := SplitWords(tipoequip); escudo := escudo[1];
	escudo.Lower();

	return escudo == "escudo";
endfunction

/**
 * getType(item)
 *
 * Obtém o tipo de proteção de um item de armadura através de sua configuração
 *
 * Parâmetros
 * item: objRef do item para verificar o tipo de proteção
 *
 * Valor de retorno
 * Retorna o valor de TipoProtecao (Tipo de Proteção) do arquivo de configuração
 * ou um erro se o Tipo de Proteção não estiver definido
 */
exported function getType(item)
	var elem := getItemCfgElem(item);
	return elem.TipoProtecao ? elem.TipoProtecao : error{"errortext" := "The item have no Protection Type!"};
endfunction