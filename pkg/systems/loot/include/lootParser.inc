use uo;
use util;
use cfgfile;
use os;

include ":loot:magicLoot";

function Loot_Generate(container, loot_index)
	if ( !container.IsA(POLCLASS_CONTAINER) )
		return error{"errortext" := "Object passed was not a container."};
	endif

	var index_cfg := ReadConfigFile(":loot:config/loot-index");
	if ( index_cfg.errortext )
		return error{"errortext" := "Error - Could not open loot-index.cfg -> "+index_cfg.errortext};
	endif

	var index_elem := index_cfg[loot_index];
	if ( !index_elem )
		return error{"errortext" := "Error - loot-index.cfg elem ["+index_elem+"] does not exist."};
    //print( "Error - loot-index.cfg elem ["+index_elem+"] does not exist.");
	endif

	foreach line in GetConfigStringArray(index_elem, "Item")
		Loot_CreateItem(container, line);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Stack")
		Loot_CreateStack(container, line);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Random")
		Loot_CreateRandom(container, line);
		sleepms(2);
	endforeach
	
	foreach line in GetConfigStringArray(index_elem, "Group")
		if ( line != loot_index )
			//To do:
			// Check to make sure were not going back into the same group
			// to avoid a potential endless loop!
			Loot_Generate(container, line);
		endif
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Magic");
		Loot_CreateMagic(container, line);
		sleepms(2);
	endforeach

	return 1;
endfunction



function Loot_CreateItem(container, line)
	var parsed := Loot_ParseString(line);

	if ( Loot_ChanceCheck(parsed.chance) )
		var i;
		for ( i:=1; i<= parsed.amount; i:=i+1 )
			var item := CreateItemInContainer(container, parsed.name);
			if ( item.errortext )
          //print("Error - Could not create ["+parsed.name+"] ->"+item.errortext + " " + parsed); 
				return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
			endif
			sleepms(2);
		endfor
	endif
	
	return 1;
endfunction

function Loot_CreateStack(container, line)
	var parsed := Loot_ParseString(line);
	if ( Loot_ChanceCheck(parsed.chance) )
		var item := CreateItemInContainer(container, parsed.name, parsed.amount);
		if ( item.errortext )
			return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
		endif
	endif
	
	return 1;
endfunction

function Loot_CreateRandom(container, line)
	var parsed := Loot_ParseString(line);
	var groups_cfg := ReadConfigFile(":loot:config/loot-groups");
	if ( groups_cfg.errortext )
		return error{"errortext" := "Error - Could not open loot-groups.cfg -> "+groups_cfg.errortext};
	endif
	
	var groups_elem := groups_cfg[parsed.name];
	if ( !groups_elem )
		return error{"errortext" := "Error - loot-groups.cfg elem ["+parsed.name+"] does not exist."};
	endif
	
	if ( Loot_ChanceCheck(parsed.chance) )
		var items := GetConfigStringArray(groups_elem, "Item");
		var num_items := items.size();
		var i;
		for ( i:=1; i<=parsed.amount; i:=i+1 )
			Loot_CreateItem(container, items[RandomInt(num_items)+1]);
			sleepms(2);
		endfor
	endif
	
	return 1;
endfunction

function Loot_ChanceCheck(chance)
var rolado := Cint(RandomFloat(1000)); 
var rolado2 := rolado+1;
	if ( rolado2 <= CInt(CDbl(chance)*10.0) )
		return 1;
	else
		return 0;
	endif
endfunction

function Loot_ParseString(line)
	line := SplitWords(line);
	
	var name := CStr(line[1]);
	
	var amount := CStr(line[2]);
	amount := RandomDiceRoll(amount);
	
	var chance := CDbl(line[3]);
	if ( !chance )
		chance := 100.0;
	endif

	var level := CInt(line[4]);
	if (!level)
		level := 1;
	endif

	var result := struct;
	result.+name := name;
	result.+amount := amount;
	result.+chance := chance;
	result.+level := level;

	return result;
endfunction

