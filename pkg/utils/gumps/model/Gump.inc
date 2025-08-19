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
			GUMP_CONTROL::TEXT -> array{},
			GUMP_CONTROL::TOOLTIP -> array{},
			GUMP_CONTROL::IMAGE -> array{},
			GUMP_CONTROL::BUTTON -> array{},
			GUMP_CONTROL::CHECKBOX -> array{},
			GUMP_CONTROL::RADIOBUTTON -> array{}
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
			GUMP_CONTROL::TEXT: id := this._controls[GUMP_CONTROL::TEXT].size() + id; break;
			GUMP_CONTROL::TOOLTIP: id := this._controls[GUMP_CONTROL::TOOLTIP].size() + id + 1000; break;

			GUMP_CONTROL::IMAGE: id := this._controls[GUMP_CONTROL::IMAGE].size() + id + 2000; break;

			GUMP_CONTROL::BUTTON: id := this._controls[GUMP_CONTROL::BUTTON].size() + id + 3000; break;
			GUMP_CONTROL::CHECKBOX: id := this._controls[GUMP_CONTROL::CHECKBOX].size() + id + 4000; break;
			GUMP_CONTROL::RADIOBUTTON: id := this._controls[GUMP_CONTROL::RADIOBUTTON].size() + id + 5000; break;
		endcase

		control.SetGumpId(id);

		this._controls[control.GetType()].append(control);
	endfunction

	function ToString(this)
		return "Gump: { Title: " + this._title + ", Width: " + CStr(this._width) + ", Height: " + CStr(this._height) + ", Options: " + CStr(this._options) + " }";
	endfunction

	function CreateControls(byref dialog, dict)
		foreach control in (dict)
			control.SelfCreate(dialog);
		endforeach
	endfunction

	function Show(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);

		CreateControls(this._gump, this._controls[GUMP_CONTROL::TEXT]);
		CreateControls(this._gump, this._controls[GUMP_CONTROL::TOOLTIP]);
		CreateControls(this._gump, this._controls[GUMP_CONTROL::IMAGE]);
		CreateControls(this._gump, this._controls[GUMP_CONTROL::BUTTON]);
		CreateControls(this._gump, this._controls[GUMP_CONTROL::CHECKBOX]);
		CreateControls(this._gump, this._controls[GUMP_CONTROL::RADIOBUTTON]);

		foreach custom in (this._customization)
			custom(this._gump);
		endforeach
		this._pid := GetPid();

		var gumpResult := GFSendGump(this._parent, this._gump);
		var eventHandled;
		print("Gump sent, result: " + gumpResult[0]);

		if (gumpResult[0] == 0)
			eventHandled := "CLOSED";
		elseif (gumpResult[0] < 1000)
			eventHandled := this._controls[GUMP_CONTROL::TEXT][gumpResult[0]];
		elseif (gumpResult[0] < 2000)
			eventHandled := this._controls[GUMP_CONTROL::TOOLTIP][gumpResult[0]-1000];
		elseif (gumpResult[0] < 3000)
			eventHandled := this._controls[GUMP_CONTROL::IMAGE][gumpResult[0]-2000];
		elseif (gumpResult[0] < 4000)
			eventHandled := this._controls[GUMP_CONTROL::BUTTON][gumpResult[0]-3000];
		elseif (gumpResult[0] < 5000)
			eventHandled := this._controls[GUMP_CONTROL::CHECKBOX][gumpResult[0]-4000];
		elseif (gumpResult[0] < 6000)
			eventHandled := this._controls[GUMP_CONTROL::RADIOBUTTON][gumpResult[0]-5000];
		else
			eventHandled := error { errortext := "Unknown control type!" };
		endif

		print("EventHandled -> " + eventHandled);
		print("EventHandled -> " + eventHandled.ToString());
		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction
endclass