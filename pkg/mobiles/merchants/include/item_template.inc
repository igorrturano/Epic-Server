use uo;
use os;
use polsys;

function CreateItemTemplate(item)
	if ( !item.IsA(POLCLASS_ITEM) )
		var temp := error;
		temp.+errortext := "Error: CreateItemTemplate() -> Object passed is not an item.";
		return temp;
	endif

	var template := GetItemDescriptor(CInt(item.objtype));
	if ( template == error )
		return template;
	endif

	template.+Amount;
	foreach property in template
		var item_property := item.get_member(Lower(CStr(_property_iter)));
		if ( item_property != error )
			template[_property_iter] := item_property;
		endif
		sleepms(2);
	endforeach

	//precisa por as props que nao ficam em cfgs manualmente
	if (item.xeast != error)
		template["xeast"] := item.xeast;
	endif
	if ( item.xwest != error)
		template["xwest"] := item.xwest;
	endif
	if ( item.ynorth != error )
		template["ynorth"] := item.ynorth;
	endif
	if ( item.ysouth != error )
		template["ysouth"] := item.ysouth;
	endif
	if ( item.gumpwidth != error)
		template["gumpwidth"] := item.gumpwidth;
	endif
	if ( item.gumpheight != error)
		template["gumpheight"] := item.gumpheight;
	endif

	if ( item.ar_mod != error )
		template["ar_mod"] := item.ar_mod;
	endif

	if ( item.onhitscript != error )
		template["onhitscript"] := item.onhitscript;
	endif

	if (item.quality != error)
		template["quality"] := item.quality;
	endif

	if (item.hp != error )
		template["hp"] := item.hp;
	endif

	if (item.maxhp_mod != error)
		template["maxhp_mod"] := item.maxhp_mod;
	endif

	if ( item.usescript != error)
		template["usescript"] := item.usescript;
	endif

	if ( item.equipscript != error)
		template["equipscript"] := item.equipscript;
	endif

	if ( item.unequipscript != error )
		template["unequipscript"] := item.unequipscript;
	endif

	if ( item.dmg_mod != error)
		template["dmg_mod"] := item.dmg_mod;
	endif

	if ( item.speed_mod != error)
		template["speed_mod"] := item.speed_mod;
	endif

	if (item.hitscript != error )
		template["hitscript"] := item.hitscript;
	endif

	foreach name in GetObjPropertyNames(item);
		template.CProps.insert(name, GetObjProperty(item, name));
		sleepms(2);
	endforeach

	if (item.isBook())
		var bookinfo := item.extractInfo();
		if (!item.isWritable())
			bookinfo.+readonly := 1;
		else
			bookinfo.+readonly := 0;
		endif

		template.Cprops.insert("bookinfo", bookinfo);
	endif

	if (item.isA(POLCLASS_WEAPON))
		template["damage_cold_mod"] := item.damage_cold_mod;
		template["damage_fire_mod"] := item.damage_fire_mod;
		template["damage_energy_mod"] := item.damage_energy_mod;
		template["damage_physical_mod"] := item.damage_physical_mod;
		template["damage_poison_mod"] := item.damage_poison_mod;
	endif

	if (item.isa(POLCLASS_ITEM))
		template["resist_cold_mod"] := item.resist_cold_mod;
		template["resist_fire_mod"] := item.resist_fire_mod;
		template["resist_energy_mod"] := item.resist_energy_mod;
		template["resist_physical_mod"] := item.resist_physical_mod;
		template["resist_poison_mod"] := item.resist_poison_mod;
	endif

	return template;
endfunction

function CreateIteminBackpackFromTemplate(player, selling)

	var purchase := CreateItemInBackPack(player, selling, selling.amount);
	if ( purchase == error )
		purchase := CreateItemAtLocation(player.x, player.y, player.z, selling, selling.amount);
		if ( purchase == error )
			SendSysMessage(player, "Error - Unable to create item! ->"+purchase.errortext);
		endif
	endif
	if (selling.amount == 1)
		SetName(purchase, selling.desc);
	endif
	foreach prop in selling
		//sendsysmessage(player, " " + _prop_iter + " "+ selling[_prop_iter]);
		purchase.set_member(_prop_iter, selling[_prop_iter]);
	endforeach

	if (purchase.isBook())
		var bookinfo := GetObjProperty(purchase, "bookinfo");
		purchase.setAuthor(bookinfo.Author );
		purchase.setTitle(bookinfo.Title);
		if (bookinfo.readonly == 1)
			purchase.ReadOnly(1);
		else
			purchase.ReadOnly(0);
		endif
		var book_data_file := DFOpenDataFile(":sysbook:books", DF_CREATE);
		var book_elem := DFFindElement(book_data_file, Hex(purchase.serial), DF_CREATE);
		book_elem.SetProp("Contents", bookinfo.contents);
	endif
	purchase.movable := 1;
	return purchase;
