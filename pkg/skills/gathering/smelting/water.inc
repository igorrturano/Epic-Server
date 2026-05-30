use uo;
include ":crafting:include/util/fabricacaoConstants";

//var WATERSOURCES := array{0x5103};

function CheckForWater(who)

	var itensProximos := ListItemsNearLocation(who.x,who.y,who.z,3,who.realm);
	var staticosProximos := ListStaticsNearLocation(who.x,who.y,who.z,3,who.realm);
	itensProximos := itensProximos + staticosProximos;

	foreach item in itensProximos
		if (item.objtype in CATALOGO_FONTE_AGUA)
			return 1;
		endif
	endforeach
	return 0;
endfunction