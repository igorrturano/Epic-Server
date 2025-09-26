include ":datafile:datafile";

class DFTools()
	var _packageName := "";
	var _fileName := "";
	var _fullPath := "";
	var _file; // DataFile reference
	var _element := struct{};
	var _property := struct{};

	function DFTools(this, packageName, fileName, elem_name := false, prop_name := false, createFile := true)
		this._packageName := CStr(packageName);
		this._fileName := CStr(fileName);
		this._fullPath := $":{this._packageName}:{this._fileName}";
		this._element := struct{
			name := error{ errortext := "Element not loaded" },
			value := error{ errortext := "Element not loaded" }
		};
		this._property := struct{
			name := error{ errortext := "Property not loaded" },
			value := error{ errortext := "Property not loaded" }
		};

		this._LoadFile(createFile);
		if (elem_name) this.SetElementName(elem_name); endif
		if (prop_name) this.SetPropertyName(prop_name);	endif
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

		result := this._element.value.PropNames();
		if (!result) result := error{ errortext := $"No properties found in element {this._element.name} of {this._fullPath}" }; endif

		return result;
	endfunction

	function GetPropertyName(this)
		return this._property.name;
	endfunction

	function GetPropertyValue(this)
		return this._property.value;
	endfunction

	function SetPackageName(this, packageName)
		this._packageName := packageName;
		return this;
	endfunction

	function SetFileName(this, fileName)
		this._fileName := fileName;
		return this;
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

	function SetPropertyName(this, prop_name)
		this._property.name := CStr(prop_name);
		this._LoadProperty(this._property.name);
		return this;
	endfunction

	function SetPropertyValue(this, byRef prop_value)
		this._property.value := prop_value;
		this._element.value.SetProp(this._property.name, this._property.value);
		return this;
	endfunction

	function Erase(this)
		return DFPurgeFile(this._fileName);
	endfunction

	function _LoadFile(this, createFile := false)
		this._file := DFOpenDataFile(this._fullPath, createFile);
	endfunction
	
	function _LoadElement(this, elem_name := false)
		if (elem_name) this._element.name := elem_name; endif
		
		this._element.value := this._file.FindElement(elem_name);

		return this._element.value;
	endfunction

	function _LoadProperty(this, prop_name := false)
		if (prop_name) this._property.name := prop_name; endif

		this._property.value := this._element.value.GetProp(this._property.name);

		return this._property.value;
	endfunction
endclass