include "Control";

class CheckBox(Control)
	var _uncheckedImage;
	var _checkedImage;
	var _status;

	function CheckBox(this, name, x, y, uncheckedImage, checkedImage, status := GUMP_CONTROL_CHECKBOX_STATUS::UNCHECKED)
		Super(GUMP_CONTROL::CHECKBOX, name, x, y);
		this._uncheckedImage := uncheckedImage;
		this._checkedImage := checkedImage;
		this._status := status;
	endfunction

	function GetUncheckedImage(this)
		return this._uncheckedImage;
	endfunction

	function GetCheckedImage(this)
		return this._checkedImage;
	endfunction

	function GetStatus(this)
		return this._status;
	endfunction

	function SetUncheckedImage(this, uncheckedImage)
		this._uncheckedImage := uncheckedImage;
	endfunction

	function SetCheckedImage(this, checkedImage)
		this._checkedImage := checkedImage;
	endfunction

	function SetStatus(this, status)
		this._status := status;
	endfunction

	function ToString(this)
		return "CheckBox(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", uncheckedImage=" + CStr(this.GetUncheckedImage())
			+ ", checkedImage=" + CStr(this.GetCheckedImage())
			+ ", status=" + CStr(this.GetStatus())
			+ ")";
	endfunction

	function SelfCreate(this, byRef gump)
		GFCheckBox(gump, this.GetX(), this.GetY(), this.GetUncheckedImage(), this.GetCheckedImage(), this.GetStatus(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction
endclass

enum class GUMP_CONTROL_CHECKBOX_STATUS
	UNCHECKED,
	CHECKED
endenum

enum class GUMP_CONTROL_CHECKBOX_IMAGE
	DEFAULT_UNCHECKED := 2362,
	DEFAULT_CHECKED := 2360
endenum