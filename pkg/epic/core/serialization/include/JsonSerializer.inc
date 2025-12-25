include "system/exception/FormatException/InvalidFormatException";

include "StructSerializer";

class JsonSerializer(StructSerializer)
	function JsonSerializer(this)
	endfunction

	function Serialize(this, object, options...)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to serialize must be a class instance.");
		endif

		var serializedStruct := Super::Serialize(this, object);

		if (JSON_OPTION::COMPACT in options)
			return PackJSON(serializedStruct, JSON_OPTION::COMPACT);
		else
			return PackJSON(serializedStruct, JSON_OPTION::INDENT);
		endif

		return InvalidFormatException("Invalid serialization option provided.");
	endfunction

	function Deserialize(this, data, object)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to deserialize must be a class instance.");
		endif

		return Super::Deserialize(this, UnpackJSON(data), object);
	endfunction
endclass

enum class JSON_OPTION
	COMPACT,
	INDENT
endenum
