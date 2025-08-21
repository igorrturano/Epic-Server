include "Control";

class RadioButton(Control)
	var _unpressedImage;
	var _pressedImage;
	var _status;

	function RadioButton(this, name, x, y, unpressedImage, pressedImage, status := GUMP_CONTROL_RADIOBUTTON_STATUS::UNPRESSED)
		Super(GUMP_CONTROL::RADIOBUTTON, name, x, y);
		this._unpressedImage := unpressedImage;
		this._pressedImage := pressedImage;
		this._status := status;
	endfunction

	function GetUnpressedImage(this)
		return this._unpressedImage;
	endfunction

	function GetPressedImage(this)
		return this._pressedImage;
	endfunction

	function GetStatus(this)
		return this._status;
	endfunction

	function SetUnpressedImage(this, unpressedImage)
		this._unpressedImage := unpressedImage;
	endfunction

	function SetPressedImage(this, pressedImage)
		this._pressedImage := pressedImage;
	endfunction

	function SetStatus(this, status)
		this._status := status;
	endfunction

	function ToString(this)
		return "RadioButton(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", unpressedImage=" + CStr(this.GetUnpressedImage())
			+ ", pressedImage=" + CStr(this.GetPressedImage())
			+ ", status=" + CStr(this.GetStatus())
			+ ")";
	endfunction

	function SelfCreate(this, byRef gump)
		GFRadioButton(gump, this.GetX(), this.GetY(), this.GetUnpressedImage(), this.GetPressedImage(), this.GetStatus(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction
endclass

enum class GUMP_CONTROL_RADIOBUTTON_STATUS
	UNPRESSED,
	PRESSED
endenum

enum class GUMP_CONTROL_RADIOBUTTON_IMAGE
	DEFAULT_UNPRESSED := 2362,
	DEFAULT_PRESSED := 2360
endenum