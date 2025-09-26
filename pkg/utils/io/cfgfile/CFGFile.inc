use cfgfile;

class CFGFile()
	var _packageName := "";
	var _fileName := "";
	var _file;
	var _element := struct{};
	var _key := struct{};
	
	function CFGFile(this, packageName, fileName, elem_name := false, key_name := false, createFile := true)
		this._packageName := CStr(packageName);
		this._fileName := CStr(fileName);
		this._element := struct{
			name := error{ errortext := "Element not loaded" },
			value := error{ errortext := "Element not loaded" }
		};
		this._key := struct{
			name := error{ errortext := "Property not loaded" },
			value := error{ errortext := "Property not loaded" }
		};

		this._LoadFile(createFile);
		if (elem_name) this.SetElementName(elem_name); endif
		if (key_name) this.SetKeyName(key_name); endif
	endfunction

	function GetPackageName(this)
		return this._packageName;
	endfunction

	function GetFileName(this)
		return this._fileName;
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
		this._element.name := CStr(elem_name);
		this._LoadElement(this._element.name);
		return this;
	endfunction

	function SetElementValue(this, byRef elem_value)
		this._element.value := elem_value;
		return this;
	endfunction

	function SetKeyName(this, key_name, returnAsDictionary := false)
		this._key.name := CStr(key_name);
		this._LoadKey(this._key.name, returnAsDictionary);
		return this;
	endfunction

	function SetKeyValue(this, byRef key_value)
		this._key.value := key_value;
		this._element.value.SetProp(this._key.name, this._key.value);
		return this;
	endfunction

	function Unload(this)
		UnloadConfigFile(this._file);
	endfunction

	// Private Functions
	function _LoadFile(this)
		this._file := ReadConfigFile($":{this._packageName}:{this._fileName}");
	endfunction
	
	function _LoadElement(this, elem_name := false)
		if (elem_name) this._element.name := elem_name; endif

		this._element.value := FindConfigElem(this._file, elem_name);

		return this._element.value;
	endfunction

	function _LoadKey(this, key_name := false, returnAsDictionary := false)
		if (key_name) this._key.name := key_name; endif

		if (TypeOf(this._key.name) == "String")
			if (returnAsDictionary)
				this._key.value := GetConfigStringDictionary(this._file, this._key.name);
			else
				this._key.value := GetConfigStringArray(this._file, this._key.name);
				if (this._key.value.size() == 1) this._key.value := this._key.value[1]; endif
			endif
		elseif (TypeOf(this._key.name) == "Integer")
		else
			return error{ errortext := "Key name must be a string or integer" };
		endif

		return this._key.value;
	endfunction
endclass