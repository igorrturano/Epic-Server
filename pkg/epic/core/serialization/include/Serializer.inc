include "system/exception/FormatException/InvalidFormatException";

class Serializer()

	function Serializer(this)
	endfunction

	function Serialize(this, obj, serializeOption := SERIALIZE_OPTION::TO_STRUCT)
		if (TypeOfInt(obj) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to serialize must be a class instance.");
		endif
		
		if (serializeOption == SERIALIZE_OPTION::TO_STRUCT)
			return this._ObjToStruct(obj);
		elseif (serializeOption == SERIALIZE_OPTION::TO_DICTIONARY)
			return this._ObjToDictionary(obj);
		elseif (serializeOption == SERIALIZE_OPTION::TO_JSON)
			return this._ObjToJSON(obj);
		endif

		return InvalidFormatException("Invalid serialization option provided.");
	endfunction

	function Deserialize(this, data, classObj)
		if (TypeOfInt(classObj) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to deserialize must be a class instance.");
		endif

		var obj;

		if (TypeOfInt(data) == OT_STRUCT)
			return this._StructToObj(data, classObj);
		elseif (TypeOfInt(data) == OT_DICTIONARY)
			return this._DictionaryToObj(data, classObj);
		elseif (TypeOfInt(data) == OT_STRING)
			return this._JsonToObj(data, classObj);
		endif
		
		return InvalidFormatException("Invalid data format for deserialization.");
	endfunction

	function _ObjToStruct(obj)
		var data := struct{};

		foreach attribute in (obj)
			data.+_attribute_iter := obj.attribute;
		endforeach

		return data;
	endfunction

	function _ObjToDictionary(obj)
		var data := dictionary{};

		foreach attribute in (obj)
			data[_attribute_iter] := obj.attribute;
		endforeach

		return data;
	endfunction

	function _ObjToJson(obj, option := OPTION::INDENT)
		return PackJSON(_ObjToStruct(obj), option);
	endfunction

	function _StructToObj(data, classObj)
		var obj := classObj();

		foreach attribute in (data)
			obj.attribute := data.attribute;
		endforeach

		return obj;
	endfunction

	function _DictionaryToObj(data, classObj)
		var obj := classObj();

		foreach key in (data)
			obj.key := data[key];
		endforeach

		return obj;
	endfunction

	function _JsonToObj(data, classObj)
		var structData := UnpackJSON(data);
		return _StructToObj(structData, classObj);
	endfunction
endclass

enum class OPTION
	COMPACT,
	INDENT
endenum

enum class SERIALIZE_OPTION
	TO_STRUCT,
	TO_DICTIONARY,
	TO_JSON
endenum