include ":gumps:epicGumps";
include ":gumps:autoClose";

class Gump()
	var _parent;
	var _pid;
	var _gump;
	
	var _title;
	var _width;
	var _height;

	var _options;

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;

		this._title := title;
		this._width := width;
		this._height := height;

		this._options := options.size() < 1 ? array{BORDER_GOLDEN} : options;
	endfunction

	function GetParent(this)
		return this._parent;
	endfunction

	function _GetPid(this)
		return this._pid;
	endfunction

	function GetGump(this)
		return this._gump;
	endfunction

	function GetTitle(this)
		return this._title;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function GetOptions(this)
		return this._options;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;
	endfunction

	function SetTitle(this, title)
		this._title := title;
	endfunction

	function SetWidth(this, width)
		this._width := width;
	endfunction

	function SetHeight(this, height)
		this._height := height;
	endfunction

	function SetOptions(this, options)
		this._options := options;
	endfunction

	function AddOption(this, option)
		if (option in this._options) return error { errortext := "Option already set!" }; endif

		this._options.append(option);
	endfunction

	function RemoveOption(this, option)
		if (!(option in this._options)) return error { errortext := "Option not found!" }; endif

		this._options.erase(this._options.findIndex(@(v) { return v == option; }));
	endfunction

	function ToString(this)
		return "Gump: { Title: " + this._title + ", Width: " + CStr(this._width) + ", Height: " + CStr(this._height) + ", Options: " + CStr(this._options) + " }";
	endfunction

	function Show(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
		this._pid := GetPid();

		return GFSendGump(this._parent, this._gump);
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction

	function Update(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
	endfunction
endclass