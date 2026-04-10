include "IGump";
include ":gumps:epicGumps";
include ":gumps:autoClose";
include ":gumpDesigner:controls/Control";
include ":gumpDesigner:controls/Button";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:events/GumpEvent";
include ":gumpDesigner:events/exceptions/NoEventHandledException";

class Gump(IGump)
	var _parent;
	var _pid;
	var _gump;
	var _gumpResult;
	
	var _title := "";
	var _width := 0;
	var _height := 0;

	var _options := array{}; // Array de opções (enum GUMP_OPTION)
	var _customization := array{}; // Array de funções lambda

	var _controls := struct{
			idOrder := array{},
			lastId := dictionary{},
			position := dictionary{}
		};

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;

		this._title := title;
		this._width := width;
		this._height := height;

		this._options := options.size() < 1 ? array{BORDER_GOLDEN} : options;
		this._customization := array{};
		this._controls := struct{
			idOrder := array{},
			lastId := dictionary{},
			position := dictionary{}
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

	function GetGumpResult(this)
		return this._gumpResult;
	endfunction

	function GetTitle(this)
		return this._title;
	endfunction

	function SetTitle(this, title)
		this._title := title;
		return this;
	endfunction

	function GetWidth(this)
		return this._width;
	endfunction

	function SetWidth(this, width)
		this._width := width;
		return this;
	endfunction

	function GetHeight(this)
		return this._height;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
	endfunction

	function GetOptions(this)
		return this._options;
	endfunction

	function SetOptions(this, options)
		this._options := options;
		return this;
	endfunction

	function AddOption(this, option)
		if (option in this._options) return error { errortext := "Option already set!" }; endif

		this._options.append(option);
		return this;
	endfunction

	function GetControls(this)
		return this._controls;
	endfunction

	function AddControl(this, byRef control)
		if (!this._controls.lastId.exists(control.GetType()))
			this._controls.lastId[control.GetType()] := _GetIdOffsetByControlType(control.GetType());
		endif

		this._controls.lastId[control.GetType()]++;
		control.SetId(this._controls.lastId[control.GetType()]);
		this._controls.idOrder.append(control.GetId());	
		this._controls.position[control.GetId()] := control;
		print ("Added control: " + control.ToString() + " with ID: " + control.GetId());
		
		if (control is @Container)
			foreach childControl in (control.GetControls())
				this.AddControl(childControl);
			endforeach
		endif

		return this;
	endfunction

	function AddCustomization(this, byRef customFunction)
		this._customization.append(customFunction);
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
		
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
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
		print ("Event handled for control: " + eventHandled.GetControl().ToString());

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
		buttonHelp.AddAction(@() {
			SendSysMessage(this.GetParent(), "This is a help message for the gump.");
		});

		this.AddControl(buttonHelp);
	endfunction
endclass
