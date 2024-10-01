use uo;

var WATERSOURCES := array{0x5103};

function CheckForWater(who)
	foreach item in ListItemsNearLocation(who.x, who.y, who.z, 3, who.realm)
		if (item.objtype in WATERSOURCES)
			return 1;
		endif
	endforeach
	return 0;
endfunction