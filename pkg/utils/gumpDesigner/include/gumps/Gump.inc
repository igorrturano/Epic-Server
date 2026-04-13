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
	var _gumpString;

	var _layout; // Objeto GumpLayout (ou algum herdado dele)
	var _title; // Objeto GumpTitle
	var _pages := array{}; // Array das páginas, cada página é o Objeto GumpBody

	var _options := array{};
	var _addedControls := array{};
	var _createdControls := struct{
			orderById := array{},
			countByType := dictionary{},
			positionById := dictionary{}
		};
	var helpAction;

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;

		this.SetLayout(GoldenLayout(CInt(width), CInt(height)));
		this.SetTitle(GumpTitle(CStr(title)));
		this._pages := Gump::_pages;
		this.AddPage(GumpBody());

		this._options := options;
		this._addedControls := Gump::_addedControls;
		this._createdControls := Gump::_createdControls;
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

	function GetGumpString(this)
		return this._gumpString;
	endfunction

	function SetGumpString(this, gumpStr)
		this._gumpString := gumpStr;

		return this;
	endfunction

	function GetX(this)
		return this.GetLayout().GetX();
	endfunction

	function SetX(this, x)
		this.GetLayout().SetX(CInt(x));

		return this;
	endfunction

	function GetY(this)
		return this.GetLayout().GetY();
	endfunction

	function SetY(this, y)
		this.GetLayout().SetY(CInt(y));

		return this;
	endfunction

	function GetWidth(this)
		return this.GetLayout().GetWidth();
	endfunction

	function SetWidth(this, width)
		this.GetLayout().SetWidth(CInt(width));

		return this;
	endfunction

	function GetHeight(this)
		return this.GetLayout().GetHeight();
	endfunction

	function SetHeight(this, height)
		this.GetLayout().SetHeight(CInt(height));

		return this;
	endfunction

	function GetLayout(this)
		return this._gumpLayout;
	endfunction

	function SetLayout(this, layout)
		// print ("Setting layout: " + layout.ToString());
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

		if (!this._pages)
			this._pages := array{};
		endif

		if (this._pages.size() >= 10)
			return InvalidParameterException("A gump cannot have more than 10 pages.");
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
		return this._addedControls;
	endfunction

	function AddControl(this, byRef control)
		if (!(control is @Control))
			return InvalidParameterException("Only controls of type Control can be added to the gump.");
		endif

		this._addedControls.append(control);

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
		return "(Gump) title=" + this.GetTitle().ToString();
			+ ", layout=" + this.GetLayout().ToString()
			+ ", parent=" + this.GetParent()
			+ ", width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", options=" + this.GetOptions()
			+ ", controls=" + this.GetControls().size()
			+ ", pages=" + this._pages.size();
	endfunction

	function Show(this)
		this._pid := GetPid();

		this._CreateHelpButton();
		this._ConfigureDiag();
		this._DrawDiag();


		print(this.GetGumpString());
		var result := GFSendGump(this._parent, this.GetGumpString());
		
		var eventHandled;
		if (!result[0])
			return NoEventHandledException("No event was handled for this gump.");
		else
			eventHandled := GumpEvent(this.GetControls().position[result[0]]);
		endif

		this._UpdateInputValues();
		eventHandled.GetControl().ExecuteActions();

		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction

	function Dispose(this)
		this._gumpString := Gump::_gumpString;
		foreach control in (this._addedControls)
			control.Dispose();
			sleepms(2);
		endforeach
		this._addedControls := Gump::_addedControls;
	endfunction

	// Private functions
	function _ConfigureDiag(this)
		this._IndexControl(this.GetLayout());
		this._IndexControl(this.GetTitle());
		foreach page in (this._pages)
			this._IndexControl(page);
			SleepMS(2);
		endforeach
	endfunction

	function _IndexControls(this, controls)
		foreach control in (controls)
			this._IndexControl(control);
			SleepMS(2);
		endforeach
	endfunction

	function _IndexControl(this, control)
		if (!(control is @Control))
			return InvalidParameterException("Control  must be of type 'Control'.");
		endif

		if (!this._createdControls.countByType[control.GetType()])
			this._createdControls.countByType[control.GetType()] := 0;
		endif

		this._createdControls.countByType[control.GetType()] += 1;

		var idOffset := this._GetIdOffsetByControlType(control.GetType());
		var controlId := idOffset + this._createdControls.countByType[control.GetType()];
		control.SetId(controlId);
		this._createdControls.orderById.append(controlId);
		this._createdControls.positionById[controlId] := control;

		if (control is @Container)
			this._IndexControls(control.GetControls());
		endif
	endfunction

	function _DrawDiag(this)
		this.SetGumpString(GFCreateGump(this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight()));
		
		foreach id in (this._createdControls.orderById)
			var control := this._createdControls.positionById[id];
			control.Draw(this);
			SleepMS(2);
		endforeach
	endfunction
	
	function _UpdateInputValues(this)
		foreach control in (this._createdControls.positionById)
			control.SetValue(GFExtractData(this.GetGumpString(), control.GetId()));
		endforeach
	endfunction

	function _CreateHelpButton(this)
		if (this.GetOptions().contains(HIDE_TITLE))
			return;
		endif
		
		var buttonHelp := Button("Help", this.GetTitle().GetWidth() - 30, 36, 0x5689, 0x568A);
		buttonHelp.AddAction(this.helpAction);

		this.GetTitle().AddControl(buttonHelp);
	endfunction

	function _GetIdOffsetByControlType(this, controlType)
		case (controlType) // Feito com valores fixos pra poder ter um controle melhor da resposta no input[0]
			GumpControl::Text: return 1000;
			GumpControl::TextCropped: return 2000;
			GumpControl::ImageGump: return 3000;
			GumpControl::ImageGumpTiled: return 4000;
			GumpControl::ImageTiled: return 5000;
			GumpControl::Button: return 6000;
			GumpControl::CheckBox: return 7000;
			GumpControl::RadioButton: return 8000;
			GumpControl::TextBox: return 9000;
			GumpControl::RadioGroup: return 10000;

			default: return 0;
		endcase
	endfunction
endclass
