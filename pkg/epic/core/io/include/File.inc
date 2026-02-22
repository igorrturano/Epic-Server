include ":io:Exceptions/_all";

class File()
	var _filePath;
	var _file;

	function File(this, filePath)
		this._filePath := CStr(filePath);
	endfunction

	function GetFilePath(this)
		return this._filePath;
	endfunction
	
	function ToString(this)
		return $"FilePath={this._filePath}";
	endfunction
endclass