include ":gumps:epicGumps";
include ":gumps:autoClose";
include ":gumps:model/Control";

class Gump()
	var _parent;
	var _pid;
	var _gump;
	
	var _title;
	var _width;
	var _height;

	var _options;
	var _customization;
	var _controls;

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;

		this._title := title;
		this._width := width;
		this._height := height;

		this._options := options.size() < 1 ? array{BORDER_GOLDEN} : options;
		this._customization := array{};
		this._controls := dictionary{
			GUMP_CONTROL_TEXT -> array{},
			GUMP_CONTROL_TOOLTIP -> array{},
			GUMP_CONTROL_IMAGE -> array{},
			GUMP_CONTROL_BUTTON -> array{},
			GUMP_CONTROL_CHECKBOX -> array{},
			GUMP_CONTROL_RADIOBUTTON -> array{}
		};
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

	function AddCustom(this, customFunction)
		this._customization.append(customFunction);
	endfunction

	// function AddControl(this, type, name, value, design)
	function AddControl(this, control)
		var id := 1;
		case (control.GetType())
			GUMP_CONTROL_TEXT: id := this._controls[GUMP_CONTROL_TEXT].size() + id; break;
			GUMP_CONTROL_TOOLTIP: id := this._controls[GUMP_CONTROL_TOOLTIP].size() + id + 1000; break;

			GUMP_CONTROL_IMAGE: id := this._controls[GUMP_CONTROL_IMAGE].size() + id + 2000; break;

			GUMP_CONTROL_BUTTON: id := this._layoutElements[GUMP_CONTROL_BUTTON].size() + id + 3000; break;
			GUMP_CONTROL_CHECKBOX: id := this._controls[GUMP_CONTROL_CHECKBOX].size() + id + 4000; break;
			GUMP_CONTROL_RADIOBUTTON: id := this._controls[GUMP_CONTROL_RADIOBUTTON].size() + id + 5000; break;
		endcase

		control.SetId(id);
		print(control.ToString());

		var controlS := struct{
			id := id,
			type := control.GetType(),
			name := control._GetName(),
			value := control.GetValue(),

			color := control.GetDesign().color,
			x := control.GetDesign().x,
			y := control.GetDesign().y,
			width := control.GetDesign().width,
			height := control.GetDesign().height
		};

		this._controls[control.GetType()].append(control);
	endfunction

	function RemoveOption(this, option)
		if (!(option in this._options)) return error { errortext := "Option not found!" }; endif

		this._options.erase(this._options.findIndex(@(v) { return v == option; }));
	endfunction

	function ToString(this)
		return "Gump: { Title: " + this._title + ", Width: " + CStr(this._width) + ", Height: " + CStr(this._height) + ", Options: " + CStr(this._options) + " }";
	endfunction

	function Create(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
	endfunction
	
	function Show(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);

		foreach text in (this._controls[GUMP_CONTROL_TEXT])
			GFTextCrop(this._gump, text.GetDesign().x, text.GetDesign().y, text.GetDesign().width, text.GetDesign().height, text.GetDesign().color, text.GetValue());
			// this.AddLayout(GUMP_LAYOUT_TEXT, text.name, text.value, text.x, text.y);
		endforeach

		foreach custom in (this._customization)
			custom(this._gump);
		endforeach
		this._pid := GetPid();

		var gumpResult := GFSendGump(this._parent, this._gump);
		if (gumpResult < 1000)
			return this._controls[GUMP_CONTROL_TEXT][gumpResult];
		elseif (gumpResult < 2000)
			return this._controls[GUMP_CONTROL_TOOLTIP][gumpResult-1000];
		elseif (gumpResult < 3000)
			return this._controls[GUMP_CONTROL_IMAGE][gumpResult-2000];
		elseif (gumpResult < 4000)
			return this._controls[GUMP_CONTROL_BUTTON][gumpResult-3000];
		elseif (gumpResult < 5000)
			return this._controls[GUMP_CONTROL_CHECKBOX][gumpResult-4000];
		elseif (gumpResult < 6000)
			return this._controls[GUMP_CONTROL_RADIOBUTTON][gumpResult-5000];
		else
			return gumpResult;
		endif

		// return GFSendGump(this._parent, this._gump);
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction
endclass

enum TEXT_COLOR
	TEXT_COLOR_DEFAULT := 1,

	TEXT_COLOR_BLACK := 1,
	TEXT_COLOR_RED := 38,
	TEXT_COLOR_GREEN := 68,
	TEXT_COLOR_BLUE := 90,
	TEXT_COLOR_MAUVE := 30,
	TEXT_COLOR_LAVENDER := 20,
	TEXT_COLOR_BROWN := 46,
	TEXT_COLOR_YELLOW := 53,

	TEXT_COLOR_ERROR := TEXT_COLOR_RED,
	TEXT_COLOR_ALERT := TEXT_COLOR_YELLOW
endenum