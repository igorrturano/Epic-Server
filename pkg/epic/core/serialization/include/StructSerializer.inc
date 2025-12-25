include "system/exception/FormatException/InvalidFormatException";

class StructSerializer()
	function StructSerializer(this)
	endfunction

	function Serialize(this, object, data)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to serialize must be a class instance.");
		endif

		this._object := object;
		this._data := data;
		
		return this.__ObjToStruct();
	endfunction

	function Deserialize(this, data, object)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to deserialize must be a class instance.");
		endif

		this._object := object;
		this._data := data;

		return this.__StructToObj();
	endfunction

	function __ObjToStruct(this)
		foreach attribute in (this._object)
			this._data[_attribute_iter] := attribute;
		endforeach

		return this._data;
	endfunction

	function __StructToObj(this)
		foreach attribute in (this._data)
			this._object[_attribute_iter] := attribute;
		endforeach

		return this._object;
	endfunction
endclass
