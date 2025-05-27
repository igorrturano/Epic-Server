include ":datafile:datafile";

class DFTools()
	var filename;

	function DFTools(this, _filename)
		this.filename := _filename;
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
		var file := DFOpenDataFile(this.filename, DF_CREATE);
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
		var file := OpenDataFile(this.filename);
		var elem := file.FindElement(elem_name);

		if (!elem) return error{ errortext := $"Element {elem_name} not found in {this.filename}" }; endif
		value := elem.GetProp(prop_name);
		if (!value) return error{ errortext := $"Property {prop_name} not found in element {elem_name}" }; endif

		return value;
	endfunction

	function DeleteElement(this, elem_name)
		var file := DFOpenDataFile(this.filename, DF_CREATE);
		var elem := DFFindElement(file, elem_name, DF_CREATE);

		if (!elem) return error{ errortext := $"Element {elem_name} not found in {this.filename}" }; endif
		file.DeleteElement(elem_name);

		return true;
	endfunction

	function DeleteProp(this, elem_name, prop_name)
		var file := DFOpenDataFile(this.filename, DF_CREATE);
		var elem := DFFindElement(file, elem_name, DF_CREATE);

		if (!elem) return error{ errortext := $"Element {elem_name} not found in {this.filename}" }; endif
		elem.EraseProp(prop_name);

		return true;
	endfunction

	/**
	 * GetAllElements(this)
	 *
	 * Recupera todos os elementos presentes no arquivo de dados associado ao objeto.
	 *
	 * Parâmetros
	 * this: Referência ao objeto que contém o atributo 'filename' com o caminho do arquivo de dados.
	 *
	 * Valor de retorno
	 * Retorna uma lista com todas as chaves (elementos) encontradas no arquivo de dados.
	 * Em caso de erro ao abrir o arquivo ou se nenhum elemento for encontrado, retorna um objeto de erro com a propriedade 'errortext'.
	 */
	function GetAllElements(this)
		var elems;
		var file := OpenDataFile(this.filename);
		if (!file) return error{ errortext := $"Could not open data file {this.filename}" }; endif
		elems := file.Keys();
		if (!elems) return error{ errortext := $"No elements found in {this.filename}" }; endif

		return elems;
	endfunction

	/**
	 * GetElementPropNames(this, elem_key)
	 *
	 * Obtém os nomes das propriedades de um elemento específico em um arquivo de dados
	 *
	 * Parameters
	 * this: Objeto contendo o nome do arquivo de dados (this.filename)
	 * elem_key: Chave/identificador do elemento a ser pesquisado
	 *
	 * Return value
	 * Array com os nomes das propriedades do elemento ou erro caso:
	 * - O arquivo não possa ser aberto
	 * - O elemento não seja encontrado
	 * - O elemento não possua propriedades
	 */
	function GetElementPropNames(this, elem_key)
		var file := OpenDataFile(this.filename);
		if (!file) return error{ errortext := $"Could not open data file {this.filename}" }; endif

		var elem := file.FindElement(elem_key);
		if (!elem) return error{ errortext := $"Element {elem_key} not found in {this.filename}" }; endif

		var props := elem.PropNames();
		if (!props) return error{ errortext := $"No properties found in element {elem_key} of {this.filename}" }; endif

		return props;
	endfunction

	function Erase(this)
		DFPurgeFile(this.filename);
	endfunction
endclass