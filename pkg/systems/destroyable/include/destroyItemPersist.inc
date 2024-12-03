use uo;
use os;

include ":datafile:include/datafile";
include ":merchants:/include/item_template";

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


