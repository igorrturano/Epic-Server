include ":datafile:datafile";

class DFTools()
	var _packageName := "";
	var _fileName := "";
	var _file;
	var _element := struct{};
	var _property := struct{};

	function DFTools(this, packageName, fileName, elem_name := false, prop_name := false, createFile := true)
		this._packageName := CStr(packageName);
		this._fileName := CStr(fileName);
		this._file := this._LoadFile(createFile);
		this._element := struct{
			name := error{ errortext := "Element not loaded" },
			value := error{ errortext := "Element not loaded" }
		};
		this._property := struct{
			name := error{ errortext := "Property not loaded" },
			value := error{ errortext := "Property not loaded" }
		};

		if (elem_name)
			this._element.name := CStr(elem_name);
			this._element.value := this._LoadElement(this._element.name);
		endif

		if (prop_name)
			this._property.name := CStr(prop_name);
			this._property.value := this._LoadProperty(this._property.name);
		endif
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

	function GetPropertyName(this)
		return this._property.name;
	endfunction

	function GetPropertyValue(this)
		// this._Read();
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
		this._element.name := elem_name;
		return this;
	endfunction

	function SetElementValue(this, byRef elem_value)
		this._element.value := elem_value;
		return this;
	endfunction

	function SetPropertyName(this, prop_name)
		this._property.name := prop_name;
		return this;
	endfunction

	function SetPropertyValue(this, byRef prop_value)
		this._property.value := prop_value;
		this._Write();
		return this;
	endfunction

	function _Write(this, elem_name := false, prop_name := false, value := false)
		this._LoadElement(elem_name);
		this._element.value := DFFindElement(this._packageName + this._fileName, elem_name, DF_CREATE);
		this._element.value.SetProp(prop_name, value);

		return this;
	endfunction

	function _Read(this, elem_name := false, prop_name := false)
		this._LoadElement(this._element.name);

		if (!this._element.value) return error{ errortext := $"Element {elem_name} not found in {this._packageName}" }; endif
		this._property.value := this._element.value.GetProp(prop_name);
		if (!this._property.value) return error{ errortext := $"Property {prop_name} not found in element {elem_name}" }; endif

		return this._property.value;
	endfunction

	function GetElementKeys(this, elem_key := false)
		if (!this._element.value) return error{ errortext := $"Element {elem_key} not found in {this._packageName}" }; endif

		var props := this._element.value.PropNames();
		if (!props) return error{ errortext := $"No properties found in element {elem_key} of {this._packageName}" }; endif

		return props;
	endfunction

	/**
	 * Erase(this)
	 *
	 * Apaga todos os dados do arquivo de dados associado a este objeto.
	 *
	 * Parameters
	 * this: O objeto Datafile cuja propriedade 'filename' especifica o arquivo a ser apagado.
	 *
	 */
	function Erase(this)
		return DFPurgeFile(this._filename);
	endfunction

	function _LoadFile(this, createFile := false)
		this._file := DFOpenDataFile($":{this._packageName}:{this._fileName}", createFile);
	endfunction
	
	function _LoadElement(this, elem_name := false)
		if (elem_name) this._element.name := elem_name; endif
		
		this._element.value := this._file.FindElement(elem_name);

		return this._element;
	endfunction

	function _LoadProperty(this, prop_name := false)
		if (prop_name) this._property.name := prop_name; endif

		this._property.value := this._element.value.FindProp(this._property.name);

		return this._property;
	endfunction
endclass