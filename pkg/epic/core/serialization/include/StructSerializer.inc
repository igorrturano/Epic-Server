include "system/exception/FormatException/InvalidFormatException";

include "Serializer";

class StructSerializer(Serializer)
	function StructSerializer(this)
	endfunction

	function Serialize(this, object)
		this.SetData(struct{});
		
		return Super::Serialize(this, object);
	endfunction

	function Deserialize(this, data, object)
		return Super::Deserialize(this, data, object);
	endfunction
endclass
