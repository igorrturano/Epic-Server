use uo;
include ":message:include/model/Message";

// Dungeon-allowed mount piece graphics (what players equip when mounted)
const BEETLE_MOUNTPIECE_GRAPHIC := 0x3ebc;  // Giant Beetle2
const BEETLE_MOUNTPIECE_GRAPHIC2 := 0x3e95; // Giant Beetle
const ETHEREAL_BEETLE_GRAPHIC := 0x3E97;    // Ethereal Giant Beetle
const TARANTULA_MOUNTPIECE_GRAPHIC := 0x3ECA; // Tarantula
const RIDGEBACK_MOUNTPIECE_GRAPHIC := 0x3EB8; // Ridgeback
const SAVAGE_RIDGEBACK_GRAPHIC := 0x3EBA;     // Savage Ridgeback
const ETHEREAL_RIDGEBACK_GRAPHIC := 0x3E9A;   // Ethereal Ridgeback

/*
 * IsDungeonApprovedMount(mount_graphic)
 *
 * Purpose
 * Checks if a mount piece graphic is allowed in dungeons
 *
 * Parameters
 * mount_graphic: The mount piece graphic to check
 *
 * Return value
 * Returns 1 if mount is dungeon-approved, 0 otherwise
 */
function IsDungeonApprovedMount(mount_graphic)
	case (mount_graphic)
		BEETLE_MOUNTPIECE_GRAPHIC:    // Giant Beetle2
		BEETLE_MOUNTPIECE_GRAPHIC2:   // Giant Beetle
		ETHEREAL_BEETLE_GRAPHIC:      // Ethereal Giant Beetle
		TARANTULA_MOUNTPIECE_GRAPHIC: // Tarantula
		RIDGEBACK_MOUNTPIECE_GRAPHIC: // Ridgeback
		SAVAGE_RIDGEBACK_GRAPHIC:     // Savage Ridgeback
		ETHEREAL_RIDGEBACK_GRAPHIC:   // Ethereal Ridgeback
			return 1; // Dungeon-approved mount
		default:
			return 0; // Not allowed in dungeon
	endcase
endfunction

/*
 * InDungeonMountCheck(mobile, coord)
 *
 * Purpose
 * Checks if a mobile can use their mount based on dungeon status
 *
 * Parameters
 * mobile: The mobile to check
 * coord:  The coordinates to check
 *
 * Return value
 * Returns 1 if mount is allowed (not in dungeon OR riding dungeon-approved mount)
 * Returns 0 if mount is not allowed in dungeon
 */
function InDungeonMountCheck(mobile, coord)
	var mount_ref := GetEquipmentByLayer(mobile, 0x19); //LAYER_MOUNT

	// No mount equipped - allowed
	if ( !mount_ref )
		return 1;
	endif

	// Not in dungeon - allowed
	if ( !InDungeon(coord) )
		return 1;
	endif

	// In dungeon - check if mount is dungeon-approved
	if (IsDungeonApprovedMount(mount_ref.graphic))
		return 1;
	else
		Message("Você não pode usar essa montaria em masmorras").SetColor(SSM_FAIL).SendSys(mobile);
		return 0;
	endif
endfunction

function InDungeon(coord) // retorna 1 se tiver numa caverna
	if ( coord.realm == "britannia")
		return 1;
    elseif ( coord.x >= 3399 && coord.y >= 0)
		return 1;
	else
		return 0;
	endif
endfunction

function InDarkness(who)
    var diaounoite := GetGlobalProperty("dia");

    if ((diaounoite == "dia" && !InDungeon(who)))
        return 0;
    endif

    return 1;
endfunction
