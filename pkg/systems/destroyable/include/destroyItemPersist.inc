use uo;
use os;

include ":datafile:include/datafile";
include ":merchants:/include/item_template";
include "/include/utils";

enum DESTRUIVEIS_NOMES_LOG
	TUPLA := "tupla",
	CAMPO_QUANDO := "campo_quando",
	CAMPO_QUANDO_FORMATADO := "campo_quando_formatado",
	CAMPO_QUANDO_TIMESTAMP := "campo_quando_timestamp",
	CAMPO_QUEM := "campo_quem",
	CAMPO_ONDE := "campo_onde",
	CAMPO_ONDE_X := "x",
	CAMPO_ONDE_Y := "y",
	CAMPO_ONDE_Z := "z",
	CAMPO_ONDE_REALM := "realm",
	CAMPO_COMO := "campo_como",
	CAMPO_OQUE := "campo_oque",
	CAMPO_RECONSTRUIVEL := "campo_reconstruivel",
endenum

const LOG_XML_NOME := "::data/log/logDestruiveis.xml";

function GetItemsDataFile()
	var data_file := DFOpenDataFile(":destroyable:recriar", DF_CREATE);
	return data_file;
endfunction

function GetItemsDataElem(elem_name)
	var data_file := GetItemsDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction

function PegaListaItens(filtroTempo)
	var data_file := GetItemsDataFile();
	var data_list := DFGetElemNames(data_file);

	var itens := array{};
	var itensDeed := dictionary;
	var retorno := struct;

	foreach elemento in data_list
		var elementoDados :=  DFFindElement(data_file,elemento);
		var info := MontaStruturaInfo(elementoDados);
		if(info.momento <= (polcore().systime - filtroTempo))
			if(info.deedSerials) //se faz parte de uma deed
				var arrayDeedItens := array;
				if(itensDeed[info.deedSerials]) // Se for parte de uma deed conhecida, pega o array para adicionar mais este.
					arrayDeedItens :=  itensDeed[info.deedSerials];
					arrayDeedItens.append(info);
				else //se não, cria um novo array.
					arrayDeedItens.append(info);
				endif
				var ret := itensDeed.insert(info.deedSerials,arrayDeedItens);
			else
				itens.append(info);
			endif
		endif
		Sleepms(4);
	endforeach

	retorno.+itens := itens;
	retorno.+itensDeed := itensDeed;
	return retorno;
endfunction

function PegaItemMarcado(itemSerial)
	var elementoDados := GetItemsDataElem(Hex(itemSerial));
	DFGetElemProps(elementoDados);
	
	return MontaStruturaInfo(elementoDados);
endfunction

function MontaChaveHex(arrayInt)
	var retorno;
		foreach inteiro in arrayInt //monta uma chave para ser usado no dict depois.
			retorno := retorno + Hex(inteiro);
			Sleepms(2);
		endforeach;
	return Cstr(retorno); 
endfunction

function MontaStruturaInfo(elemento)
	var itemInfo := struct;
	itemInfo.+serial := elemento.GetProp("serial");
	itemInfo.+objtype := elemento.GetProp("objtype");
	var deedSerials := elemento.GetProp("deedSerials");
	if(deedSerials)
		itemInfo.+deedSerials := MontaChaveHex(deedSerials);//monta uma chave para ser usado no dict depois.
	endif
	itemInfo.+amount := elemento.GetProp("amount");
	itemInfo.+template := elemento.GetProp("template");
	itemInfo.+x := elemento.GetProp("x");
	itemInfo.+y := elemento.GetProp("y");
	itemInfo.+z := elemento.GetProp("z");
	itemInfo.+movable := elemento.GetProp("movable");
	itemInfo.+realm := elemento.GetProp("realm");
	itemInfo.+momento := elemento.GetProp("momento");
	return itemInfo;
endfunction

