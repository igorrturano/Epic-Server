include "IOFile";

include ":datafile:datafile";

class DataFile(IOFile)
	var _elementName := "";
	var _element;
	
	function DataFile(this, packageName, fileName, createFile := true)
		Super(packageName, fileName);
		this.LoadFile(createFile);
	endfunction

	function GetElementName(this)
		return this._elementName;
	endfunction

	function SetElementName(this, elem_name)
		this._elementName := elem_name;
		this._element := this._file.FindElement(elem_name);

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

		result := this._element.PropNames();
		if (!result)
			return InvalidPropertyException($"No properties found in element {this._elementName} of {this.GetFullPath()}");
		endif

		return result;
	endfunction
	
	function DeleteElement(this)
		this._file.DeleteElement(this._elementName);
		
		return this;
	endfunction

	function GetPropertyValue(this, propName)
		return this._element.GetProp(propName);
	endfunction

	function SetPropertyValue(this, value)
		this._element.SetProp(this._elementName, value);

		return this;
	endfunction

	function DeleteProperty(this, propName)
		if (!this._element)
			return InvalidElementException();
		endif

		this._element.EraseProp(propName);

		return this;
	endfunction

	function LoadFile(this, createFile := false)
		this._file := DFOpenDataFile(this._fullPath, createFile);

		return this;
	endfunction

	function EraseFile(this)
		foreach elemName in (this._file.GetElemNames())
			this._file.DeleteElement(elemName);
			SleepMS(2);
		endforeach
	endfunction

	function ToString(this)
		return $"DataFile: {Super::ToString(this)}, ElementName={this._elementName}";
	endfunction
endclass