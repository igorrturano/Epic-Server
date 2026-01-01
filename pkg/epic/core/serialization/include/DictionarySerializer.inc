include "Serializer";

class DictionarySerializer(Serializer)
	function DictionarySerializer(this)
	endfunction

	function Serialize(this, object)
		this.SetData(dictionary{});
		
		return Super::Serialize(this, object);
	endfunction

	function Deserialize(this, data, object)
		return Super::Deserialize(this, data, object);
	endfunction
endclass