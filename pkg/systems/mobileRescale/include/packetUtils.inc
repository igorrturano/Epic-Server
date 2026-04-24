use uo;

const NORMAL_SCALE := 1000;
const INT16_SIZE   := 2;
const MaxScaleValue := 60000;

function AppendScaleProp2Packet(object,byref packet)
	var scaleProp := GetObjProperty(object,"entityScale");
	var lastOffset := packet.GetSize();

	if(scaleProp && scaleProp.height && scaleProp.width)
		var heightScale := CInt(scaleProp.height);
		var widthScale := CInt(scaleProp.width);
		if(heightScale > 0 && heightScale <= MaxScaleValue && widthScale > 0 && widthScale <= MaxScaleValue)
			packet.SetInt16(lastOffset, cint(heightScale));
			packet.SetInt16(lastOffset + INT16_SIZE, cint(widthScale));
		endif
	else
		packet.SetInt16(lastOffset, NORMAL_SCALE);
		packet.SetInt16(lastOffset + INT16_SIZE, NORMAL_SCALE);
	endif
endfunction