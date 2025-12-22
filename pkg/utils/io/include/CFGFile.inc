use cfgfile;

include "model/IOFile";

class CFGFile(IOFile)
	var _elementName := "";
	var _element;
	
	function CFGFile(this, packageName, fileName, createFile := true)
		Super(packageName, fileName);
		this._LoadFile(createFile);
	endfunction

	function GetElementName(this)
		return this._elementName;
	endfunction

	function SetElementName(this, elem_name)
		this._elementName := elem_name;
		this._element := FindConfigElem(this._file, this._elementName);

		return this;
	endfunction

	function GetElement(this)
		return this._element;
	endfunction

	function GetElementProps(this)
		var result;

		if (!this._elementName)
			return InvalidPropException("Element name not set");
		endif
		
		if (!this._element)
			return InvalidPropException($"Element {this._elementName} not found in {this.GetFullPath()}");
		endif

		result := ListConfigElemProps(this._element);
		if (!result)
			return error{ errortext := $"No properties found in element {this._elementName} of {this.GetFullPath()}" };
		endif

		return result;
	endfunction

	function GetKeyValue(this, keyName, type := OT_STRING, isDictionary := false)
		var keyValue;
		
		if (isDictionary)
			keyValue := GetConfigStringDictionary(this._element, keyName);

			if (type == OT_LONG)
				foreach value in (keyValue.values())
					value := CInt(value);
				endforeach
			elseif (type == OT_DOUBLE)
				foreach value in (keyValue.values())
					value := CDbl(value);
				endforeach
			endif
		else
			keyValue := GetConfigStringArray(this._element, this._keyName);

			if (type == OT_LONG)
				foreach value in (keyValue)
					value := CInt(value);
				endforeach
			elseif (type == OT_DOUBLE)
				foreach value in (keyValue)
					value := CDbl(value);
				endforeach
			endif

			if (keyValue.size() == 1)
				keyValue := keyValue[1];
			endif
		endif

		return keyValue;
	endfunction

	function Unload(this)
		UnloadConfigFile(this._file);
	endfunction

	function ToString(this)
		return $"CFGFile: {Super::ToString(this)}, ElementName={this._elementName}";
	endfunction

	// Private Functions
	function _LoadFile(this, createFile := false)
		this._file := ReadConfigFile($"{this.GetFullPath()}");
	endfunction
endclass