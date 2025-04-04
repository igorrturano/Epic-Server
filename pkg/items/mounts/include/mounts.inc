// {$Id}

/*===============================================================
* Current Version
* MOUNTS.INC - v1.0
* Updated 10/3/2005 12:22PM
*
* -- Revision v1.0 --
* Austin:
*  Created include file
===============================================================*/

use uo;
use os;
use cfgfile;

//Arbitrary objtype in the core.
const MOUNT_OBJTYPE := 0x1F021;

include "include/client";


/*
 * MP_GetMountsCfgFile()
 *
 * Purpose
 * Retrieves the mounts.cfg file
 *
 * Parameters
 *
 * Return value
 * Returns a config file
 *
 */
function MP_GetMountsCfgFile()
	var cfg := ReadConfigFile(":mounts:config/mounts");

	if ( cfg.errortext )
		SysLog("Error::MP_GetMountsCfgFile() - Unable to open [:mounts:config/mounts.cfg] ->"+cfg.errortext);
	endif

	return cfg;
endfunction

/*
 * MP_GetMountsCfgElem(elem_name, cfg_file)
 *
 * Purpose
 * Retrieves an elem from a config file.
 *
 * Parameters
 * elem_name:	A string matching the elem name to be retrieved.
 * cfg_file:	Optional parameter - reference to a config already read in by AP_GetMountsCfgFile()
 *
 * Return value
 * A config file elem.
 *
 */
function MP_GetMountsCfgElem(elem_name, byref cfg_file:=0)
	if ( !cfg_file )
		cfg_file := MP_GetMountsCfgFile();
	endif

	var elem := cfg_file[elem_name];

	if ( elem.errortext )
		SysLog("Error::MP_GetMountsCfgElem() - Unable to find elem ["+elem_name+"] ->"+elem.errortext);
	endif

	return elem;
endfunction

/*
 * MP_GetMountGraphic(mobile)
 *
 * Purpose
 * Finds what graphic a mount piece must be set to in order to match an npc.
 *
 * Parameters
 * mobile:	NPC to get the mount tile graphic for.
 *		Can also be the graphic number of an npc.
 *
 * Return value
 * Returns a graphic number.
 * Returns error if no match can be found.
 *
 */
function MP_GetMountGraphic(mobile)
	var mobile_graphic;
	if ( mobile.IsA(POLCLASS_NPC) )
		mobile_graphic := Hex(mobile.graphic);
	else
		mobile_graphic := Hex(CInt(mobile));
	endif

	var elem := MP_GetMountsCfgElem("MobileToMount");
	var tile_graphic := GetConfigInt(elem, mobile_graphic);

	if ( tile_graphic )
		return tile_graphic;
	else
		return error{"errortext":="Error::MP_GetMountGraphic() - No mount tile graphic matches mobile graphic '"+mobile_graphic+"'"};
	endif
endfunction

/*
 * MP_GetMountTemplate(mount)
 *
 * Purpose
 * Finds what npc template must be used to match a mount tile's graphic.
 *
 * Parameters
 * mount:	An item (mount piece) to find an npc template from.
 *		Can also be the graphic number of a mount piece.
 *
 * Return value
 * Returns a string.
 * Returns error if no match can be found.
 *
 */
function MP_GetMountTemplate(mount)
	var mount_graphic, template;
	if ( mount.IsA(POLCLASS_ITEM) )
		template := GetObjProperty(mount, "NPCTemplate");
		if ( template )
			return template;
		else
			mount_graphic := Hex(mount.graphic);
		endif
	else
		mount_graphic := Hex(CInt(mount));
	endif

	var elem := MP_GetMountsCfgElem("MountToMobile");
	template := GetConfigString(elem, mount_graphic);

	if ( template )
		return template;
	else
		return error{"errortext":="Error::MP_GetMountTemplate() - No NPC template match for mount piece graphic '"+mount_graphic+"'"};
	endif
endfunction

