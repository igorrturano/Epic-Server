class Serializer()
	var _type;

	function Serializer(this, type)
		this._type := type;
	endfunction

	function Serialize(this, obj)

	endfunction

	function Deserialize(this, data, object)
	endfunction
endclass

enum class SERIALIZERTYPE
	FROM_JSON,
	FROM_STRUCT,
	FROM_DICT
endenum