endfunction

function CreateItemAtLocationFromTemplate(x, y, z, realm, selling)

	var purchase := CreateItemAtLocation(x, y, z, selling, selling.amount, realm);
	if ( purchase == error )
		//SendSysMessage(player, "Error - Unable to create item! ->"+purchase.errortext);
	endif
	foreach prop in selling
		//sendsysmessage(player, " " + _prop_iter + " "+ selling[_prop_iter]);
		purchase.set_member(_prop_iter, selling[_prop_iter]);
	endforeach
	if (purchase.isBook())
		var bookinfo := GetObjProperty(purchase, "bookinfo");
		purchase.setAuthor(bookinfo.Author);
		purchase.setTitle(bookinfo.Title);
		if (bookinfo.readonly == 1)
			purchase.ReadOnly(1);
		else
			purchase.ReadOnly(0);
		endif
		var book_data_file := DFOpenDataFile(":sysbook:books", DF_CREATE);
		var book_elem := DFFindElement(book_data_file, Hex(purchase.serial), DF_CREATE);
		book_elem.SetProp("Contents", bookinfo.contents);
	endif
	if (selling.amount == 1)
		SetName(purchase, selling.desc);
	endif
	purchase.movable := 1;
	return purchase;
endfunction

function CreateItemInContainerFromTemplate(container, template)
    if (!container || !container.IsA(POLCLASS_CONTAINER))
        Print("DEBUG: Invalid container for item creation");
        return 0;
    endif
    
    if (!template || !template.objtype)
        Print("DEBUG: Invalid template for item creation");
        return 0;
    endif
    
    var item := CreateItemInContainer(container, template.objtype, template.amount ? template.amount : 1);
    if (!item)
        Print("DEBUG: Failed to create base item in container");
        return 0;
    endif
    
    // Apply all template properties to the item
    ApplyTemplatePropertiesToItem(item, template);
    
    return item;
endfunction

function ApplyTemplatePropertiesToItem(item, template)
    // Apply standard member properties
    foreach prop_name in (template.Keys())
        var prop_value := template[prop_name];
        
        case (prop_name)
            "objtype": continue; // Skip, already set
            "amount": continue;  // Skip, already set
            "color": item.color := CInt(prop_value);
            "name": SetName(item, CStr(prop_value));
            "desc": item.desc := CStr(prop_value);
            "graphic": item.graphic := CInt(prop_value);
            "CProps": 
                // Apply all custom properties
                if (TypeOf(prop_value) == "Dictionary")
                    foreach custom_prop in (prop_value.Keys())
                        SetObjProperty(item, custom_prop, prop_value[custom_prop]);
                        sleepms(2);
                    endforeach
                endif
            default:
                // Try to set as member property first
                var result := item.set_member(prop_name, prop_value);
                if (result == error)
                    // If not a member, set as obj property
                    SetObjProperty(item, prop_name, prop_value);
                endif
        endcase
        sleepms(2);
    endforeach
    
    // Handle book contents if needed
    if (item.isBook())
        var bookinfo := GetObjProperty(item, "bookinfo");
        if (bookinfo)
            item.setAuthor(bookinfo.Author);
            item.setTitle(bookinfo.Title);
            if (bookinfo.contents)
                var book_data_file := DFOpenDataFile(":sysbook:books", DF_CREATE);
                var book_elem := DFFindElement(book_data_file, Hex(item.serial), DF_CREATE);
                book_elem.SetProp("Contents", bookinfo.contents);
            endif
            
            if (bookinfo.readonly == 1)
                item.ReadOnly(1);
            else
                item.ReadOnly(0);
            endif
        endif
    endif
    
    return 1;
endfunction