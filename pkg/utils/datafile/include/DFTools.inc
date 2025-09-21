include ":datafile:datafile";

class DFTools()
	var _filename;

	function DFTools(this, filename)
		this._filename := filename;
	endfunction

	/*
	 * WriteValue(elem_name, prop_name, value)
	 *
	 * Purpose: Writes a property value to a specific element in the data file
	 *
	 * Parameters:
	 *	elem_name: String - Name of the element to write to (created if doesn't exist)
	 *	prop_name: String - Name of the property to set
	 *	value:     Mixed  - Value to assign to the property
	 *
	 * Return Value:
	 *	Boolean - Always returns true on successful operation
	 *
	 * Notes:
	 *	- Opens the data file in create mode (creates file if it doesn't exist)
	 *	- Creates the element if it doesn't exist in the file
	 *	- Overwrites existing property values
	 */
	function WriteValue(this, elem_name, prop_name, value)
		var file := DFOpenDataFile(this._filename, DF_CREATE);
		var elem := DFFindElement(file, elem_name, DF_CREATE);
		elem.SetProp(prop_name, value);

		return true;
	endfunction

	/*
	 * LoadValue(elem_name, prop_name)
	 *
	 * Purpose: Loads a specific property value from an element in the data file
	 *
	 * Parameters:
	 *   elem_name - String: Name of the element to find in the data file
	 *   prop_name - String: Name of the property to retrieve from the element
	 *
	 * Returns:
	 *   Mixed: The value of the specified property if found
	 *   Error: Returns error struct with errortext if element or property not found
	 *
	 * Notes:
	 *   - Opens the data file specified in this.filename
	 *   - Searches for the specified element by name
	 *   - Retrieves the specified property from the found element
	 *   - Returns descriptive error messages for debugging
	 */
	function LoadValue(this, elem_name, prop_name)
		var value;
		var file := OpenDataFile(this._filename);
		var elem := file.FindElement(elem_name);

		if (!elem) return error{ errortext := $"Element {elem_name} not found in {this._filename}" }; endif
		value := elem.GetProp(prop_name);
		if (!value) return error{ errortext := $"Property {prop_name} not found in element {elem_name}" }; endif

		return value;
	endfunction

	/**
	 * GetElementKeys(this, elem_key)
	 *
	 * Purpose: Retrieves the names of all properties for a specific element in the data file.
	 *
	 * Parameters:
	 * this: The DFTools instance containing the filename of the data file.
	 * elem_key: The key (name) of the element whose properties are to be retrieved.
	 *
	 * Returns:
	 * An array of property names for the specified element, or an error if the element is not found.
	 */
	function GetElementKeys(this, elem_key)
		var file := DFOpenDataFile(this._filename, DF_CREATE);
		var elem := file.FindElement(elem_key);
		if (!elem) return error{ errortext := $"Element {elem_key} not found in {this._filename}" }; endif

		var props := elem.PropNames();
		if (!props) return error{ errortext := $"No properties found in element {elem_key} of {this._filename}" }; endif

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
		DFPurgeFile(this._filename);
	endfunction
endclass