/*
 * MP_Mount(mobile, npc)
 *
 * Purpose
 * Removes the NPC, sets the mount piece up, and equips it on the mobile.
 *
 * Parameters
 * mobile:	Mobile that will be mounted.
 * npc:		NPC to convert into a mount piece.
 *
 * Return value
 * Returns the mount piece.
 *
 */
function MP_Mount(mobile, npc)

	var mount_graphic := MP_GetMountGraphic(npc);
	var chardata := GetObjProperty(mobile,  "chardata");
	var mount_piece := GetEquipmentByLayer(mobile, LAYER_MOUNT);
	var npctemplate := NPC_ParseTemplateName(npc.npctemplate);
	var cfg := ReadConfigFile(":*:npcdesc");

	if (cfg[npctemplate.template].Category != "Mount")
		SendSysMessage(mobile, "Nao pode montar este animal.");
		return 0;
	endif


	if ( Cint(GetObjProperty(mobile, "#desmontar")) == 1)
		SendSysMessage(mobile, "Voce nao pode montar.");
		return 0;
	endif

	//check de polymorph druid
	if ( Cint(GetObjProperty(mobile, "metamorfosedruida")) == 1)
		SendSysMessage(mobile, "Voce nao pode montar nessa forma.");
		return 0;
	endif



	//check do Urso
	if (npc.graphic == 0xD5 && chardata.povo != "Numajene")
		SendSysMessage(mobile, "Voce nao pode montar essa criatura.");
		return 0;
	endif

	//check do Unicornio
	if (npc.graphic == 0x7A && chardata.povo != "Numajene")
		SendSysMessage(mobile, "Voce nao pode montar essa criatura.");
		return 0;
	endif

	//Lock dos tigres
	if (npc.graphic == 0x4e6 || npc.graphic == 0x4e7)
		return error{"errortext":="Voce nao pode montar essa criatura"};
	endif

	npc.hidden := 1;


	if ( mount_piece )
		return error{"errortext":="Mobile is already mounted."};
	endif

	if (GetObjProperty(npc, "owner") != mobile.serial)
		return error{"errortext":="Mobile is already mounted."};;
	endif

	Set_Critical(1);

	if ( !mount_graphic )
		return mount_graphic;
	endif

	mount_piece := CreateItemAtLocation(mobile.x, mobile.y, mobile.z, MOUNT_OBJTYPE, 1);
	mount_piece.graphic := mount_graphic;
	mount_piece.color := npc.color;
	//sendsysmessage(mobile, "cor " + mount_piece.color + " " + npc.color);
	SetName(mount_piece, npc.name);

	var result := EquipItem(mobile, mount_piece);
	if ( !result )
		// Unable to equip the mount piece.
		DestroyItem(mount_piece);
		return result;
	else
		// Setting equipped mounts prevents un-equip exploits.
		mount_piece.movable := 0;
	endif

	SetObjProperty(mount_piece, "serial", npc.serial);
	SetObjProperty(mount_piece, "riderserial", mobile.serial);

	//ExchangeStamina(mobile, npc);

	MoveObjectToLocation(npc, 6066, 60, 22, npc.realm, MOVEOBJECT_FORCELOCATION);
	//MoveObjectToLocation(me, owner.x, owner.y, owner.z, me.realm, MOVEOBJECT_FORCELOCATION);

	return mount_piece;
endfunction


/*
 * MP_Unmount(mobile)
 *
 * Purpose
 * Removes the mount piece from the mobile and restores it to an npc.
 *
 * Parameters
 * mobile:	Mobile to unmount
 *
 * Return value
 * Returns the restored npc.
 *
 */
function MP_Unmount(mobile)
	var mount_piece := GetEquipmentByLayer(mobile, LAYER_MOUNT);

	if ( !mount_piece )
		SendSysMessage(mobile, "Nao tem a mount piece");
		return error{"errortext":="Mobile does not have a mount item."};
	elseif ( Lower(mount_piece.desc) == "mount piece" )
		// Mount piece exists, but contains no npc information.
		SendSysMEssage(mobile, "mount piece desc errada");
		DestroyItem(mount_piece);
		return 0;
	else
		return MP_RestoreMountToAnimal(mobile, mount_piece);
	endif
