use cfgfile;

include "IOFile";

class CFGFile(IOFile)
	var _elementName := "";
	var _element;
	
	function CFGFile(this, packageName, filePath, createFile := true)
		Super(packageName, filePath);
		this.LoadFile(createFile);
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
			return InvalidElementException("Element name not set");
		endif
		
		if (!this._element)
			return InvalidElementException($"Element {this._elementName} not found in {this.GetFullPath()}");
		endif

		result := ListConfigElemProps(this._element);
		if (!result)
			return InvalidPropertyException($"No properties found in element {this._elementName} of {this.GetFullPath()}");
		endif

		return result;
	endfunction

	function GetPropertyValue(this, propName, type := OT_STRING, isDictionary := false)
		var propValue;
		
		if (isDictionary)
			propValue := GetConfigStringDictionary(this._element, propName);

			if (type == OT_LONG)
				foreach value in (propValue.values())
					value := CInt(value);
				endforeach
			elseif (type == OT_DOUBLE)
				foreach value in (propValue.values())
					value := CDbl(value);
				endforeach
			endif
		else
			propValue := GetConfigStringArray(this._element, propName);

			if (type == OT_LONG)
				foreach value in (propValue)
					value := CInt(value);
				endforeach
			elseif (type == OT_DOUBLE)
				foreach value in (propValue)
					value := CDbl(value);
				endforeach
			endif
		endif

		return propValue;
	endfunction

	function LoadFile(this, createFile := false)
		this._file := ReadConfigFile($"{this.GetFullPath()}");

		return this;
	endfunction

	function Unload(this)
		UnloadConfigFile(this._file);
	endfunction

	function ToString(this)
		return $"CFGFile: {Super::ToString(this)}, ElementName={this._elementName}";
	endfunction
endclass