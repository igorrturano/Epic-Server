include ":gumps:epicGumps";

class Control()
	var _gumpId;
	var _type;
	var _name;

	var _x;
	var _y;

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

	function _GetName(this)
		return this._name;
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

	function SetX(this, x)
		this._x := x;
	endfunction

	function SetY(this, y)
		this._y := y;
	endfunction

	function SelfCreate(this, byRef gump) endfunction

	function ToString(this)
		return "Control(name=" + this._GetName()
			+ ", gumpId=" + this.GetGumpId()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ")";
	endfunction
endclass

enum class GUMP_CONTROL
	TEXT := "text",
	TEXTCROPPED := "textcropped",
	IMAGEGUMP := "imagegump",
	IMAGETILE := "imagetile",
	TOOLTIP := "tooltip",

	BUTTON := "button",
	RADIOBUTTON := "radiobutton",
	CHECKBOX := "checkbox"
endenum