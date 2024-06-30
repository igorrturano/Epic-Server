//
// 1.1
//
// CraftHelp.inc
//
// Helper functions commonly used in craft related scripts.
// Helps to remove redundancy.
//
// **************************************************************************
// * FindTool(mobile, Array of obj types)
// * AmountInPack(mobile, obj_type)
// * GetGraphic(obj_type)
// * GetName(obj_type)
// **************************************************************************
//
// Created  9/05/2001 8:00AM
// Modified 04/27/2002 9:49AM
//
//

use uo;
use os;
use util;
use cfgfile;


//
//  LeftOvers()
//
//  Reads the itemdesc.cfg to see if the item gives leftovers.
//

function LeftOvers(who, byref cfg_elem)
	var left_overs := GetConfigStringArray(cfg_elem, "LeftOvers");
	
	foreach line in left_overs
		line := SplitWords(line);
		if ( len(line) >= 2 )
			var item := line[1];
			if ( GetObjTypeByName(item) )
				item := GetObjTypeByName(item);
			else
				item := CInt(item);
			endif
			var amount := CInt(line[2]);
				
			var created := CreateItemInBackPack(who, item, amount);
			if ( !created )
				for ( amount; amount>0; amount:=amount-1 ) 
					// For non-stackable items
					created := CreateItemInBackPack(who, item, 1);
					if ( !created )
						break;
					endif
					sleepms(2);
				endfor
			endif
		
			if( created.errortext )
				SendSysMessageEX(who, "LeftOvers:"+created.errortext);
			endif
			sleepms(2);
		endif
	endforeach
endfunction

//
//  FindTool()
//
//  * Returns an item reference if a tool is found. 0 if not.
//
function FindTool(mobile, tool_list)
	if ( mobile.cmdlevel >= CMDLEVEL_GM )
		SendSysMessageEX(mobile, "Checking pack...", SSM_INFO_DEBUG);
	endif
	foreach item in EnumerateItemsInContainer(mobile.backpack)
		if ( CStr(item.objtype) in tool_list )
			return item;
		endif
	endforeach
	
	if ( mobile.cmdlevel >= CMDLEVEL_GM )
		SendSysMessageEX(mobile, "Checking non-static items nearby...", SSM_INFO_DEBUG);
	endif
	foreach item in ListItemsNearLocation(mobile.x, mobile.y, mobile.z, 5)
		if ( CheckLineOfSight(mobile, item) )
			if ( CStr(item.objtype) in tool_list )
				return item;
			endif
		endif
		sleepms(2);
	endforeach
	return 0;
endfunction

//
//  AmountInPack()
//
//  * Returns the total amount of the ObjType 
//    passed in the mobile's pack.
//
function AmountInPack(mobile, obj_type, color:=0)
	var amount := 0;
	foreach item in EnumerateItemsInContainer(mobile.backpack)
		if( item.objtype == obj_type )
			if ( !color || item.color == color )
				amount := amount + item.amount;
			endif
		endif
		sleepms(2);
	endforeach
	
	return amount;
endfunction

//
//  GetGraphic()
//
//  * Returns the graphic of the objtype you pass.
//
function GetGraphic(obj_type)
	obj_type := cint(obj_type);
	var config := ReadConfigFile(":*:itemdesc");
	config := config[obj_type];
	
	if (config.graphic)
		return config.graphic;
	else
		return obj_type;
	endif
endfunction

//
//  GetName()
//
//  * Returns the name for the item of the objtype passed.
//
function GetName(obj_type, add_S := 0)
	obj_type := CInt(obj_type);
	
	var config := ReadConfigFile(":*:itemdesc");
	config := config[obj_type];
	
	var name;
	if (config.desc)
		name := config.desc;
	elseif (config.name)
		name := config.name;
	else
		return "[error: Item not defined obj_type is " + hex(obj_type) + "]";
	endif

	if ( !name["%"] )
		if ( add_S == 0 )
			return name;
		endif
		if ( name[len(name)] in array{"s"} )
			return name;
		elseif ( name[len(name)] in array{"h","j","x","z"} )
			return name + "es";
		endif
		return name + "s";
	endif

	// Will deal with:
	//
	//	Bread Stick%s
	//	Bread Loa%ves/f%
	//	Lump%s% of Coal
	//	Loa%ves/f% of Bread
	//
	var first := find(name, "%", 1);
	var slash := find(name, "/", first);
	var second := find(name, "%", first + 1);
	if ( slash && !second )
		// Force-append a % if a '/' exists!
		name := name + "%";
		second := len(name);
	elseif ( !slash )
		slash := second;
	endif

	// wwwww%xx[/yy]%[zzzzz]
	// 12345678     9 01234
	// 12345678 901 2 34567

	var s_plur := name[first+1, slash-(first+1)];
	var s_sing := "";
	if ( slash != first )
		s_sing := name[slash+1, second-(slash+1)];
	endif

	var s_first := name[1, first-1];
	var s_middle := s_plur;
	var s_last := "";
	if ( len(name) > second )
		s_last := name[second+1, len(name)-second];
	endif

	if ( add_S == 0 )
		s_middle := s_sing;
	endif
	
	if ( !s_first )
		s_first := "";
	endif
	if ( !s_middle )
		s_middle := "";
	endif
	if ( !s_last )
		s_last := "";
	endif
	return s_first + s_middle + s_last;

endfunction
