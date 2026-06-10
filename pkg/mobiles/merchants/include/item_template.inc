use uo;
use os;
use polsys;
use datafile;

include ":datafile:datafile";

function CreateItemTemplate(item, capture_contents := 0)
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

	if (item.name != error && item.name != "")
		template["name"] := item.name;
	endif

	if (item.desc != error && item.desc != "")
		template["desc"] := item.desc;
	endif

	if (item.color != error)
		template["color"] := item.color;
	endif

	if (item.graphic != error)
		template["graphic"] := item.graphic;
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

	/* não mais necessário no novo combat
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
	*/
	template.+isVendorTemplate := true;

	// Optionally snapshot a container's contents recursively (opt-in so the
	// other callers of CreateItemTemplate are unaffected).
	if (capture_contents && item.IsA(POLCLASS_CONTAINER))
		template.+__contents := CaptureContainerContents(item, 1);
	endif

	return template;
endfunction

// Recursively snapshot a container's contents as an array of item templates.
// Each child carries its own members; nested containers recurse with a depth
// guard so pathological nesting can't hang the capture.
function CaptureContainerContents(container, depth)
	var contents := array{};
	if (depth > 10)
		Print("CaptureContainerContents: nesting exceeds depth 10, truncating");
		return contents;
	endif
	foreach child in (EnumerateItemsInContainer(container))
		var child_template := CreateItemTemplate(child, 0);
		if (child_template != error)
			if (child.IsA(POLCLASS_CONTAINER))
				child_template.+__contents := CaptureContainerContents(child, depth + 1);
			endif
			contents.append(child_template);
		endif
		sleepms(2);
	endforeach
	return contents;
endfunction

// Recreate captured contents inside a freshly created container. Recurses via
// CreateItemInContainerFromTemplate, which restores nested contents in turn.
function RestoreContainerContents(container, contents)
	if (!contents || !container)
		return 0;
	endif
	foreach child_template in (contents)
		CreateItemInContainerFromTemplate(container, child_template);
		sleepms(2);
	endforeach
	return 1;
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
		// "CProps", "__contents" e flags internas são tratados separadamente — pular aqui.
		if (_prop_iter == "CProps" || _prop_iter == "__contents" || _prop_iter == "isVendorTemplate")
			continue;
		endif
		purchase.set_member(_prop_iter, selling[_prop_iter]);
	endforeach

	// REAPLICA CProps do template ao item final, sobrescrevendo o que o
	// CreateScript do itemdesc possa ter aleatorizado (ex: AssignCraftRecipe
	// gera recipe RANDOM toda vez que um recipe scroll é criado por objtype).
	// Sem esta reaplicação, o comprador recebe um scroll com recipe diferente
	// do que estava à venda — bug histórico do vendor.
	if (purchase && purchase != error && TypeOf(selling.CProps) == "Dictionary")
		foreach custom_prop in (selling.CProps.Keys())
			SetObjProperty(purchase, custom_prop, selling.CProps[custom_prop]);
			sleepms(2);
		endforeach
	endif

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

	// Restaura conteúdo de container preservado por CreateItemTemplate(item, 1).
	// Sem isso, vender uma bag cheia entregava bag vazia ao comprador.
	if (selling.__contents && purchase.IsA(POLCLASS_CONTAINER))
		RestoreContainerContents(purchase, selling.__contents);
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
		if (_prop_iter == "__contents")
			continue; // restored separately as child items
		endif
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

	// Recreate captured container contents (recursive)
	if (selling.__contents && purchase.IsA(POLCLASS_CONTAINER))
		RestoreContainerContents(purchase, selling.__contents);
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

    // Recreate captured container contents (recursive)
    if (template.__contents && item.IsA(POLCLASS_CONTAINER))
        RestoreContainerContents(item, template.__contents);
    endif

    return item;
endfunction

function ApplyTemplatePropertiesToItem(item, template)
    // Apply standard member properties
    foreach prop_name in (template.Keys())
        var prop_value := template[prop_name];
        
        case (prop_name)
            "objtype": continue; // Skip, already set
            "amount": continue;  // Skip, already set
            "__contents": continue; // Skip, restored separately as child items
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