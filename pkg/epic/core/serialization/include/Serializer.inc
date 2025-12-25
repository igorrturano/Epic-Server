include "system/exception/FormatException/InvalidFormatException";

class Serializer()
	var _object;
	var _data;

	function Serializer(this)
	endfunction

	function GetObject(this)
		return this._object;
	endfunction

	function SetObject(this, object)
		this._object := object;
	endfunction

	function GetData(this)
		return this._data;
	endfunction

	function SetData(this, data)
		this._data := data;
	endfunction

	function Serialize(this, object)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to serialize must be a class instance.");
		endif

		this.SetObject(object);

		return this.__ObjToData();
	endfunction

	function Deserialize(this, data, object)
		if (TypeOfInt(object) != OT_CLASSINSTANCEREF)
			return InvalidFormatException("Object to deserialize must be a class instance.");
		endif

		this.SetObject(object);
		this.SetData(data);

		return this.__DataToObj();
	endfunction

	function __ObjToData(this)
		foreach attribute in (this._object)
			this._data[_attribute_iter] := attribute;
		endforeach

		return this._data;
	endfunction

	function __DataToObj(this)
		foreach attribute in (this._data)
			this._object[_attribute_iter] := attribute;
		endforeach

		return this._object;
	endfunction
endclass
