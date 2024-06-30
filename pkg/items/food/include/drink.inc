

function reduce_container_fullness (byref character, byref container)
	//sendsysmessage(character, " " + container);
	var fullness := GetObjProperty (container, "fullness");

	var cfgfile := ReadConfigFile( "itemdesc" );
	var elem := FindConfigElem( cfgfile, container.objtype );
	var return_type := GetConfigInt (elem, "return_type");

	if (! fullness)
		SetObjProperty (container, "fullness", 3);
		var tempstring := container.desc;
		container.name := "um quase cheio " + tempstring;

	elseif (fullness == 3)
		SetObjProperty (container, "fullness", 2);
		var tempstring := container.desc;
		if (tempstring ["a mostly full"])
			tempstring ["a mostly full"] := "metade da";
		elseif (tempstring ["mostly full"])
			tempstring ["mostly full"] := "metade da";
		endif
		container.name := tempstring;
	elseif (fullness == 2)
		SetObjProperty (container, "fullness", 1);
		var tempstring := container.desc;
		if (tempstring ["half a"])
			tempstring ["half a"] := "quase vazio";
		endif
		container.name := tempstring;

	else
		if (return_type)
			var empty_container := CreateItemInBackpack (character, return_type );

			if (GetObjProperty (container, "EmptyName") )
				empty_container.name := GetObjProperty (container, "EmptyName");
			endif
		endif
		destroyitem (container);
	endif
endfunction  