function MarcarRecriarItem(item)
	var data_elem := GetItemsDataElem(Hex(item.serial));
	if(data_elem)
		if(item.ISA(POLCLASS_ITEM))
			var template := CreateItemTemplate(item);
			data_elem.SetProp("serial",item.serial);
			data_elem.SetProp("objtype",item.objtype);
			if(GetObjProperty(item,"ItemsCreatedSerials"))
				data_elem.setprop("deedSerials",GetObjProperty(item,"ItemsCreatedSerials"));
			endif
			data_elem.SetProp("amount",item.amount);
			data_elem.SetProp("template",template);
			data_elem.SetProp("qtd",item.amount);
			data_elem.SetProp("x",item.x);
			data_elem.SetProp("y",item.y);
			data_elem.SetProp("z",item.z);
			data_elem.SetProp("movable",item.movable);
			data_elem.SetProp("realm",item.realm);
			data_elem.SetProp("momento", polcore().systime);
		endif
	endif
endfunction

function RemoveItemMarcado(itemSerial)
	var data_file := GetItemsDataFile();

	return data_file.DeleteElement(Hex(itemSerial));
endfunction

function PegaLogXMLDestruiveis()
	return AbreLogXml(LOG_XML_NOME);
endfunction

function ListaLogItemDestruido(quem,filtroRegiao := 0,filtroTempo := 0)
	var xml := PegaLogXMLDestruiveis();
	var conjuntoResultado := array;
	
	if(xml)
		foreach linha in xml
			if(_linha_iter == 1)
				continue;
			endif
			Sleepms(1);
			
			var tupla := struct;
			var quandoAFiltrar := ExtraiDado(linha[CAMPO_QUANDO][CAMPO_QUANDO_TIMESTAMP]);
			if(filtroTempo)
				if(POLCore().systime - filtroTempo >= POLCore().systime)
					continue;
				endif
			endif

			tupla.+quando := ExtraiDado(linha[CAMPO_QUANDO][CAMPO_QUANDO_FORMATADO]);
			tupla.+x := ExtraiDado(linha[CAMPO_ONDE][CAMPO_ONDE_X]);
			tupla.+y := ExtraiDado(linha[CAMPO_ONDE][CAMPO_ONDE_Y]);
			tupla.+z := ExtraiDado(linha[CAMPO_ONDE][CAMPO_ONDE_Z]);
			tupla.+realm := ExtraiDado(linha[CAMPO_ONDE][CAMPO_ONDE_REALM]);

			If(filtroRegiao)
				var distanciaEuclidiana := Sqrt( Pow(tupla.x - quem.x,2) + Pow(tupla.y - quem.y,2) + Pow(tupla.z - quem.z,2));
				if(distanciaEuclidiana > filtroRegiao)
					continue;
				endif
			endif

			var quemObj := SystemFindObjectBySerial( ExtraiDado(linha[CAMPO_QUEM]), SYSFIND_SEARCH_OFFLINE_MOBILES );
			if(quemObj.acctname)
				tupla.+quem := quemObj.acctname;
			else
				tupla.+quem := quemObj.name + "[NPC]";
			endif
			tupla.+oque := ExtraiDado(linha[CAMPO_OQUE]);
			tupla.+como := ExtraiDado(linha[CAMPO_COMO]);
			tupla.+isReconstruivel := ExtraiDado(linha[CAMPO_RECONSTRUIVEL]);

			conjuntoResultado.Append(tupla);
			Sleepms(1);
		endforeach
	else
		return conjuntoResultado;
	endif
	conjuntoResultado.reverse();
	return conjuntoResultado;
endfunction

function ExtraiDado(xmlNode)
	var atributos := xmlNode.attributes;
	if(atributos["tipo"] == "String")
		return  Cstr(xmlNode.firstxmlchild());
	elseif(atributos["tipo"] == "Integer")
		return  CInt(Cstr(xmlNode.firstxmlchild()));
	else
		return  Cstr(xmlNode.firstxmlchild());
	endif
endfunction

function AdicionaXmlNode(byref pai,atributos,chave,valores := {})
	var no := pai.appendxmlnode(chave,atributos);

	foreach valor in valores // fiz com array por que não vi um jeito de fazer com um if sem precisar de mais de 1 parametro.
		no.appendxmltext(CSTr(valor));
		Sleepms(1);
	endforeach

	return no;
