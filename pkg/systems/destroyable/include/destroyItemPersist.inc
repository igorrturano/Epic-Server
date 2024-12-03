use uo;

include ":datafile:include/datafile";
include ":merchants:/include/item_template";


//---=[ Unique Items specific functions ]=-----------------------------

function GetItemsDataFile()
	var data_file := DFOpenDataFile(":destroyable:recriar", DF_CREATE);
	return data_file;
endfunction

function GetItemsDataElem(elem_name)
	var data_file := GetItemsDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction


//
// Returns a list of strings in reference to unique items.
//
function GetItemsList()
	var data_file := GetItemsDataFile();
	var data_list := DFGetElemNames(data_file);

	//Reversed so that LastNum is more likely to be found first
	//and makes newer items show up at the top, and older items further down.
	data_list.Reverse();

	return data_list;
endfunction

function PegaItemMarcado(itemSerial)
	var data_elem := GetItemsDataElem(Hex(itemSerial));
	DFGetElemProps(data_elem);
	var itemInfo := struct;
	itemInfo.+template := data_elem.GetProp("template");
	itemInfo.+x := data_elem.GetProp("x");
	itemInfo.+y := data_elem.GetProp("y");
	itemInfo.+z := data_elem.GetProp("z");
	itemInfo.+movable := data_elem.GetProp("movable");
	itemInfo.+realm := data_elem.GetProp("realm");
	itemInfo.+momento := data_elem.GetProp("momento");
	return itemInfo;
endfunction

function MarcarRecriarItem(item)
	var data_elem := GetItemsDataElem(Hex(item.serial));
	var template := CreateItemTemplate(item);
	data_elem.SetProp("template",template);
	data_elem.SetProp("qtd",item.amount);
	data_elem.SetProp("x",item.x);
	data_elem.SetProp("y",item.y);
	data_elem.SetProp("z",item.z);
	data_elem.SetProp("movable",item.movable);
	data_elem.SetProp("realm",item.realm);
	data_elem.SetProp("momento", polcore().systime);
endfunction

//
// Removes an item from the Unique Items list.
//
function RemoveItemOfTheList(itemSerial)
	var data_file := GetItemsDataFile();

	return data_file.DeleteElement(Hex(itemSerial));
endfunction


function RemoveAllofTheList()
	var data_file := GetItemsDataFile();
	var list := GetItemsList();

	foreach elem in list
		data_file.DeleteElement(elem);
	endforeach
endfunction

