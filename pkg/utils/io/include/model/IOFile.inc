include ":io:Exceptions/_all";

class IOFile()
	var _packageName;
	var _package;
	var _filePath;
	var _fullPath;
	var _file;

	function IOFile(this, packageName, filePath)
		this._packageName := CStr(packageName);
		this._package := $":{this._packageName}:";
		this._filePath := CStr(filePath);
		this._fullPath := $"{this._package}{this._filePath}";
	endfunction

	function GetPackageName(this)
		return this._packageName;
	endfunction

	function GetPackage(this)
		return this._package;
	endfunction

	function GetFilePath(this)
		return this._filePath;
	endfunction
	
	function GetFullPath(this)
		return this._fullPath;
	endfunction

	function ToString(this)
		return $"Package={this._package}, PackageName={this._packageName}, FilePath={this._filePath}, FullPath={this._fullPath}";
	endfunction

	uninit function GetElementName(this);
	uninit function SetElementName(this, default elemName);
	uninit function GetElementProps(this);
	uninit function GetPropertyValue(this, default propName);
	uninit function LoadFile(this, default createFile);
endclass