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
	endfunction

	function SetCheckedImage(this, checkedImage)
		this._checkedImage := checkedImage;
	endfunction

	function SetValue(this, value)
		this._value := value;
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
		GFCheckBox(gump, this.GetX(), this.GetY(), this.GetUncheckedImage(), this.GetCheckedImage(), this.GetValue(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction
endclass

enum class CHECKBOX_VALUE
	UNCHECKED,
	CHECKED
endenum

enum class CHECKBOX_IMAGE
	DEFAULT_UNCHECKED := 2362,
	DEFAULT_CHECKED := 2360,

	BLUE_PEARL_UNCHECKED := 0x4B9,
	BLUE_PEARL_CHECKED := 0x4BA,

	ROUND_UNCHECKED := 0xD0,
	ROUND_CHECKED := 0xD1,
	SQUARE_UNCHECKED := 0xD2,
	SQUARE_CHECKED := 0xD3,

	GREY_X_UNCHECKED := 0x47F,
	GREY_X_CHECKED := 0x47E,
	GREY_Y_UNCHECKED := 0x482,
	GREY_Y_CHECKED := 0x481,

	CONTINUE_UNCHECKED := 0x4BF,
	CONTINUE_CHECKED := 0x4C0,
	CANCEL_UNCHECKED := 0x4C2,
	CANCEL_CHECKED := 0x4C3,

	NEXT_UNCHECKED := 0x4D0,
	NEXT_CHECKED := 0x4D1,
	PREVIOUS_UNCHECKED := 0x4D3,
	PREVIOUS_CHECKED := 0x4D4,

	SKILL1_UNCHECKED := 0x4D8,
	SKILL1_CHECKED := 0x4D7,
	SKILL2_UNCHECKED := 0x4DB,
	SKILL2_CHECKED := 0x4DA,
	SKILL3_UNCHECKED := 0x4DE,
	SKILL3_CHECKED := 0x4DD,

	MODERN_UNCHECKED := 0x4C4E,
	MODERN_CHECKED := 0x4C4F
endenum