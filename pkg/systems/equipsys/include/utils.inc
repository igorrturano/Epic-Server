use uo;
use cfgfile;

function getItemCfgElem(item) //RETORNA TODOS OS VALORES DO ITEM QUE AINDA ESTIVEREM NO .CFG
    return FindConfigElem(ReadConfigFile(":combat:itemdesc"), item.objtype);
endfunction

function getType(item)
    var elem := getItemCfgElem(item);
	var tipo;

	if (item.IsA(POLCLASS_WEAPON))
		tipo := elem.TipoAtaque;
	elseif (item.IsA(POLCLASS_ARMOR))
		tipo := elem.TipoProtecao;
	endif

    return tipo;
endfunction

function getSlot(item) //PEGA O SLOT DO ITEM NO .CFG
    var elem := getItemCfgElem(item);
    var slot := elem.Coverage;
    if (IsEscudo(item)) slot := "Shield"; endif
    return slot;
endfunction

function IsEscudo(item)
    var tipoequip := getType(item);
    var escudo := SplitWords(tipoequip); escudo := escudo[1];
	escudo.Lower();

    if (escudo == "escudo")
		return 1;
    else
		return 0;
    endif
endfunction

function getPropBase(item) //PEGA A PROP DE MODIFICADORES BASE
    var prop := item.GetProp("atributos");
    if (!prop) prop := Dictionary; endif
    return prop;
endfunction

function getPropExcep(item) //PEGA A PROP DE MODIFICADORES EXCEPCIONAL
    var prop := item.GetProp("atributos_excp");
    if (!prop) prop := Dictionary; endif
    return prop;
endfunction

function setPropBase(item, prop) //SETA A PROP DE MODIFICADORES BASE
    if (prop.Size() > 0) item.SetProp("atributos_base", prop); endif
endfunction

function setPropExcep(item, prop) //SETA A PROP DE MODIFICADORES EXCEPCIONAL
    if (prop.Size() > 0) item.SetProp("atributos_excp", prop); endif
endfunction