use uo;
use attributes;
use vitals;

include "include/client";
include ":attributes:skills";
include ":datafile:datafile";
include ":attributes:attributes";
include ":attributes:stats";
include ":gumps:requestGump";
include ":tn:equipment";

function possess (me, npc)
	if (GetObjProperty (me, "lastpossessedserial"))
		var oldnpc := SystemFindObjectBySerial(GetObjProperty(me, "lastpossessedserial"));
		if (oldnpc)
			unpossess (me);
		endif
	endif

	var oldbody := CreateNpcFromTemplate(":epicai:mannequin", npc.x, npc.y, npc.z, 0, npc.realm);
	MoveObjectToLocation (oldbody, 5385, 923, 0, me.realm, MOVEOBJECT_FORCELOCATION);
	SetObjProperty(me, "oldmeserial", oldbody.serial);
	SetObjProperty(me, "lastpossessedserial", npc.serial);

	clonar(oldbody, me);
	foreach iteminbag in EnumerateItemsInContainer(me.backpack, ENUMERATE_ROOT_ONLY)
		MoveItemToContainer(iteminbag, oldbody.backpack);
		sleepms(10);
	endforeach

	clonar(me, npc);
	DestroyItem(me.backpack);
	var bolsa := CreateItemAtLocation(5384, 923, 0, 0x9B2);
	EquipItem(me, bolsa);
	foreach iteminbag in EnumerateItemsInContainer(npc.backpack, ENUMERATE_ROOT_ONLY)
		MoveItemToContainer(iteminbag, me.backpack);
		sleepms(10);
	endforeach

	npc.concealed := 1;
	me.concealed := 0;
	me.facing := npc.facing;
	MoveObjectToLocation (me, npc.x, npc.y, npc.z, npc.realm, MOVEOBJECT_FORCELOCATION);
	MoveObjectToLocation (npc, 5383, 923, 0, npc.realm, MOVEOBJECT_FORCELOCATION);
	npc.frozen := 1;
	oldbody.frozen := 1;
	me.Disable("invul");
	me.Disable("losany");

	if ((GetObjProperty(npc,"npchero"))!=error)
		SetObjProperty(me,"npchero",1);
	endif

	return 1;
endfunction

function unpossess(me, freeze :=1, allowMissingPossessed := 0)
	var oldnpcserial := getObjProperty(me, "lastpossessedserial");
	var oldnpc := SystemFindObjectBySerial(oldnpcserial);
	var oldbodyserial := getObjProperty(me, "oldmeserial");
	var oldbody := SystemFindObjectBySerial(oldbodyserial);
	var isGolemPilot := 0;

	if (!oldbody)
		return 0;
	endif

	if (!oldnpc && !allowMissingPossessed)
		return 0;
	endif

	if (oldnpc)
		oldnpc.frozen := 0;
		if (GetObjProperty(me, "pilotando") != error && oldnpc.npctemplate == ":epicai:PowerArmor")
			isGolemPilot := 1;
		endif
	endif

	// Erase pilotando early — canInsert.src blocks MoveItemToContainer while it's set
	EraseObjProperty(me, "pilotando");

	// Restore possessed NPC (skip for golem pilots — golem stays as-is)
	if (oldnpc)
		if (isGolemPilot)
			oldnpc.facing := me.facing;
			MoveObjectToLocation(oldnpc, me.x, me.y, me.z, me.realm, MOVEOBJECT_FORCELOCATION);
			oldnpc.concealed := 0;
			oldnpc.frozen := 0;
			me.concealed := 1;
		else
			var container := CreateItemAtLocation(5384, 923, 0, "backpack", 1, oldnpc.realm);
			container.invisible := 1;
			foreach item in ( EnumeratePhysicalItems(oldnpc) )
				MoveItemToContainer(item, container);
				SleepMS(2);
			endforeach
			DestroyItem(container);

			clonar(oldnpc, me);

			DestroyItem(oldnpc.backpack);
			var bolsa := CreateItemAtLocation(5383, 923, 0, 0x9B2);
			EquipItem(oldnpc, bolsa);
			foreach iteminbag in EnumerateItemsInContainer(me.backpack, ENUMERATE_ROOT_ONLY)
				MoveItemToContainer(iteminbag, oldnpc.backpack);
				sleepms(10);
			endforeach
			oldnpc.facing := me.facing;
			MoveObjectToLocation (oldnpc, me.x, me.y, me.z, me.realm, MOVEOBJECT_FORCELOCATION);
			oldnpc.frozen := 1;
			oldnpc.concealed := 0;
			me.concealed := 1;
		endif
	endif

	// Restore player from oldbody backup
	var container := CreateItemAtLocation(5383, 923, 0, "backpack", 1, me.realm);
	container.invisible := 1;
	foreach item in ( EnumeratePhysicalItems(me) )
		MoveItemToContainer(item, container);
		SleepMS(2);
	endforeach
	DestroyItem(container);

	clonar(me, oldbody);
	DestroyItem(me.backpack);
	var bolsa := CreateItemAtLocation(5383, 923, 0, 0x9B2);
	EquipItem(me, bolsa);
	foreach iteminbag in EnumerateItemsInContainer(oldbody.backpack, ENUMERATE_ROOT_ONLY)
		MoveItemToContainer(iteminbag, me.backpack);
		sleepms(10);
	endforeach

	SetObjProperty(oldbody, "GuardKill", 1);
	ApplyRawDamage(oldbody, 6000);
	destroyitem(oldbody);

	me.enable("invul");
	me.enable("losany");

	if (oldnpc)
		oldnpc.frozen := 0;
	endif

	AP_RefreshVitals(me);
	EraseObjProperty(me, "npchero");
	EraseObjProperty(me, "oldmeserial");
	EraseObjProperty(me, "lastpossessedserial");
	EraseObjProperty(me, "unattack");
	return 1;
