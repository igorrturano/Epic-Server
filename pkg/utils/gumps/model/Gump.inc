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

	function AddControl(this, control)
		var id := 1;
		
		case (control.GetType())
			GUMP_CONTROL_TEXT: id := this._controls[GUMP_CONTROL_TEXT].size() + id; break;
			GUMP_CONTROL_TOOLTIP: id := this._controls[GUMP_CONTROL_TOOLTIP].size() + id + 1000; break;

			GUMP_CONTROL_IMAGE: id := this._controls[GUMP_CONTROL_IMAGE].size() + id + 2000; break;

			GUMP_CONTROL_BUTTON: id := this._controls[GUMP_CONTROL_BUTTON].size() + id + 3000; break;
			GUMP_CONTROL_CHECKBOX: id := this._controls[GUMP_CONTROL_CHECKBOX].size() + id + 4000; break;
			GUMP_CONTROL_RADIOBUTTON: id := this._controls[GUMP_CONTROL_RADIOBUTTON].size() + id + 5000; break;
		endcase
		print ("id->" + id);

		control.SetGumpId(id);
		print("added control->" + control.ToString());

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
			GFTextLine(this._gump, text.GetX(), text.GetY(), text.GetColor(), text.GetValue());
			// this.AddLayout(GUMP_LAYOUT_TEXT, text.name, text.value, text.x, text.y);
		endforeach

		foreach button in (this._controls[GUMP_CONTROL_BUTTON])
			GFAddButton(this._gump, button.GetX(), button.GetY(), button.GetImage(), button.GetPressedImage(), GF_CLOSE_BTN, button.GetGumpId());
		endforeach

		foreach custom in (this._customization)
			custom(this._gump);
		endforeach
		this._pid := GetPid();

		var gumpResult := GFSendGump(this._parent, this._gump);
		print("Gump sent, result: " + gumpResult[0]);
		if (gumpResult[0] < 1000)
			return this._controls[GUMP_CONTROL_TEXT][gumpResult[0]];
		elseif (gumpResult[0] < 2000)
			return this._controls[GUMP_CONTROL_TOOLTIP][gumpResult[0]-1000];
		elseif (gumpResult[0] < 3000)
			return this._controls[GUMP_CONTROL_IMAGE][gumpResult[0]-2000];
		elseif (gumpResult[0] < 4000)
			print("EventHandled -> " + this._controls[GUMP_CONTROL_BUTTON][gumpResult[0]-3000].ToString());
			return this._controls[GUMP_CONTROL_BUTTON][gumpResult[0]-3000];
		elseif (gumpResult[0] < 5000)
			return this._controls[GUMP_CONTROL_CHECKBOX][gumpResult[0]-4000];
		elseif (gumpResult[0] < 6000)
			return this._controls[GUMP_CONTROL_RADIOBUTTON][gumpResult[0]-5000];
		else
			return gumpResult[0];
		endif

		// return GFSendGump(this._parent, this._gump);
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction
endclass