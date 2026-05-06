use uo;
include ":charactercreation:include/RPTraits";

const MAX_TRAIT_VALUE := 200;
const INT8_SIZE       := 1;

function AppendRacialTraits2Packet(object,byref packet)
	var lastOffset := packet.GetSize();
	var chardata := GetObjProperty(object, "chardata");
	
	if (!chardata || !chardata.caracteristicas_rp)
		packet.SetInt8(lastOffset, ALTURA_RACIAL::MEDIO);
		packet.SetInt8(lastOffset + INT8_SIZE, PESO_RACIAL::MEDIO);
		return 0;
	endif

	var chosenHeight := cint(chardata.caracteristicas_rp.Altura);
	var chosenWeight := cint(chardata.caracteristicas_rp.Peso);
	chosenHeight :=  chosenHeight < 0 || chosenHeight > MAX_TRAIT_VALUE || TypeOfInt(chosenHeight) == OT_ERROR? ALTURA_RACIAL::MEDIO : chosenHeight;
	chosenWeight :=  chosenWeight < 0 || chosenWeight > MAX_TRAIT_VALUE || TypeOfInt(chosenWeight) == OT_ERROR? PESO_RACIAL::MEDIO : chosenWeight;

	packet.SetInt8(lastOffset,chosenHeight);
	packet.SetInt8(lastOffset + INT8_SIZE,chosenWeight);
endfunction