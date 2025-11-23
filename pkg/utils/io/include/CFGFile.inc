use cfgfile;

include "model/IOFile";

class CFGFile(IOFile)
	var _element := struct{};
	var _key := struct{};
	
	function CFGFile(this, packageName, fileName, elem_name := false, key_name := false, createFile := true)
		Super(packageName, fileName);
		this._LoadFile(createFile);
		
		this._element := struct{
			name := error{ errortext := "Element not loaded" },
			value := error{ errortext := "Element not loaded" }
		};
		this._key := struct{
			name := error{ errortext := "Property not loaded" },
			value := error{ errortext := "Property not loaded" }
		};

		if (elem_name) this.SetElementName(elem_name); endif
		if (key_name) this.SetKeyName(key_name); endif
	endfunction

	function GetElementName(this)
		return this._element.name;
	endfunction

	function GetElementValue(this)
		return this._element.value;
	endfunction

	function GetElementKeys(this, elem_name := false)
		var result;

		if (elem_name) this._element.name := elem_name; endif
		this._LoadElement(this._element.name);

		if (!this._element.value) result := error{ errortext := $"Element {this._element.name} not found in {this._fullPath}" }; endif

		result := ListConfigElemProps(this._element.value);
		if (!result) result := error{ errortext := $"No properties found in element {this._element.name} of {this._fullPath}" }; endif

		return result;
	endfunction

	function GetKeyName(this)
		return this._key.name;
	endfunction

	function GetKeyValue(this)
		return this._key.value;
	endfunction

	function SetElementName(this, elem_name)
		this._element.name := elem_name;
		this._LoadElement(this._element.name);
		return this;
	endfunction

	function SetKeyName(this, key_name, isDictionary := false)
		this._key.name := key_name;
		this._LoadKey(this._key.name, isDictionary);
		return this;
	endfunction

	function Unload(this)
		UnloadConfigFile(this._file);
	endfunction

	function ToString(this)
		return $"CFGFile: Package: {this._packageName}, File: {this._fileName}, FullPath: {this._fullPath}, Element: {this._element.name}, Key: {this._key.name}";
	endfunction

	// Private Functions
	function _LoadFile(this, createFile := false)
		this._file := ReadConfigFile($":{this._packageName}:{this._fileName}");
	endfunction
	
	function _LoadElement(this, elem_name := false)
		if (elem_name) this._element.name := elem_name; endif

		this._element.value := FindConfigElem(this._file, elem_name);

		return this._element.value;
	endfunction

	function _LoadKey(this, key_name := false, isDictionary := false)
		if (key_name) this._key.name := key_name; endif

		if (TypeOf(this._key.name) == "String")
			if (isDictionary)
				this._key.value := GetConfigStringDictionary(this._element.value, this._key.name);
			else
				this._key.value := GetConfigStringArray(this._element.value, this._key.name);
				// if (this._key.value.size() > 1)
				// 	this._key.value := GetConfigStringArray(this._element.value, this._key.name);
				// else
				// 	this._key.value := GetConfigString(this._element.value, this._key.name);
				// endif
			endif
		elseif (TypeOf(this._key.name) == "Integer")
		else
			return error{ errortext := "Key name must be a 'string' or 'integer'" };
		endif

		return this._key.value;
	endfunction
endclass