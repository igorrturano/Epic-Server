include "system/exception/FormatException/InvalidFormatException";

include "StructSerializer";

class JsonSerializer(StructSerializer)
	function JsonSerializer(this)
	endfunction

	function Serialize(this, object, data, options...)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to serialize must be a class instance.");
		endif

		this._object := object;
		this._data := data;
		Super::Serialize(this, object, data);
		
		if (JSON_OPTION::COMPACT in options)
			return this.__ObjToJson(JSON_OPTION::COMPACT);
		else
			return this.__ObjToJson(JSON_OPTION::INDENT);
		endif

		return InvalidFormatException("Invalid serialization option provided.");
	endfunction

	function Deserialize(this, data, classObj)
		if (TypeOfInt(classObj) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to deserialize must be a class instance.");
		endif

		this._data := data;
		Super::Deserialize(this, data, classObj);

		return this.__JsonToObj(classObj);
	endfunction

	function __ObjToJson(this, option)
		return PackJSON(this._ObjToStruct(this._object), option);
	endfunction

	function __JsonToObj(this, classObj)
		var structData := UnpackJSON(this._data);
		return this._StructToObj(structData, classObj);
	endfunction
endclass

enum class JSON_OPTION
	COMPACT,
	INDENT
endenum