endfunction


function clonar(newlook, oldlook)
	var chardata:=dictionary;
	var valor;
	var mod;
	var mypack := GetEquipmentByLayer(newlook, 0x15);
	if (!mypack)
		mypack := CreateItemAtLocation(5383, 923, 0, 0xe75, 1, newlook.realm);
	else
		MoveObjectToLocation(mypack, 5383, 923, 0, newlook.realm, MOVEOBJECT_FORCELOCATION);
	endif

	var itemlist := ListEquippedItems(newlook);
	var y;

	foreach attrib_name in AP_ListAttributesByType()
		valor:= GetAttribute( oldlook, attrib_name );
		mod := AP_GetStatMod( oldlook, attrib_name ) * -1;
		valor := valor + mod;
		AP_SetTrueSkill(newlook, attrib_name, valor);
		sleepms(10);
	endforeach

	var newhits := CInt(GetVital(oldlook, "hits"));
	var newmana := CInt(GetVital(oldlook, "mana"));
	if ( newhits == GetVitalMaximumValue(oldlook, "hits") )
		newhits := CInt(GetVitalMaximumValue(newlook, "hits"));
	endif
	if ( newmana == GetVitalMaximumValue(oldlook, "mana") )
		newmana := CInt(GetVitalMaximumValue(newlook, "mana"));
	endif
	SetVital(newlook, "hits", newhits);
	SetVital(newlook, "stamina", CInt(GetVitalMaximumValue(newlook, "stamina")));
	SetVital(newlook, "mana", newmana);

	foreach x in itemlist
		MoveItemToContainer(x, mypack);
	endforeach

	itemlist := ListEquippedItems(oldlook);
	foreach x in itemlist
		y := CreateItemCopyAtLocation(5383, 923, 0, x);
		y.color := x.color;
		y.name := x.name;
		y.desc := x.desc;
		EquipItem(newlook, y);
	endforeach

	var desc_oldlook:=GetObjProperty(oldlook,"description");

	if (desc_oldlook!=error)
		SetObjProperty(newlook,"description",desc_oldlook);
	else
		EraseObjProperty(newlook,"description");
	endif

	if(newlook.ip)
		SetObjProperty(newlook,"raca",oldlook.title_race);
	endif

	newlook.gender := oldlook.gender;
	newlook.graphic := oldlook.graphic;
	newlook.color := oldlook.color;
	newlook.title_race:=oldlook.title_race;
	newlook.title_suffix:=oldlook.title_suffix;
	newlook.title_prefix:=oldlook.title_prefix;
	newlook.name := oldlook.name;
	newlook.TrueColor := oldlook.TrueColor;
	newlook.trueobjtype := oldlook.trueobjtype;

	if (newlook.ip)
		chardata:=GetObjProperty(newlook,"chardata");
		chardata["raca"]:=newlook.title_race;
	endif

	//Copia o profile do npc.
	var cp_data_file_oldlook  := DFOpenDataFile(":charprofile:CPFile", DF_CREATE);
	var cp_elem_oldlook       := DFFindElement(cp_data_file_oldlook, oldlook.serial, DF_CREATE);
	var cp_profile_oldlook    := DFGetProp(cp_elem_oldlook, "Profile", DF_CREATE);

	var cp_data_file_newlook  := DFOpenDataFile(":charprofile:CPFile", DF_CREATE);
	var cp_elem_newlook       := DFFindElement(cp_data_file_newlook, newlook.serial, DF_CREATE);
	var cp_profile_newlook    := DFGetProp(cp_elem_newlook, "Profile", DF_CREATE);

	cp_elem_newlook.SetProp("Profile", cp_profile_oldlook);

	var newpack := GetEquipmentByLayer(newlook, 0x15);
	if (newpack)
		DestroyItem(newpack);
	endif
	EquipItem(newlook, mypack);

	if ((GetObjProperty(oldlook,"classe"))!=error)
		SetObjProperty(newlook,"classe",GetObjProperty(oldlook,"classe"));
	endif

	if ((GetObjProperty(oldlook,"kit"))!=error)
		SetObjProperty(newlook,"kit",GetObjProperty(oldlook,"kit"));
	endif

	if ((GetObjProperty(oldlook,"myhabs"))!=error)
		SetObjProperty(newlook,"myhabs",GetObjProperty(oldlook,"myhabs"));
	endif

	if ((GetObjProperty(oldlook,"buyhab"))!=error)
		SetObjProperty(newlook,"buyhab",GetObjProperty(oldlook,"buyhab"));
	endif

	return 1;
endfunction