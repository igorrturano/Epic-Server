include "Control";

class CheckBox(Control)
	var _uncheckedImage := 0x0;
	var _checkedImage := 0x0;
	var _value := CHECKBOX_VALUE::UNCHECKED;

	function CheckBox(this, name, x, y, uncheckedImage, checkedImage, value := CHECKBOX_VALUE::UNCHECKED)
		Super(GUMP_CONTROL::CHECKBOX, name, x, y);
		this._uncheckedImage := uncheckedImage;
		this._checkedImage := checkedImage;
		this._value := value;
	endfunction

	function GetUncheckedImage(this)
		return this._uncheckedImage;
	endfunction

	function GetCheckedImage(this)
		return this._checkedImage;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetUncheckedImage(this, uncheckedImage)
		this._uncheckedImage := uncheckedImage;
		return this;
	endfunction

	function SetCheckedImage(this, checkedImage)
		this._checkedImage := checkedImage;
		return this;
	endfunction

	function SetValue(this, value)
		this._value := value;
		return this;
	endfunction

	function ToString(this)
		return "CheckBox(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", uncheckedImage=" + CStr(this.GetUncheckedImage())
			+ ", checkedImage=" + CStr(this.GetCheckedImage())
			+ ", value=" + CStr(this.GetValue())
			+ ")";
	endfunction

	function SelfCreate(this, byRef gump)
		GFCheckBox(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetUncheckedImage(), this.GetCheckedImage(), this.GetValue(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction
endclass

enum class CHECKBOX_VALUE
	UNCHECKED,
	CHECKED
endenum

enum class CHECKBOX_IMAGE
	DEFAULT_UNCHECKED := 0x938,
	DEFAULT_CHECKED := 0x939,

	BLUE_PEARL_UNCHECKED := 0x4B9,
	BLUE_PEARL_CHECKED := 0x4BA,

	ROUND_UNCHECKED := 0xD0,
	ROUND_CHECKED := 0xD1,
	SQUARE_UNCHECKED := 0xD2,
	SQUARE_CHECKED := 0xD3,

	GREY_X_UNCHECKED := 0x47F,
	GREY_X_CHECKED := 0x47E,
	GREY_V_UNCHECKED := 0x482,
	GREY_V_CHECKED := 0x481,

	CANCEL_UNCHECKED := 0x4C2,
	CANCEL_CHECKED := 0x4C3,
	CONTINUE_UNCHECKED := 0x4D0,
	CONTINUE_CHECKED := 0x4D1,
	PREVIOUS_UNCHECKED := 0x4D3,
	PREVIOUS_CHECKED := 0x4D4,


endenum