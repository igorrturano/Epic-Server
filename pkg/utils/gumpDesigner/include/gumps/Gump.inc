include "IGump";

include ":gumps:autoClose";

// include ":gumpDesigner:gumps/DiagFactory";
include ":gumpDesigner:controls/Control";
include ":gumpDesigner:controls/Button";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:layouts/GoldenLayout";
include ":gumpDesigner:layouts/GumpTitle";
include ":gumpDesigner:layouts/GumpBody";
include ":gumpDesigner:events/GumpEvent";
include ":gumpDesigner:exceptions/NoEventHandledException";
include ":gumpDesigner:exceptions/InvalidOptionException";

include ":message:model/Message";

class Gump(IGump)
	var _parent;
	var _pid;
	var _gump;
	var _gumpResult;
	var _width := 0;
	var _height := 0;

	var _layout; // Objeto GumpLayout (ou algum herdado dele)
	var _title; // Objeto GumpTitle
	var _pages := array{}; // Array das páginas, cada página é o Objeto GumpBody

	var _options := array{};
	var _controls := struct{
			preStored := array{},
			idOrder := array{},
			lastId := dictionary{},
			position := dictionary{}
		};
	var helpAction;

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;
		this._width := width;
		this._height := height;

		this.SetLayout(GoldenLayout(width, height));
		this.SetTitle(GumpTitle(title));
		this.AddPage(GumpBody());

		this._options := options.size() < 1 ? array{BORDER_GOLDEN} : options;
		this._controls := Gump::_controls;
		this.helpAction := @() {
			Message("Esta ajuda ainda não foi implementada.").SetColor(Color(SYSTEM_COLOR::ALERT)).SendSys(this.GetParent());
		};
	endfunction

	function GetParent(this)
		return this._parent;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;

		return this;
	endfunction

	function _GetPid(this)
		return this._pid;
	endfunction

	function GetGump(this)
		return this._gump;
	endfunction

	function SetGump(this, gumpStr)
		this._gump := gumpStr;

		return this;
	endfunction

	function GetGumpResult(this)
		return this._gumpResult;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function SetWidth(this, width)
		this._width := CInt(width);

		return this;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function SetHeight(this, height)
		this._height := CInt(height);

		return this;
	endfunction

	function GetLayout(this)
		return this._gumpLayout;
	endfunction

	function SetLayout(this, layout)
		if (!(layout is @GumpLayout))
			return InvalidParameterException("Layout must be of type GumpLayout.");
		endif
		
		this._gumpLayout := layout;
		
		return this;
	endfunction

	function GetTitle(this)
		return this._title;
	endfunction

	function SetTitle(this, title)
		if (!(title is @GumpTitle))
			return InvalidParameterException("Title must be of type GumpTitle.");
		endif

		this._title := title;

		return this;
	endfunction

	function GetPage(this, pageNumber)
		if (pageNumber < 1 || pageNumber > this._pages.size())
			return InvalidParameterException("Page number out of range.");
		endif

		return this._pages[pageNumber];
	endfunction

	function SetPage(this, pageNumber, page)
		if (pageNumber < 1 || pageNumber > this._pages.size())
			return InvalidParameterException("Page number out of range.");
		endif

		if (!(page is @GumpBody))
			return InvalidParameterException("Page must be of type GumpBody.");
		endif

		this._pages[pageNumber] := page;

		return this;
	endfunction

	function AddPage(this, page)
		if (!(page is @GumpBody))
			return InvalidParameterException("Page must be of type GumpBody.");
		endif

		this._pages.append(page);

		return this;
	endfunction

	function GetOptions(this)
		return this._options;
	endfunction

	function AddOption(this, option)
		if (option in this._options)
			return InvalidOptionException("Option already set!");
		endif

		this._options.append(option);

		return this;
	endfunction

	function GetControls(this)
		return this._controls.preStored;
	endfunction

	function AddControl(this, control)
		if (!(control is @Control))
			return InvalidParameterException("Only controls of type Control can be added to the gump.");
		endif

		this._controls.preStored.append(control);

		return this;
	endfunction

	function AddCustomization(this, byRef customFunction)
		this._customization.append(customFunction);
		
		return this;
	endfunction

	function SetHelpAction(this, byRef helpAction)
		this.helpAction := helpAction;
		
		return this;
	endfunction

	function ToString(this)
		return "Gump(_parent=" + this._parent
			+ ", _title=" + this._title
			+ ", _width=" + this._width
			+ ", _height=" + this._height
			+ ", _options=" + this._options
			+ ", _customization=" + this._customization
			+ ", _controls=" + this.GetControls()
			+ ")";
	endfunction

	function Show(this)
		var eventHandled, gumpResult;
		
		this._CreateHelper();
		this._CreateControls();
		this._CreateCustoms();

		this._pid := GetPid();
		this._gumpResult := GFSendGump(this._parent, this._gump);

		if (!this._gumpResult[0])
			return NoEventHandledException("No event was handled for this gump.");
		else
			eventHandled := GumpEvent(this.GetControls().position[this._gumpResult[0]]);
		endif

		this._UpdateInputValues();
		eventHandled.GetControl().ExecuteActions();

		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction

	function Dispose(this)
		this._gump := Gump::_gump;
		this._gumpResult := Gump::_gumpResult;
		foreach control in (this.GetControls().position)
			control.Dispose();
			sleepms(2);
		endforeach
		this._controls := Gump::_controls;
	endfunction

	// Private functions
	function _GetIdOffsetByControlType(controlType)
		case (controlType) // Feito com valores fixos pra poder ter um controle melhor da resposta no input[0]
			GumpControl::Text: return 0;
			GumpControl::TextCropped: return 1000;
			GumpControl::ImageGump: return 2000;
			GumpControl::ImageGumpTiled: return 3000;
			GumpControl::ImageTiled: return 4000;
			GumpControl::Button: return 5000;
			GumpControl::CheckBox: return 6000;
			GumpControl::RadioButton: return 7000;
			GumpControl::TextBox: return 8000;
			GumpControl::Tooltip: return 9000;
			GumpControl::RadioGroup: return 10000;
			GumpControl::Container: return 11000;
			GumpControl::ContainerGridLayout: return 12000;

			default: return 0;
		endcase
	endfunction

	function _CreateControls(this)
		foreach id in (this.GetControls().idOrder)
			this.GetControls().position[id].SelfCreate(this._gump);
		endforeach
	endfunction

	function _CreateControl(this, byRef control)
		if (!(control is @Control))
			return InvalidParameterException("Only controls of type Control can be created in the gump.");
		endif
		
		if (!this._controls.lastId.exists(control.GetType()))
			this._controls.lastId[control.GetType()] := _GetIdOffsetByControlType(control.GetType());
		endif

		this._controls.lastId[control.GetType()]++;
		control.SetId(this._controls.lastId[control.GetType()]);
		this._controls.idOrder.append(control.GetId());	
		this._controls.position[control.GetId()] := control;
		
		if (control is @Container)
			foreach childControl in (control.GetControls())
				this.AddControl(childControl);
			endforeach
		endif

		return this;
	endfunction

	function _CreateCustoms(this)
		foreach custom in (this._customization)
			custom(this._gump);
		endforeach
	endfunction

	function _UpdateInputValues(this)
		foreach id in (this.GetControls().idOrder)
			var control := this.GetControls().position[id];
			control.SetValue(GFExtractData(this._gump, control.GetId()));
		endforeach
	endfunction

	function _CreateHelper(this)
		if (this.GetOptions().contains(HIDE_TITLE))
			return;
		endif
		
		var buttonHelp := Button("Help", this.GetWidth() - 30, 36, 0x5689, 0x568A);
		buttonHelp.AddAction(this.helpAction);

		this.GetLayout().AddControl(buttonHelp);
	endfunction
endclass
