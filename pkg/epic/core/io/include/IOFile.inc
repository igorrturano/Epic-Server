include "File";

class IOFile(File)
	var _packageName;
	var _package;
	var _fullPath;

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

	function GetFullPath(this)
		return this._fullPath;
	endfunction

	function ToString(this)
		return $"Package={this._package}, PackageName={this._packageName}, {Super::ToString(this)}, FullPath={this._fullPath}";
	endfunction

	uninit function LoadFile(this, default createFile);
endclass