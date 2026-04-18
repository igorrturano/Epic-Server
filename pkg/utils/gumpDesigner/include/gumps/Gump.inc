include "IGump";

include ":gumps:autoClose";

include ":gumpDesigner:layouts/_importall";
include ":gumpDesigner:factories/GumpFactory";
include ":gumpDesigner:events/GumpEvent";
include ":gumpDesigner:exceptions/NoEventHandledException";
include ":gumpDesigner:exceptions/InvalidOptionException";

include ":message:model/Message";

class Gump(Container, IGump)
	var _pid;
	var _gumpString;

	var _layout; // Objeto herdado de GumpLayout
	var _titleName := "Default Title";
	var _title; // Objeto TitleLayout
	var _pages := array{}; // Array das páginas, cada página é o Objeto BodyLayout
	var _currentPage := 1;

	var _options := array{};
	var _createdControls := struct{
			orderById := array{},
			countByType := dictionary{},
			positionById := dictionary{}
		};
	var helpAction;

	function Gump(this, parent, title, width, height, options := array{})
		this.SetParent(parent);
		this.SetWidth(width);
		this.SetHeight(height);
		this.SetTitleName(title);

		this._options := options;
		this._createdControls := Gump::_createdControls;
		this.helpAction := @() {
			Message("Esta ajuda ainda não foi implementada.").SetColor(Color(SYSTEM_COLOR::ALERT)).SendSys(this.GetParent());
		};
	endfunction

	function _GetPid(this)
		return this._pid;
	endfunction

	function SetPid(this, pid)
		this._pid := pid;

		return this;
	endfunction

	function GetGumpString(this)
		return this._gumpString;
	endfunction

	function SetGumpString(this, gumpStr)
		this._gumpString := gumpStr;

		return this;
	endfunction

	// // Override
	// function Resize(this, width, height)
	// 	Super::Resize(this, width, height);
		
	// 	if (this.GetLayout())
	// 		this.GetLayout().Resize(width, height);
	// 	endif

	// 	return this;
	// endfunction

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

	function GetTitleName(this)
		return this._titleName;
	endfunction

	function SetTitleName(this, titleName)
		this._titleName := CStr(titleName);

		return this;
	endfunction

	function GetTitle(this)
		return this._title;
	endfunction

	function SetTitle(this, title)
		if (!(title is @TitleLayout))
			return InvalidParameterException("Title must be of type TitleLayout.");
		endif

		this._title := title;

		return this;
	endfunction

	function GetPages(this)
		return this._pages;
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

		if (!(page is @BodyLayout))
			return InvalidParameterException("Page must be of type BodyLayout.");
		endif

		this._pages[pageNumber] := page;

		return this;
	endfunction

	function AddPage(this, page)
		if (!(page is @BodyLayout))
			return InvalidParameterException("Page must be of type BodyLayout.");
		endif

		if (!this._pages)
			this._pages := Gump::_pages;
		endif

		if (this._pages.size() >= 10)
			return InvalidParameterException("A gump cannot have more than 10 pages.");
		endif

		this._pages.append(page);

		return this;
	endfunction

	function GetCurrentPage(this)
		return this._currentPage;
	endfunction

	function SetCurrentPage(this, pageNumber)
		if (pageNumber < 1 || pageNumber > this._pages.size())
			return InvalidParameterException("Page number out of range.");
		endif

		this._currentPage := pageNumber;

		return this;
	endfunction

	function AddControl(this, control)
		// if (!(control is @Control))
		// 	return InvalidParameterException("Control must be of type 'Control'.");
		// endif

		// var page := this.GetPage(this.GetCurrentPage());
		// control.SetParent(page);
		// page.AddControl(control);

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

		this._ConfigureDiag();
		this._DrawDiag();

		var result := GFSendGump(this._parent, this.GetGumpString());
		
		var eventHandled;
		if (!result[0])
			return NoEventHandledException("No event was handled for this gump.");
		else
			eventHandled := GumpEvent(this.GetControls().position[result[0]]);
		endif

		this._UpdateInputValues();
		eventHandled.GetControl().ExecuteActions();
		this.Dispose();

		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction

	function Dispose(this)
		this._gumpString := Gump::_gumpString;
		this._createdControls := Gump::_createdControls;
	endfunction

	// Private functions
	function _ConfigureDiag(this)
		this._AdjustLayout();
		this._AdjustTitle();
		// this._AdjustPage();
		
		this._IndexControl(this.GetCurrentPage());
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

	function _AdjustLayout(this)
		if (!this.GetLayout())
			this.SetLayout(GoldenGumpLayout());
		endif

		this.GetLayout().Resize(this.GetWidth(), this.GetHeight());
		this.GetLayout().PositionControls();
	endfunction

	function _AdjustTitle(this)
		if (GumpOption::HideTitle in this.GetOptions())
			return;
		endif

		if (!this.GetTitle())
			this.SetTitle(this.GetLayout().GetTitleLayout());
		endif

		this.GetTitle().SetText(this.GetTitleName());
		this.GetTitle().SetHelpButton(this._CreateHelpButton());
	endfunction

	function _AdjustPage(this)
		if (!this.GetCurrentPage())
			return;
		endif

		// TODO
	endfunction

	function _DrawDiag(this)
		this.SetGumpString(GFCreateGump(this.GetX(), this.GetY(), this.GetWidth(), this.GetHeight()));

		this.GetLayout().Draw(this);
		if (this.GetTitle())
			this.GetTitle().Draw(this);
		endif
		// this.GetPage(this.GetCurrentPage()).Draw(this);
		// foreach control in (this.GetControls())
		// 	control.Draw(this);
		// 	SleepMS(2);
		// endforeach
		
		// foreach id in (this._createdControls.orderById)
		// 	var control := this._createdControls.positionById[id];
		// 	control.Draw(this);
		// 	SleepMS(2);
		// endforeach
	endfunction
	
	function _UpdateInputValues(this)
		foreach control in (this._createdControls.positionById)
			control.SetValue(GFExtractData(this.GetGumpString(), control.GetId()));
		endforeach
	endfunction

	function _CreateHelpButton(this)
		var buttonHelp := Button("Help", this.GetTitle().GetWidth() - 30, 36, 0x5689, 0x568A);
		buttonHelp.AddAction(this.helpAction);
		return buttonHelp;
	endfunction

	function _GetIdOffsetByControlType(this, controlType)
		case (controlType) // Feito com valores fixos pra poder ter um controle melhor da resposta no input[0]
			GumpControl::Text: return 1000;
			GumpControl::TextCropped: return 2000;
			GumpControl::ImageGump: return 3000;
			GumpControl::ImageTile: return 4000;
			GumpControl::Button: return 5000;
			GumpControl::CheckBox: return 6000;
			GumpControl::RadioButton: return 7000;
			GumpControl::TextBox: return 8000;
			GumpControl::RadioGroup: return 9000;

			default: return 0;
		endcase
	endfunction
endclass

enum class GumpOption
	HideTitle := "hide_title",
	HideBorder := "hide_border",
	NoMovable := "no_movable",
	NoClosable := "no_closable",
endenum