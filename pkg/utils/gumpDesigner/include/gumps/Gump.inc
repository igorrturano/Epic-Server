include "IGump";
include ":gumps:epicGumps";
include ":gumps:autoClose";
include ":gumpDesigner:controls/Control";
include ":gumpDesigner:controls/Button";
include ":gumpDesigner:events/EventHandler";

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
	var _controls := array{}; // Array de Controls
	var _controlsIdCount := dictionary{}; // Dicionário para contar a quantidade de controles de cada tipo, usado para gerar IDs únicos

	function Gump(this, parent, title, width, height, options := array{})
		this._parent := parent;

		this._title := title;
		this._width := width;
		this._height := height;

		this._options := options.size() < 1 ? array{BORDER_GOLDEN} : options;
		this._customization := array{};
		this._controls := array{};
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

	function GetGumpResult(this)
		return this._gumpResult;
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

	function GetControls(this)
		return this._controls;
	endfunction

	function SetParent(this, parent)
		this._parent := parent;
		return this;
	endfunction

	function SetTitle(this, title)
		this._title := title;
		return this;
	endfunction

	function SetWidth(this, width)
		this._width := width;
		return this;
	endfunction

	function SetHeight(this, height)
		this._height := height;
		return this;
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

	function AddCustomization(this, byRef customFunction)
		this._customization.append(customFunction);
		return this;
	endfunction

	// function AddControl(this, byRef control)
		// var id := 1;

		// if (!this._controls[control.GetType()])
		// 	this._controls[control.GetType()] := array{};
		// endif
		
		// if (control.GetType() == GUMP_CONTROL::TEXT)
		// 	id := GUMP_ID_REFERENCE::TEXT + id + this._controls[GUMP_CONTROL::TEXT].size();
		// elseif (control.GetType() == GUMP_CONTROL::TEXTCROPPED)
		// 	id := GUMP_ID_REFERENCE::TEXTCROPPED + id + this._controls[GUMP_CONTROL::TEXTCROPPED].size();
		// elseif (control.GetType() == GUMP_CONTROL::IMAGEGUMP)
		// 	id := GUMP_ID_REFERENCE::IMAGEGUMP + id + this._controls[GUMP_CONTROL::IMAGEGUMP].size();
		// elseif (control.GetType() == GUMP_CONTROL::IMAGEGUMPTILED)
		// 	id := GUMP_ID_REFERENCE::IMAGEGUMPTILED + id + this._controls[GUMP_CONTROL::IMAGEGUMPTILED].size();
		// elseif (control.GetType() == GUMP_CONTROL::IMAGETILED)
		// 	id := GUMP_ID_REFERENCE::IMAGETILED + id + this._controls[GUMP_CONTROL::IMAGETILED].size();
		// elseif (control.GetType() == GUMP_CONTROL::BUTTON)
		// 	id := GUMP_ID_REFERENCE::BUTTON + id + this._controls[GUMP_CONTROL::BUTTON].size();
		// elseif (control.GetType() == GUMP_CONTROL::CHECKBOX)
		// 	id := GUMP_ID_REFERENCE::CHECKBOX + id + this._controls[GUMP_CONTROL::CHECKBOX].size();
		// elseif (control.GetType() == GUMP_CONTROL::RADIOBUTTON)
		// 	id := GUMP_ID_REFERENCE::RADIOBUTTON + id + this._controls[GUMP_CONTROL::RADIOBUTTON].size();
		// elseif (control.GetType() == GUMP_CONTROL::TEXTBOX)
		// 	id := GUMP_ID_REFERENCE::TEXTBOX + id + this._controls[GUMP_CONTROL::TEXTBOX].size();
		// elseif (control.GetType() == GUMP_CONTROL::CONTAINER) // Container precisa pegar os controles internos e adicionar aqui no gump
		// 	foreach containerControl in (control.GetControls())
		// 		this.AddControl(containerControl); // Adiciona os controles encapsulados ao gump
		// 	endforeach
		// elseif (control.GetType() == GUMP_CONTROL::CONTAINERGRIDLAYOUT) // ContainerGridLayout é ainda pior, precisa pegar os controles internos de cada célula
		// 	foreach row in (control.GetControls()) // Cada linha do grid é um array de containers
		// 		foreach container in (row) // Cada container pode ter múltiplos controles
		// 			foreach containerControl in (container.GetControls())
		// 				this.AddControl(containerControl); // Adiciona os controles encapsulados ao gump
		// 			endforeach
		// 		endforeach
		// 	endforeach
		// elseif (control.GetType() == GUMP_CONTROL::NONE)
		// 	return error { errortext := "Control type is NONE!" };
		// else
		// 	return error { errortext := "Unknown control type!" };
		// endif

		// control.SetGumpId(id);

		// this._controls[control.GetType()].append(control);

	// 	return this;
	// endfunction

	function AddControl(this, byRef control)
		if (!this._controlsIdCount[control.GetType()])
			this._controlsIdCount[control.GetType()] := 0;
			this._controlsIdCount[control.GetType()] += _GetIdOffsetByControlType(control.GetType());
		endif

		this._controlsIdCount[control.GetType()]++;
		control.SetGumpId(this._controlsIdCount[control.GetType()]);

		this._controls.append(control);

		return this;
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

	function Show(this)
		var eventHandled, gumpResult;
		
		this._gump := GFECreateGump(this._title, this._width, this._height, this._options);
		this._CreateHelper();

		this._CreateControls();
		this._CreateCustoms();

		this._pid := GetPid();
		this._gumpResult := GFSendGump(this._parent, this._gump);

		

		this._UpdateInputValues();
		eventHandled.GetControl().ExecuteActions();

		return eventHandled;
	endfunction

	function Close(this)
		return GFCloseGump(this._parent, this._pid);
	endfunction

	function Dispose(this)
		this._gump := 0;
		this._gumpResult := 0;
		foreach controlGroup in (this._controls)
			foreach control in (controlGroup)
				control.Dispose();
			endforeach
		endforeach
		this._controls := dictionary{};
	endfunction

	// Private functions
	function _GetIdOffsetByControlType(byRef control)
		case (control.GetType()) // Feito com valores fixos pra poder ter um controle melhor
			GUMP_CONTROL::TEXT: return 0;
			GUMP_CONTROL::TEXTCROPPED: return 1000;
			GUMP_CONTROL::IMAGEGUMP: return 2000;
			GUMP_CONTROL::IMAGEGUMPTILED: return 3000;
			GUMP_CONTROL::IMAGETILED: return 4000;
			GUMP_CONTROL::BUTTON: return 5000;
			GUMP_CONTROL::CHECKBOX: return 6000;
			GUMP_CONTROL::RADIOBUTTON: return 7000;
			GUMP_CONTROL::TEXTBOX: return 8000;

			default: return 0; // Controles do tipo CONTAINER e CONTAINERGRIDLAYOUT não precisam de ID, pois não geram eventos diretamente
		endcase
	endfunction

	function _CreateControls(this)
		// Containers precisam ser criados antes, para evitar sobreposição
		foreach control in (this._controls[GUMP_CONTROL::CONTAINER])
			control.SelfCreate(this._gump);
		endforeach; 

		foreach control in (this._controls[GUMP_CONTROL::CONTAINERGRIDLAYOUT])
			control.SelfCreate(this._gump);
		endforeach
		
		foreach controlGroup in (this._controls)
			if (_controlGroup_iter == GUMP_CONTROL::CONTAINER ||
				_controlGroup_iter == GUMP_CONTROL::CONTAINERGRIDLAYOUT
				)
				continue; // Containers foram criados antes
			endif

			foreach control in (controlGroup)
				control.SelfCreate(this._gump);
			endforeach
		endforeach
	endfunction

	function _CreateCustoms(this)
		foreach custom in (this._customization)
			custom(this._gump);
		endforeach
	endfunction

	function _UpdateInputValues(this)
		foreach control in (this._controls[GUMP_CONTROL::TEXTBOX])

			control.SetValue(GFExtractData(this._gumpResult, control.GetGumpId()));
		endforeach

		foreach control in (this._controls[GUMP_CONTROL::CHECKBOX])
			control.SetValue(CInt(GFExtractData(this._gumpResult, control.GetGumpId())));
		endforeach

		foreach control in (this._controls[GUMP_CONTROL::RADIOBUTTON])
			control.SetValue(CInt(GFExtractData(this._gumpResult, control.GetGumpId())));
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

// enum class GUMP_ID_REFERENCE
// 	LIMIT := 1000,
	
// 	TEXT := 0,
// 	TEXTCROPPED := 1000,
// 	IMAGEGUMP := 2000,
// 	IMAGEGUMPTILED := 3000,
// 	IMAGETILED := 4000,
// 	BUTTON := 5000,
// 	CHECKBOX := 6000,
// 	RADIOBUTTON := 7000,
// 	TEXTBOX := 8000
// endenum