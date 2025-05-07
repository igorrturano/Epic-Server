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

/* 
 * PegaListaItens(filtroTempo)
 *
 * Purpose
 *	Busca a lista de itens que foram marcados para ressurgirem.
 * 
 * Parameters
 * filtroTempo: um filtro com um timestamp de partida para a lista.
 *
 * Return value
 *   Um array com um struct dividindo os itens marcados em: 
 *				.+itens // itens marcados
 *				.+itensDeed // um array com os itens de deed agrupados.
 *
 */
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


/* 
 * PegaItemMarcado(itemSerial)
 *
 * Purpose
 *	Busca um item marcado para ressurgir de acordo com o seu serial
 * 
 * Parameters
 * itemSerial: serial do item marcado.
 *
 * Return value
 *   Um struct com os dados do item, sendo:
 *
 *
 *	itemInfo.+serial // serial do objeto marcado.
	itemInfo.+objtype // objtype do objeto marcado
	itemInfo.+deedSerials // uma chave com os serials que fazem parte da deed, caso haja.
	itemInfo.+amount // quantidade
	itemInfo.+template // template do item, caso tenha.
	itemInfo.+x // coordenada x do item
	itemInfo.+y // coordenada y do item
	itemInfo.+z // coordenada z do item
	itemInfo.+movable // se o item esta fixado ou não.
	itemInfo.+realm // o reino do item
	itemInfo.+momento // o momento que foi marcado.
 *
 *
 */
function PegaItemMarcado(itemSerial)
	var elementoDados := GetItemsDataElem(Hex(itemSerial));
	DFGetElemProps(elementoDados);
	
	return MontaStruturaInfo(elementoDados);
endfunction

/* 
 * MontaChaveHex(arrayInt)
 *
 * Purpose
 *	Monta uma chave concatenando um array de int depois de converte-los para hex.
 * 
 * Parameters
 * arrayInt: serial dos itens que comporão a chave.
 *
 * Return value
 *   uma string com os valores concatenados.
 *
 */
function MontaChaveHex(arrayInt)
	var retorno;
		foreach inteiro in arrayInt //monta uma chave para ser usado no dict depois.
			retorno := retorno + Hex(inteiro);
			Sleepms(2);
		endforeach;
	return Cstr(retorno); 
endfunction

/* 
 * MontaStruturaInfo(itemSerial)
 *
 * Purpose
 *	monta os dados de um objeto marcado e colaca num struct.
 * 
 * Parameters
 * itemSerial: serial do item marcado.
 *
 * Return value
 *   Um struct com os dados do item, sendo:
 *
 *
 *	itemInfo.+serial // serial do objeto marcado.
	itemInfo.+objtype // objtype do objeto marcado
	itemInfo.+deedSerials // uma chave com os serials que fazem parte da deed, caso haja.
	itemInfo.+amount // quantidade
	itemInfo.+template // template do item, caso tenha.
	itemInfo.+x // coordenada x do item
	itemInfo.+y // coordenada y do item
	itemInfo.+z // coordenada z do item
	itemInfo.+movable // se o item esta fixado ou não.
	itemInfo.+realm // o reino do item
	itemInfo.+momento // o momento que foi marcado.
 *
 *
 */
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

/* 
 * MarcarRecriarItem(item)
 *
 * Purpose
 *	Guarda as informações de um item dentro de um DataSet para ressurgir no futuro.
 * 
 * Parameters
 * itemSerial: sitem marcado.
 *
 * Return value
 *   sem retorno.
 *
 */
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

/* 
 * RemoveItemMarcado(itemSerial)
 *
 * Purpose
 *	Remove as informações de um que foi marcado para ressurgir no futuro.
 * 
 * Parameters
 * itemSerial: Serial do item marcado.
 *
 * Return value
 *   DataElement do item removido.
 *
 */
function RemoveItemMarcado(itemSerial)
	var data_file := GetItemsDataFile();

	return data_file.DeleteElement(Hex(itemSerial));
endfunction

/* 
 * RemoveItemMarcado(itemSerial)
 *
 * Purpose
 *	Abre o log xml dos itens que foram destruidos.
 * 
 * Parameters
 * sem parâmetro.
 *
 * Return value
 *   retorna a estrutura XML.
 *
 */
