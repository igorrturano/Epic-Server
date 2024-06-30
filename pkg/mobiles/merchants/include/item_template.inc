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
endfunction
