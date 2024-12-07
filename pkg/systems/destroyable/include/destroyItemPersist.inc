use uo;
use os;

include ":datafile:include/datafile";
include ":merchants:/include/item_template";

enum DESTRUIVEIS_NOMES_LOG
	TUPLA := "tupla",
	CAMPO_QUANDO := "campo_quando",
	CAMPO_QUEM := "campo_quem",
	CAMPO_ONDE_X := "campo_onde_x",
	CAMPO_ONDE_Y := "campo_onde_y",
	CAMPO_ONDE_Z := "campo_onde_z",
	CAMPO_ONDE_REALM := "campo_onde_realm",
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

	var conjuntoDados := array{};

	foreach elemento in data_list
		var elementoDados :=  DFFindElement(data_file,elemento);
		var info := MontaStruturaInfo(elementoDados);
		if(info.momento <= (polcore().systime - filtroTempo))
			conjuntoDados.append(info);
		endif
		Sleepms(4);
	endforeach

	return conjuntoDados;
endfunction

function PegaItemMarcado(itemSerial)
	var elementoDados := GetItemsDataElem(Hex(itemSerial));
	DFGetElemProps(elementoDados);
	
	return MontaStruturaInfo(elementoDados);
endfunction

function MontaStruturaInfo(elemento)
	var itemInfo := struct;
	itemInfo.+serial := elemento.GetProp("serial");
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
	var template := CreateItemTemplate(item);
	data_elem.SetProp("serial",item.serial);
	data_elem.SetProp("template",template);
	data_elem.SetProp("qtd",item.amount);
	data_elem.SetProp("x",item.x);
	data_elem.SetProp("y",item.y);
	data_elem.SetProp("z",item.z);
	data_elem.SetProp("movable",item.movable);
	data_elem.SetProp("realm",item.realm);
	data_elem.SetProp("momento", polcore().systime);
endfunction

function RemoveItemMarcado(itemSerial)
	var data_file := GetItemsDataFile();

	return data_file.DeleteElement(Hex(itemSerial));
endfunction

function PegaLogXMLDestruiveis()
	return AbreLogXml(LOG_XML_NOME);
endfunction

function ListaLogItemDestruido()
	var xml := PegaLogXMLDestruiveis();
	var conjuntoResultado := array;
	
	if(xml)
		foreach linha in xml
			if(_linha_iter == 1)
				continue;
			endif
			
			var tupla := struct;

			tupla.+quando := ExtraiDado(linha[CAMPO_QUANDO]);
			tupla.+x := ExtraiDado(linha[CAMPO_ONDE_X]);
			tupla.+y := ExtraiDado(linha[CAMPO_ONDE_Y]);
			tupla.+z := ExtraiDado(linha[CAMPO_ONDE_Z]);
			tupla.+realm := ExtraiDado(linha[CAMPO_ONDE_REALM]);
			tupla.+quem := ExtraiDado(linha[CAMPO_QUEM]);
			tupla.+oque := ExtraiDado(linha[CAMPO_OQUE]);
			tupla.+como := ExtraiDado(linha[CAMPO_COMO]);
			tupla.+isReconstruivel := ExtraiDado(linha[CAMPO_RECONSTRUIVEL]);

			conjuntoResultado.Append(tupla);
			Sleepms(2);
		endforeach
	else
		return conjuntoResultado;
	endif
	return conjuntoResultado;
endfunction

function ExtraiDado(xmlNode)
	var atributos := xmlNode.atrributtes;
	if(atributos["tipo"] == "String")
		return  Cstr(xmlNode.firstxmlchild());
	elseif(atributos["tipo"] == "Integer")
		return  CInt(xmlNode.firstxmlchild());
	else
		return  Cstr(xmlNode.firstxmlchild());
	endif
endfunction

function InsereLogItemDestruido(quem,quando,x,y,z,realm,oque,como,isReconstruivel)
	var xml := PegaLogXMLDestruiveis();
	if(xml)
		var entradaLog := xml.appendxmlnode(TUPLA,struct{titulo := "linha"});
		if(entradaLog)
			var no := entradaLog.appendxmlnode(CAMPO_QUANDO,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(quando));

			no := entradaLog.appendxmlnode(CAMPO_ONDE_X,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(x));
			
			no := entradaLog.appendxmlnode(CAMPO_ONDE_Y,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(y));
			
			no := entradaLog.appendxmlnode(CAMPO_ONDE_Z,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(z));
			
			no := entradaLog.appendxmlnode(CAMPO_ONDE_REALM,struct{tipo := "String"});
			no.appendxmltext(CSTr(realm));
			
			no := entradaLog.appendxmlnode(CAMPO_QUEM,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(quem.serial));
			
			no := entradaLog.appendxmlnode(CAMPO_COMO,struct{tipo := "String"});
			no.appendxmltext(CSTr(como));

			no := entradaLog.appendxmlnode(CAMPO_OQUE,struct{tipo := "String"});
			no.appendxmltext(CSTr(oque));
			
			no := entradaLog.appendxmlnode(CAMPO_RECONSTRUIVEL,struct{tipo := "Integer"});
			no.appendxmltext(CSTr(isReconstruivel));
			
			xml.savexml(LOG_XML_NOME);
		endif
		return 1;
	else
		return 0;
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

