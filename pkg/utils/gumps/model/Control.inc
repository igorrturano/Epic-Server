class Control()
	var _id;
	var _type;
	var _name;
	var _value;

	var _design;

	function Control(this, type, name, value, design)
		this._id := 0;
		this._type := type;
		this._name := name;
		this._value := value;
		this._design := design;
	endfunction

	function GetId(this)
		return this._id;
	endfunction

	function GetType(this)
		return this._type;
	endfunction

	function _GetName(this)
		return this._name;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function GetDesign(this)
		return this._design;
	endfunction

	function SetId(this, id)
		this._id := id;
	endfunction

	function SetType(this, type)
		this._type := type;
	endfunction

	function _SetName(this, name)
		this._name := name;
	endfunction

	function SetValue(this, value)
		this._value := value;
	endfunction

	function SetDesign(this, design)
		this._design := design;
	endfunction

	function ToString(this)
		return $"Control(id={this._id}, type={this._type}, name={this._name}, value={this._value}, design={this._design})";
	endfunction
endclass

enum GUMP_CONTROL_TYPE
	GUMP_CONTROL_TEXT := "text",
	GUMP_CONTROL_TOOLTIP := "tooltip",

	GUMP_CONTROL_BUTTON := "button",
	GUMP_CONTROL_IMAGE := "image",
	GUMP_CONTROL_RADIOBUTTON := "radiobutton",
	GUMP_CONTROL_CHECKBOX := "checkbox"
endenum