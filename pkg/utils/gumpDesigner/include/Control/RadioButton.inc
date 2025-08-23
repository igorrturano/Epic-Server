include "Control";

class RadioButton(Control)
	var _unpressedImage;
	var _pressedImage;
	var _value;

	function RadioButton(this, name, x, y, unpressedImage, pressedImage, value := GUMP_CONTROL_RADIOBUTTON_VALUE::UNPRESSED)
		Super(GUMP_CONTROL::RADIOBUTTON, name, x, y);
		this._unpressedImage := unpressedImage;
		this._pressedImage := pressedImage;
		this._value := value;
	endfunction

	function GetUnpressedImage(this)
		return this._unpressedImage;
	endfunction

	function GetPressedImage(this)
		return this._pressedImage;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function SetUnpressedImage(this, unpressedImage)
		this._unpressedImage := unpressedImage;
	endfunction

	function SetPressedImage(this, pressedImage)
		this._pressedImage := pressedImage;
	endfunction

	function SetValue(this, value)
		this._value := value;
	endfunction

	function ToString(this)
		return "RadioButton(gumpId=" + this.GetGumpId()
			+ ", name=" + this._GetName()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", unpressedImage=" + CStr(this.GetUnpressedImage())
			+ ", pressedImage=" + CStr(this.GetPressedImage())
			+ ", value=" + CStr(this.GetValue())
			+ ")";
	endfunction

	function SelfCreate(this, byRef gump)
		GFRadioButton(gump, this.GetX(), this.GetY(), this.GetUnpressedImage(), this.GetPressedImage(), this.GetValue(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction
endclass

enum class GUMP_CONTROL_RADIOBUTTON_VALUE
	UNPRESSED,
	PRESSED
endenum

enum class GUMP_CONTROL_RADIOBUTTON_IMAGE
	DEFAULT_UNPRESSED := 2362,
	DEFAULT_PRESSED := 2360
endenum