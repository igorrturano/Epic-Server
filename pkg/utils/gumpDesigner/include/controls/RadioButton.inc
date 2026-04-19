include "Control";

class RadioButton(Control)
	var _unpressedImage := 0x0;
	var _pressedImage := 0x0;
	var _value := RADIOBUTTON_VALUE::UNPRESSED;

	function RadioButton(this, name, x, y, unpressedImage, pressedImage, value := RADIOBUTTON_VALUE::UNPRESSED)
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
		return this;
	endfunction

	function SetPressedImage(this, pressedImage)
		this._pressedImage := pressedImage;
		return this;
	endfunction

	function SetValue(this, value)
		this._value := value;
		return this;
	endfunction

	function ToString(this)
		return "(RadioButton) " + Super::ToString(this)
			+ ", unpressedImage=" + this.GetUnpressedImage()
			+ ", pressedImage=" + this.GetPressedImage()
			+ ", value=" + this.GetValue();
	endfunction

	function HowToDraw(this, byRef gump)
		GFRadioButton(gump.GetGumpString(), this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetUnpressedImage(), this.GetPressedImage(), this.GetValue(), this.GetGumpId());
		// Super::Draw(this, gump);
	endfunction
endclass

enum class RADIOBUTTON_VALUE
	UNPRESSED,
	PRESSED
endenum

enum class RADIOBUTTON_IMAGE
	DEFAULT_UNPRESSED := 2362,
	DEFAULT_PRESSED := 2360
endenum