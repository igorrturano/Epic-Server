use uo;

include ":attributes:attributes";

function coverFace(item)
	case(item.graphic)
		0x1408: 0x1409: 0x1412:
		0x1419: 0x1451: 0x1F0B:
		0x140C: 0x140D: 0x1456:
		0x1F0C: 0x1549: 0x154A:
		0x154B: 0x154C:	0x278E:
		0x278F: 0x1388: 0xF0E6:
		0xF0E1: 0xF0DF: 0xF0CA:
		0xF0C6: 0xF0C7: 0xF0BA:
		0xF0B8: 0xF0B6:
			return 1;
		default:
			return 0;
	endcase
endfunction

function EnumeratePhysicalItems(mobile)
	var EquipList := array;

	var SkipLayers := {11, 16, 21, 25};
	var i;

	for (i:=0; i<=25; i:=i+1)
		if (!(i in SkipLayers))
			var layer := GetEquipmentByLayer(mobile, i);
			if (layer)
				EquipList.append(layer);
			endif
		endif
	endfor

	return EquipList;
endfunction

function UnequipAll(who)
	var equips := EnumeratePhysicalItems(who);

	foreach equip in equips
		MoveItemToContainer(equip, who.backpack);
	endforeach
endfunction
