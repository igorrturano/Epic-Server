include ":gumps:epicGumps";

class Control()
	var _gumpId := 0;
	var _type := GUMP_CONTROL::NONE; // enum GUMP_CONTROL
	var _name := "";
	var _tooltip; // Tooltip (Control)

	var _x := 0;
	var _y := 0;

	function Control(this, type, name, x, y, gumpId := 0)
		this._gumpId := gumpId;
		this._type := type;
		this._name := name;

		this._x := x;
		this._y := y;
	endfunction

	function GetGumpId(this)
		return this._gumpId;
	endfunction

	function GetType(this)
		return this._type;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetTooltip(this)
		return this._tooltip;
	endfunction

	function GetX(this)
		return this._x;
	endfunction

	function GetY(this)
		return this._y;
	endfunction

	function SetGumpId(this, gumpId)
		this._gumpId := gumpId;
	endfunction

	function SetType(this, type)
		this._type := type;
	endfunction

	function _SetName(this, name)
		this._name := name;
	endfunction

	function SetTooltip(this, tooltip)
		this._tooltip := tooltip;
	endfunction

	function SetX(this, x)
		this._x := x;
	endfunction

	function SetY(this, y)
		this._y := y;
	endfunction

	function SelfCreate(this, byRef gump)
		if (this.GetTooltip())
			this._tooltip.SelfCreate(gump);
		endif
	endfunction

	function Equals(this, byRef other)
		return this._gumpId == other.GetGumpId()
			&& this._type == other.GetType()
			&& this._name == other.GetName();
	endfunction

	function ToString(this)
		return "Control(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", tooltip=" + this.GetTooltip() ? "true" : "false"
			+ ")";
	endfunction
endclass

enum class GUMP_CONTROL
	NONE := "none",

	TEXT := "text",
	TEXTCROPPED := "textcropped",
	IMAGEGUMP := "imagegump",
	IMAGEGUMPTILED := "imagegumptiled",
	IMAGETILED := "imagetiled",
	TOOLTIP := "tooltip",

	BUTTON := "button",
	RADIOGROUP := "radiogroup",
	RADIOBUTTON := "radiobutton",
	CHECKBOX := "checkbox",

	TEXTBOX := "textbox",

	CONTAINER := "container"
endenum