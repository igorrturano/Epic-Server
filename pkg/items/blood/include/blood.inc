/*
 * $Id
 *
 */

use uo;
use cfgfile;

include ":brainAI:npcUtil";

/*
 * MakeBleed(object, blood_color)
 *
 * Purpose
 * Creates a blood tile where the object is and sets the blood to the appropriate color.
 *
 * Parameters
 * object:	The object to create the blood for.
 * blood_color:	Optional parameter. Will force the blood to be this color.
 *
 * Return value
 * Returns an item reference to the tile created.
 *
 */
function MakeBleed(object, blood_color:=0)
	// These checks are for players or individual NPCs.
	var blood_type := "blood";
	if ( object.isBloodless())
		return 0;
	elseif ( !blood_color )
		blood_color := CInt(GetObjProperty(object, "BloodColor"));
	endif
	var btype := GetObjProperty(object, "BloodType");
	if (btype != error)
		blood_type := btype;
	endif
	
	if ( object.npctemplate )
		// Living NPC
		var npc_info := NPC_GetNPCConfig(object.npctemplate);
		if ( npc_info.NoBlood )
			// NPC is set to not bleed.
			return 0;
		else
			if ( !blood_color )
				// If no blood color is set - retrieve NPC's template blood color.
				blood_color := CInt(npc_info.BloodColor);
			endif
			if ( npc_info.BloodType )
				blood_type := npc_info.BloodType;
			endif
		endif
	elseif ( object.IsA(POLCLASS_CORPSE) )
		// If it is a player corpse, then the CProp checks at the begining
		// of this function have taken care of what is needed. 
		// However, if it is an NPC corpse - the color needs to be retrieved from the npcdesc.cfg
		var template := object.NPCTemplate();
		if ( template )
			var npc_info := NPC_GetNPCConfig(template);
			
			if ( npc_info.NoBlood )
				//Does this npc bleed?
				return 0;
			else
				if ( !blood_color )
					//Does this npc have a special blood color?
					blood_color := CInt(npc_info.BloodColor);
				endif
				if (npc_info.BloodType)
					blood_type := npc_info.BloodType;
				endif
			endif
		endif
	endif
	var blood := CreateItemAtLocation(object.x, object.y, object.z, blood_type, 1, object.realm);
	if (blood_type == "blood");
		blood.color := blood_color;
	endif
	return blood;
endfunction

function isBlood(item)


	var cfg := ReadConfigFile(":blood:config/settings");
	var elem := FindConfigElem(cfg, "Graphics");

	var bloodGraphics := GetConfigStringArray(elem, "Graphic");

	foreach graphic in bloodGraphics
		if (item.graphic == graphic)
			return 1;
		endif
	endforeach

	return 0;

endfunction
