include "Control";

class CheckBox(Control)
	var _uncheckedImage;
	var _checkedImage;
	var _value;

	function CheckBox(this, name, x, y, uncheckedImage, checkedImage, value := GUMP_CONTROL_CHECKBOX_VALUE::UNCHECKED)
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
			+ ", name=" + this._GetName()
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

enum class GUMP_CONTROL_CHECKBOX_VALUE
	UNCHECKED,
	CHECKED
endenum

enum class GUMP_CONTROL_CHECKBOX_IMAGE
	DEFAULT_UNCHECKED := 2362,
	DEFAULT_CHECKED := 2360
endenum