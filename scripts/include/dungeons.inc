use uo;

const BEETLE_MOUNTPIECE_GRAPHIC :=  0x3ebc;

function InDungeonMountCheck(mobile, coord) //retorna 1 se estiver fora de caverna ou montado num Beetle
	
	var mount_ref := GetEquipmentByLayer(mobile, 0x19); //LAYER_MOUNT
	if ( !mount_ref )
		return 1;
	elseif ( mount_ref.graphic == BEETLE_MOUNTPIECE_GRAPHIC )
		return 1;
	elseif ( !InDungeon(coord) )
		return 1;
	else
		return 0;
	endif
endfunction

function InDungeon(coord) // retorna 1 se tiver numa caverna
	//if ( coord.x > 3768 && coord.y > 1141 && coord.x < 4326 && coord.y < 2461 )
	//	return 0;
	//elseif ( coord.x > 3757 && coord.y > 0 && coord.x < 4982 && coord.y < 1078 )
	//	return 0;
	//elseif ( coord.x > 5120 && coord.y > 0 && coord.x < 6131 && coord.y < 3346 )
	//	return 0;
	//else
	//	return 1;
	//endif
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
