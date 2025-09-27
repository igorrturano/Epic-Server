class IOFile()
	var _packageName;
	var _fileName;
	var _fullPath;
	var _file;

	function IOFile(this, packageName, fileName)
		this._packageName := CStr(packageName);
		this._fileName := CStr(fileName);
		this._fullPath := $":{this._packageName}:{this._fileName}";
	endfunction

	function GetPackageName(this)
		return this._packageName;
	endfunction

	function GetFileName(this)
		return this._fileName;
	endfunction

	function GetFullPath(this)
		return this._fullPath;
	endfunction

	function ToString(this)
		return this._fullPath;
	endfunction

	uninit function _LoadFile(this, default createFile); // Implementar nas subclasses a forma de carregar o arquivo
endclass