endfunction

/*
 * MP_DeathUnmount(corpse)
 *
 * Purpose
 * Finds a mount piece in the corpse and restores the npc.
 *
 * Parameters
 * corpse:	Corpse to search for mount pieces in.
 *
 * Return value
 * Returns the restored npc.
 *
 */
function MP_DeathUnmount(corpse)
	if ( !corpse.IsA(POLCLASS_CORPSE) )
		return error{"errortext":="Object passed is not a corpse."};
	endif

	foreach item in ( EnumerateItemsInContainer(corpse) )
		if ( item.objtype == MOUNT_OBJTYPE )
			return MP_RestoreMountToAnimal(corpse, item);
			break;
		endif
	endforeach

	return error{"errortext":="No mount pieces were found on the corpse."};
endfunction

/*
 * MP_RestoreMountToAnimal(mobile, mount)
 *
 * Purpose
 * Removes the mount piece and recreates an npc.
 *
 * Parameters
 * mobile:	Mobile that was riding the mount.
 * mount:	Mount piece that will be used to create and restore the npc.
 *
 * Return value
 * Returns the npc that was created / restored.
 *
 */
function MP_RestoreMountToAnimal(mobile, mount)
	var orig_serial := CInt(GetObjProperty(mount, "serial"));
	var animal := SystemFindObjectBySerial(orig_serial);

//	sendsysmessage(mobile, " " + orig_serial + " " + animal);

	if ( animal )
		// Supports systems where the npc is moved to a storage area rather than killed.
		var ret := MoveObjectToLocation(animal, mobile.x, mobile.y, mobile.z, mobile.realm, MOVEOBJECT_FORCELOCATION);
		animal.hidden := 0;
		SendSysMessage(mobile, " " + ret);
	else
		var template := MP_GetMountTemplate(mount);
		if ( !template )
			DestroyItem(mount);
			return template;
		endif

		animal := CreateNPCFromTemplate(template, mobile.x, mobile.y, mobile.z);
		animal.facing := mobile.facing;
		animal.color := mount.color;
		animal.name := mount.desc;
	endif
	DestroyItem(mount);

	return animal;
endfunction

/*
 * ExchangeStamina(who, me)
 *
 * Purpose
 * Troca a Stamina entre a montaria e o jogador
 *
 * Parameters
 * who:	Jogador montando o animal
 * me:	Animal Montado
 *
 * Return value
 * Returns the npc that was created / restored.
 *
 */
 function ExchangeStamina(who, me)
	var my_stamina := AP_GetVital(me, "Stamina");
	var his_stamina := AP_GetVital(who, "Stamina");

	var his_vitality := CInt(GetObjProperty(who, "Vitality"));

	SetObjProperty(who, "OldVitality", his_vitality);
	SetObjProperty(me, "Vitality", his_vitality);
	SetObjProperty(who, "Vitality", CInt(AP_GetVitalMaximumValue(me, "Stamina")));

	RecalcVitals(who);
	RecalcVitals(me);

	AP_SetVital(who, "Stamina", my_stamina);
	AP_SetVital(me, "Stamina", his_stamina);
endfunction


/*
 * IsMountable(mobile_graphic)
 *
 * Purpose
 * Checa se o grafico passado no parametro eh de um animal montavel
 *
 * Parameters
 * mobile_graphic:	Graphic do bicho
 *
 * Return value
 * Returns the npc that was created / restored.
 *
 */
function IsMountable(mobile_graphic)
	var elem := MP_GetMountsCfgElem("MobileToMount");
	if( GetConfigInt(elem, Hex(mobile_graphic)) )
		return 1;
	else
		return 0;
	endif
endfunction
