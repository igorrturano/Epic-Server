include "system/exception/FormatException/InvalidFormatException";

class Serializer()

	function Serializer(this)
	endfunction

	uninit function Serialize(this, object);
	uninit function Deserialize(this, data, classObj);
endclass
