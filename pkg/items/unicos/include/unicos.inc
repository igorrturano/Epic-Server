use uo;

include ":datafile:include/datafile";
include "item_template";


//---=[ Unique Items specific functions ]=-----------------------------

function GetItemsDataFile()
	var data_file := DFOpenDataFile(":unicos:lista", DF_CREATE);
	return data_file;
endfunction

function GetItemsDataElem(elem_name)
	var data_file := GetItemsDataFile();
	var data_elem := DFFindElement(data_file, elem_name, DF_CREATE);
	
	return data_elem;
endfunction


// returns the elem name to be used
function ElemName(itemref)
	return (Hex(itemref.serial));
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

function GetItemInfo(itemref)
	var data_elem := GetItemsDataElem(ElemName(itemref));
	
	return DFGetElemProps(data_elem);
endfunction


function AddItemToList(gm, owner, item, desc)
	var data_elem := GetItemsDataElem(ElemName(item));
	SetObjProperty(item, "itemunico", 1);

	var item_cfg := ReadConfigFile(":*:itemdesc");
	
	if( owner.acct )
		data_elem.SetProp("OwnerAcc", owner.acctname);
		data_elem.SetProp("OwnerName", owner.name);
	else
		data_elem.SetProp("Title", owner);
	endif
	data_elem.SetProp("ItemDesc", item.desc);
	data_elem.SetProp("CfgName", item_cfg[item.objtype].name);
	data_elem.SetProp("ID", item.serial);
	data_elem.SetProp("Description", desc);
	data_elem.SetProp("Creator", gm.acctname);
	data_elem.SetProp("Time", polcore().systime);

	return data_elem;
endfunction

//
// Removes an item from the Unique Items list.
//
function RemoveItemOfTheList(itemref)
	var data_file := GetItemsDataFile();

	return data_file.DeleteElement(ElemName(itemref));
endfunction


function RemoveAllofTheList()
	var data_file := GetItemsDataFile();
	var list := GetItemsList();

	foreach elem in list
		data_file.DeleteElement(elem);
	endforeach
endfunction