endfunction

function InsereLogItemDestruido(quem,quando,x,y,z,realm,oque,como,isReconstruivel)
	var xml := PegaLogXMLDestruiveis();
	if(xml)
		var entradaLog := AdicionaXmlNode(xml,struct{titulo := "linha"},TUPLA);
		if(entradaLog)
			
			var noQuando := AdicionaXmlNode(entradaLog,struct{titulo := "envelope"},CAMPO_QUANDO);
			AdicionaXmlNode(noQuando,struct{tipo := "String"},CAMPO_QUANDO_FORMATADO,{FormatarData(quando)});
			AdicionaXmlNode(noQuando,struct{tipo := "Integer"},CAMPO_QUANDO_TIMESTAMP,{quando});
			AdicionaXmlNode(entradaLog,struct{tipo := "Integer"},CAMPO_QUEM,{quem.serial});
			var noOnde := AdicionaXmlNode(entradaLog,struct{titulo := "envelope"},CAMPO_ONDE);
			AdicionaXmlNode(noOnde,struct{tipo := "Integer"},CAMPO_ONDE_X,{x});
			AdicionaXmlNode(noOnde,struct{tipo := "Integer"},CAMPO_ONDE_Y,{y});
			AdicionaXmlNode(noOnde,struct{tipo := "Integer"},CAMPO_ONDE_Z,{z});
			AdicionaXmlNode(noOnde,struct{tipo := "String"},CAMPO_ONDE_REALM,{realm});
			AdicionaXmlNode(entradaLog,struct{tipo := "String"},CAMPO_COMO,{como});
			AdicionaXmlNode(entradaLog,struct{tipo := "String"},CAMPO_OQUE,{oque});
			AdicionaXmlNode(entradaLog,struct{tipo := "Integer"},CAMPO_RECONSTRUIVEL,{isReconstruivel});
			
			xml.savexml(LOG_XML_NOME);
		endif
		return 1;
	else
		return 0;
	endif
endfunction

function LimpaLogItemDestruido()
	var xml := PegaLogXMLDestruiveis();
	if(xml)
		foreach linha in xml
			if(_linha_iter == 1)
				continue;
			endif
			var foi := xml.removexmlnode(1);
			Sleepms(2);
		endforeach
		xml.savexml(LOG_XML_NOME);
	endif
endfunction

function CriaLogXml(nomeArquivoXml);
	var arquivoXml := CreateXMLFile();
	var msgErro := error;
	msgErro.+errortext := "";

	if(arquivoXml)
		arquivoXml.setxmldeclaration("1.0", "UTF-8", "no");
		if(!arquivoXml.savexml(nomeArquivoXml))
			msgErro.errortext := "Error::CriaLogXml() - Não foi possível salvar XML " + nomeArquivoXml + " : " + arquivoXml.errortext;
			DF_OutPut(msgErro.errortext);
			Print(msgErro.errortext);
			return msgErro;
		else
			return arquivoXml;
		endif
	else
		msgErro.errortext := "Error::CriaLogXml() - Não foi possível criar objeto XML " + nomeArquivoXml + " : " + arquivoXml.errortext;
		DF_OutPut(msgErro.errortext);
		Print(msgErro.errortext);
	endif
	return msgErro;
endfunction

function AbreLogXml (nomeArquivoXml, flag := DF_CREATE)
	var objetoXml;
	if(FileExists(nomeArquivoXml))
		objetoXml := OpenXMLFile(nomeArquivoXml);
		return objetoXml;
	else
		if(flag == DF_CREATE)
			return CriaLogXml(nomeArquivoXml);
		else
			var msgErro := error;
			msgErro.errortext := "Error::CriaLogXml() - Não foi possível abrir XML " + nomeArquivoXml + " O arquivo não existe.";
			DF_OutPut(msgErro.errortext);
			Print(msgErro.errortext);
		endif
	endif
endfunction

