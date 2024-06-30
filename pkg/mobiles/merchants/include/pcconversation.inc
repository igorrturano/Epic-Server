use uo;

include ":datafile:include/datafile";

function GetConversationDataFile()
	var data_file := DFOpenDataFile(":pcconversation:npcs", DF_CREATE);
	return data_file;
endfunction

function GetConversationDataElem(elem_name)
	var data_file := GetConversationDataFile();
	var data_elem := DFFindElement(data_file, Hex(elem_name), DF_CREATE);
	
	return data_elem;
endfunction
