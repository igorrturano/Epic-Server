include ":gumps:epicGumps";
include "IControl";

class Control(IControl)
	var _id := 0; // ID que é retornado quando o gump fecha
	var _type := GumpControl::None; // enum GumpControl
	var _name := ""; // Nome do controle, pra identificar na chamada de eventos ou no script
	var _tooltip; // Tooltip (Control)

	var _x := 0;
	var _y := 0;
	var _xOffset := 0;
	var _yOffset := 0;

	function Control(this, type, name, x, y)
		this._type := type;
		this._name := name;

		this._x := x;
		this._y := y;
		this._xOffset := 0;
		this._yOffset := 0;
	endfunction

	function GetId(this)
		return this._id;
	endfunction

	function SetId(this, id)
		this._id := id;
		return this;
	endfunction

	function GetType(this)
		return this._type;
	endfunction

	function SetType(this, type)
		this._type := type;
		return this;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function _SetName(this, name)
		this._name := name;
		return this;
	endfunction

	function GetTooltip(this)
		return this._tooltip;
	endfunction

	function SetTooltip(this, tooltip)
		this._tooltip := tooltip;
		return this;
	endfunction

	function GetX(this)
		return this._x;
	endfunction

	function SetX(this, x)
		this._x := x;
		return this;
	endfunction

	function GetY(this)
		return this._y;
	endfunction

	function SetY(this, y)
		this._y := y;
		return this;
	endfunction

	function GetXOffset(this)
		return this._xOffset;
	endfunction

	function SetXOffset(this, xOffset)
		this._xOffset := xOffset;
		return this;
	endfunction

	function GetYOffset(this)
		return this._yOffset;
	endfunction

	function SetYOffset(this, yOffset)
		this._yOffset := yOffset;
		return this;
	endfunction

	function Draw(this, byRef gump)
		print(gump.GetGumpString());
		print(gump._gumpString);
		print("--------------");
		
		if (this.GetTooltip())
			this.GetTooltip().Draw(gump);
		endif
	endfunction

	function Equals(this, byRef other)
		return this._id == other.GetId()
			&& this._type == other.GetType()
			&& this._name == other.GetName();
	endfunction

	function Dispose(this)
		// Implementa descarte no objeto filho, se necessário
	endfunction

	function ToString(this)
		return "(Control) id=" + this.GetId()
			+ ", name=" + this.GetName()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", xOffset=" + this.GetXOffset()
			+ ", yOffset=" + this.GetYOffset();
	endfunction
endclass

enum class GumpControl
	None := "none",
	Text := "text",
	TextCropped := "textcropped",
	ImageGump := "imagegump",
	ImageGumpTiled := "imagegumptiled",
	ImageTiled := "imagetiled",
	Tooltip := "tooltip",
	Button := "button",
	RadioGroup := "radiogroup",
	RadioButton := "radiobutton",
	CheckBox := "checkbox",
	TextBox := "textbox",
	Container := "container",
	ContainerGridLayout := "containergridlayout"
endenum