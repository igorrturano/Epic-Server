use uo;

const NORMAL_SCALE := 1000;
const INT16_SIZE   := 2;
const MaxScaleValue := 60000;

// Appends mount's entityScale (4 bytes) to the rider's 0x78 packet.
// Must be called AFTER AppendScaleProp2Packet + AppendRacialTraits2Packet.
// Client reads it as MountScale and uses it when drawing the mount body.
function AppendMountScaleProp2Packet(object, byref packet)
    var lastOffset := packet.GetSize();

    if (!object.IsA(POLCLASS_MOBILE))
        packet.SetInt16(lastOffset,              NORMAL_SCALE);
        packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
        return;
    endif

    // Find mount item (layer 25)
    var mountItem := 0;
    foreach item in ListEquippedItems(object)
        if (item.layer == 25)
            mountItem := item;
            break;
        endif
    endforeach

    if (!mountItem)
        packet.SetInt16(lastOffset,              NORMAL_SCALE);
        packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
        return;
    endif

    var npcSerial := CInt(GetObjProperty(mountItem, "serial"));
    var npc       := npcSerial ? SystemFindObjectBySerial(npcSerial) : 0;

    if (!npc)
        packet.SetInt16(lastOffset,              NORMAL_SCALE);
        packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
        return;
    endif

    var scaleProp := GetObjProperty(npc, "entityScale");
    if (scaleProp && scaleProp.height && scaleProp.width)
        var h := CInt(scaleProp.height);
        var w := CInt(scaleProp.width);
        if (h > 0 && h <= MaxScaleValue && w > 0 && w <= MaxScaleValue)
            packet.SetInt16(lastOffset,              h);
            packet.SetInt16(lastOffset + INT16_SIZE, w);
            return;
        endif
    endif

    packet.SetInt16(lastOffset,              NORMAL_SCALE);
    packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
endfunction

function AppendScaleProp2Packet(object,byref packet)
	var scaleProp := GetObjProperty(object,"entityScale");
	var lastOffset := packet.GetSize();

	if(scaleProp && scaleProp.height && scaleProp.width)
		var heightScale := CInt(scaleProp.height);
		var widthScale := CInt(scaleProp.width);
		if(heightScale > 0 && heightScale <= MaxScaleValue && widthScale > 0 && widthScale <= MaxScaleValue)
			packet.SetInt16(lastOffset, cint(heightScale));
			packet.SetInt16(lastOffset + INT16_SIZE, cint(widthScale));
		else
			packet.SetInt16(lastOffset, NORMAL_SCALE);
			packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
		endif
	else
		packet.SetInt16(lastOffset, NORMAL_SCALE);
		packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
	endif
endfunction
