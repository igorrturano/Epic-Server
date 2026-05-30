use uo;
use os;
use util;

include ":datafile:include/datafile";
include ":epicevents:Model/ConversationNode";

const CONVERSATION_DATAFILE := ":epicevents:conversations";

// ============================================================================
// DataFile Access
// ============================================================================

function GetConversationDataFile()
	return DFOpenDataFile(CONVERSATION_DATAFILE, DF_CREATE);
endfunction

function GetConversationElem(conversation_id)
	var df := GetConversationDataFile();
	if (!df || df.errortext)
		return 0;
	endif
	return DFFindElement(df, conversation_id);
endfunction

// ============================================================================
// Conversation CRUD
// ============================================================================

function CreateConversation(npc_serial, npc_name, created_by)
	var conversation_id := "conv_" + CStr(polcore().systime) + "_" + CStr(RandomInt(9999));

	var df := GetConversationDataFile();
	if (!df || df.errortext)
		return 0;
	endif

	var elem := DFFindElement(df, conversation_id, DF_CREATE);
	if (!elem || elem.errortext)
		return 0;
	endif

	// Save metadata
	var meta := struct{};
	meta.+npc_serial := npc_serial;
	meta.+npc_name := npc_name;
	meta.+created_by := created_by;
	meta.+created_at := polcore().systime;
	elem.SetProp("meta", meta);

	// Create root node
	var root_id := GenerateNodeId();
	var root_node := ConversationNode(root_id);
	root_node.text := "Dialogo inicial...";
	elem.SetProp("node_" + root_id, root_node.ToStruct());

	elem.SetProp("root_node", root_id);
	elem.SetProp("node_list", array{ root_id });

	return conversation_id;
endfunction

function DeleteConversation(conversation_id)
	var df := GetConversationDataFile();
	if (!df || df.errortext)
		return 0;
	endif
	df.DeleteElement(conversation_id);
	return 1;
endfunction

// ============================================================================
// Node Operations
// ============================================================================

function LoadConversationNode(conversation_id, node_id)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif

	var node_data := elem.GetProp("node_" + node_id);
	if (!node_data)
		return 0;
	endif

	var node := ConversationNode(node_id);
	node.FromStruct(node_data);
	return node;
endfunction

function SaveConversationNode(conversation_id, node)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif

	elem.SetProp("node_" + node.id, node.ToStruct());
	return 1;
endfunction

function CreateNewNode(conversation_id, dialogue_text := "")
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif

	var node_id := GenerateNodeId();
	var node := ConversationNode(node_id);
	node.text := dialogue_text ? dialogue_text : "Novo dialogo...";

	elem.SetProp("node_" + node_id, node.ToStruct());

	// Update node list
	var node_list := elem.GetProp("node_list");
	if (!node_list)
		node_list := array{};
	endif
	node_list.append(node_id);
	elem.SetProp("node_list", node_list);

	return node_id;
endfunction

function DeleteNode(conversation_id, node_id)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif

	// Don't allow deleting root node
	var root := elem.GetProp("root_node");
	if (root == node_id)
		return 0;
	endif

	elem.EraseProp("node_" + node_id);

	// Remove from node list
	var node_list := elem.GetProp("node_list");
	if (node_list)
		var new_list := array{};
		foreach nid in (node_list)
			if (nid != node_id)
				new_list.append(nid);
			endif
		endforeach
		elem.SetProp("node_list", new_list);
	endif

	return 1;
endfunction

// ============================================================================
// Metadata & Utilities
// ============================================================================

function GetConversationMeta(conversation_id)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif
	return elem.GetProp("meta");
endfunction

function GetRootNodeId(conversation_id)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return 0;
	endif
	return elem.GetProp("root_node");
endfunction

function GetNodeList(conversation_id)
	var elem := GetConversationElem(conversation_id);
	if (!elem)
		return array{};
	endif
	var list := elem.GetProp("node_list");
	return list ? list : array{};
endfunction

function GenerateNodeId()
	return "dialogo_" + CStr(RandomInt(99999));
endfunction

function GetNodeSummary(conversation_id, node_id)
	var node := LoadConversationNode(conversation_id, node_id);
	if (!node)
		return "???";
	endif

	var text := node.text;
	if (Len(text) > 30)
		text := text[1, 30] + "...";
	endif
	return text;
endfunction
