class JSON()
	function Serialize(obj, options...)
		if (TypeOfInt(obj) == OT_CLASSINSTANCEREF)
			var data := struct{};

			foreach attribute in (obj)
				data.+_attribute_iter := obj.attribute;
			endforeach

			obj := data;
		endif
		
		if (OPTION::INDENT in options)
			PackJSON(obj, true);
			// Indented serialization
		endif
		// Implementation of JSON serialization
	endfunction

	function Deserialize(obj)
		return UnpackJSON(obj);
	endfunction
endclass

enum class OPTION
	COMPACT,
	INDENT
endenum