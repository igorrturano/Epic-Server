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

		this._gump := GFECreateGump(title, width, height, options);
	endfunction

	function Show(this)
		this._pid := GetPid();

		GFSendGump(this._parent, this._gump);
	endfunction

	function Close(this)
		GFCloseGump(this._parent, this._pid);
	endfunction

	function Update(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
		Close(this);
		Show(this);
	endfunction
endclass