function PegaLogXMLDestruiveis()
	return AbreLogXml(LOG_XML_NOME);
endfunction

/* 
 * ListaLogItemDestruido(quem,filtroRegiao,filtroTempo)
 *
 * Purpose
 *	lista os registros do log dos intens destruídos.
 * 
 * Parameters
 * quem: O ator que requisitou a lista.
 * filtroRegiao: um raio de uma cordenada para filtrar os mais próximos no log.
 * filtroTempo: um filtro ponto de partida no tempo.
 *
 * Return value
 *   conjuntoResultado um array com os registros do log, sendo:
 *				
 *				registro.+quando // momento que foi destruido
 *				registro.+x // coordenada x do ponto que foi destruido.
 *				registro.+y // coordenada y do ponto que foi destruido.
 *				registro.+z // coordenada z do ponto que foi destruido.
 *				registro.+quem // Quem destruíu, pode ser npc ou player.
 *				registro.+oque // A desc do item que foi destruído
 *				registro.+como // A desc da fonte de dano.
 *				registro.+isReconstruivel // 1 se foi marcado para retornar.
 *
 */
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
				if(POLCore().systime - filtroTempo >= quandoAFiltrar)
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

/* 
 * ExtraiDado(xmlNode)
 *
 * Purpose
 *	lExtrai o dado de um xmlnode.
 * 
 * Parameters
 * xmlNode: o nó xml
 *
 * Return value
 *   uma string com o valor da tag do nóxml.
 *
 */
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

/* 
 * ExtraiDado(byref pai,atributos,chave,valores := {})
 *
 * Purpose
 *	Adiciona um nó na arvore do xml.
 * 
 * Parameters
 * pai: O nó pai do nó que foi informado.
 * atributos: array de atributos que será inserido na tag do nó.
 * chave: Uma string que identificará o nó.
 * valores: um array com o valor dentro da tag // fiz assim pq queria que fosse opcional, mas sem precisar de um segundo parametro pra informar.
 *
 * Return value
 *   uma string com o valor da tag do nóxml.
 *
 */
function AdicionaXmlNode(byref pai,atributos,chave,valores := {})
	var no := pai.appendxmlnode(chave,atributos);

	foreach valor in valores // fiz com array por que não vi um jeito de fazer com um if sem precisar de mais de 1 parametro.
		no.appendxmltext(CSTr(valor));
		Sleepms(1);
	endforeach

	return no;
endfunction

/* 
 * InsereLogItemDestruido(byref pai,atributos,chave,valores := {})
 *
 * Purpose
 *	Insere um registro de item destruído no xml de log.
 * 
 * Parameters
 * quem: char ou npc que destruiu o item
 * quando: timestamp do momento que foi destruído.
 * x: coordenada x do ponto onde item foi destruído.
 * y: coordenada y do ponto onde item foi destruído.
 * z: coordenada z do ponto onde item foi destruído.
 * oque: desc do item que foi destruído.
 * como: desc da fonte de dano.
 * isReconstruivel.: 1 se foi marcado para ressurgir.
 *
 * Return value
 *   1 se foi inserido com sucesso.
 *
 */
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

/* 
 * LimpaLogItemDestruido()
 *
 * Purpose
 *	remove todo os registros do log.
 * 
 * Parameters
 * sem parâmetros.
 *
 * Return value
 *  sem retorno.
 *
 */
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

/* 
 * CriaLogXml(nomeArquivoXml)
 *
 * Purpose
 *	Cria um arquivo de log em xml.
 * 
 * Parameters
 * nomeArquivoXml: path do arquvivo.
 *
 * Return value
 *  referência do arquivo criado, ou erro.
 *
 *
 */
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

/* 
 * CriaLogXml(nomeArquivoXml, flag := DF_CREATE)
 * Purpose
 *	Abre ou cria um arquivo XML
 * 
 * Parameters
 * nomeArquivoXml: path do arquvivo.
 * flag: Flag se deve criar o arquivo caso não exista..
 *
 * Return value
 *  referência do arquivo , ou erro.
 *
 *
 */
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

