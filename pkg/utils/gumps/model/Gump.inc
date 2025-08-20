include ":gumps:epicGumps";
include ":gumps:autoClose";
include ":gumps:model/Control/Control";
include ":gumps:model/Event/EventHandler";

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
			GUMP_CONTROL::TEXTCROPPED -> array{},
			GUMP_CONTROL::TOOLTIP -> array{},
			GUMP_CONTROL::IMAGEGUMP -> array{},
			GUMP_CONTROL::IMAGETILE -> array{},
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
		
		if (control.GetType() == GUMP_CONTROL::TEXT)
			id := GUMP_ID_REFERENCE_TEXT + id + this._controls[GUMP_CONTROL::TEXT].size();
		elseif (control.GetType() == GUMP_CONTROL::TEXTCROPPED)
			id := GUMP_ID_REFERENCE_TEXTCROPPED + id + this._controls[GUMP_CONTROL::TEXTCROPPED].size();
		elseif (control.GetType() == GUMP_CONTROL::TOOLTIP)
			id := GUMP_ID_REFERENCE_TOOLTIP + id + this._controls[GUMP_CONTROL::TOOLTIP].size();
		elseif (control.GetType() == GUMP_CONTROL::IMAGEGUMP)
			id := GUMP_ID_REFERENCE_IMAGEGUMP + id + this._controls[GUMP_CONTROL::IMAGEGUMP].size();
		elseif (control.GetType() == GUMP_CONTROL::IMAGETILE)
			id := GUMP_ID_REFERENCE_IMAGETILE + id + this._controls[GUMP_CONTROL::IMAGETILE].size();
		elseif (control.GetType() == GUMP_CONTROL::BUTTON)
			id := GUMP_ID_REFERENCE_BUTTON + id + this._controls[GUMP_CONTROL::BUTTON].size();
		elseif (control.GetType() == GUMP_CONTROL::CHECKBOX)
			id := GUMP_ID_REFERENCE_CHECKBOX + id + this._controls[GUMP_CONTROL::CHECKBOX].size();
		elseif (control.GetType() == GUMP_CONTROL::RADIOBUTTON)
			id := GUMP_ID_REFERENCE_RADIOBUTTON + id + this._controls[GUMP_CONTROL::RADIOBUTTON].size();
		endif

		control.SetGumpId(id);

		this._controls[control.GetType()].append(control);
	endfunction

	function ToString(this)
		return "Gump(_parent=" + this._parent
			+ ", _title=" + this._title
			+ ", _width=" + this._width
			+ ", _height=" + this._height
			+ ", _options=" + this._options
			+ ", _customization=" + this._customization
			+ ", _controls=" + this._controls
			+ ")";
	endfunction

	function CreateControls(byref dialog, dict)
		foreach control in (dict)
			control.SelfCreate(dialog);
		endforeach
	endfunction

	function Show(this)
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);

		
		foreach controlGroup in (this._controls)
			CreateControls(this._gump, controlGroup);
		endforeach

		foreach custom in (this._customization)
			custom(this._gump);
		endforeach

		this._pid := GetPid();

		var gumpResult := GFSendGump(this._parent, this._gump);
		var eventHandled;
		print("Gump sent, result: " + gumpResult[0]);

		if (gumpResult[0] == 0)
			eventHandled := EventHandler("CLOSED GUMP");
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_TEXT + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::TEXT][gumpResult[0]]-GUMP_ID_REFERENCE_TEXT);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_TEXTCROPPED + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::TEXTCROPPED][gumpResult[0]-GUMP_ID_REFERENCE_TEXTCROPPED]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_TOOLTIP + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::TOOLTIP][gumpResult[0]-GUMP_ID_REFERENCE_TOOLTIP]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_IMAGEGUMP + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::IMAGEGUMP][gumpResult[0]-GUMP_ID_REFERENCE_IMAGEGUMP]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_IMAGETILE + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::IMAGETILE][gumpResult[0]-GUMP_ID_REFERENCE_IMAGETILE]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_BUTTON + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::BUTTON][gumpResult[0]-GUMP_ID_REFERENCE_BUTTON]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_CHECKBOX + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::CHECKBOX][gumpResult[0]-GUMP_ID_REFERENCE_CHECKBOX]);
		elseif (gumpResult[0] < GUMP_ID_REFERENCE_RADIOBUTTON + GUMP_ID_REFERENCE_LIMIT)
			eventHandled := EventHandler(this._controls[GUMP_CONTROL::RADIOBUTTON][gumpResult[0]-GUMP_ID_REFERENCE_RADIOBUTTON]);
		else
			eventHandled := EventHandler(error { errortext := "Unknown control type!" });
		endif

		print("EventHandled -> " + eventHandled);
		print("EventHandled -> " + eventHandled.ToString());
		print("EventHandled -> " + eventHandled.GetEvent().ToString());
		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction
endclass

enum GUMP_ID_REFERENCE
	GUMP_ID_REFERENCE_LIMIT := 1000,
	
	GUMP_ID_REFERENCE_TEXT := 0,
	GUMP_ID_REFERENCE_TEXTCROPPED := 1000,
	GUMP_ID_REFERENCE_TOOLTIP := 2000,
	GUMP_ID_REFERENCE_IMAGEGUMP := 3000,
	GUMP_ID_REFERENCE_IMAGETILE := 4000,
	GUMP_ID_REFERENCE_BUTTON := 5000,
	GUMP_ID_REFERENCE_CHECKBOX := 6000,
	GUMP_ID_REFERENCE_RADIOBUTTON := 7000
endenum