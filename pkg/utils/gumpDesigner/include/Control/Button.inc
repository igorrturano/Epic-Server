include "Control";

class Button(Control)
	var _unpressedImage := 0x0;
	var _pressedImage := 0x0;
	var _buttonType := BUTTON_TYPE::CLOSE;

	var _action := array{}; // Função lambda a ser executada no clique

	function Button(this, name, x, y, unpressedImage := BUTTON_IMAGE::CLOSE_UNPRESSED, pressedImage := BUTTON_IMAGE::CLOSE_PRESSED, buttonType := BUTTON_TYPE::CLOSE)
		Super(GUMP_CONTROL::BUTTON, name, x, y);
		this._unpressedImage := unpressedImage;
		this._pressedImage := pressedImage;
		this._buttonType := buttonType;
	endfunction

	function GetUnpressedImage(this)
		return this._unpressedImage;
	endfunction

	function GetPressedImage(this)
		return this._pressedImage;
	endfunction

	function GetButtonType(this)
		return this._buttonType;
	endfunction

	function GetAction(this)
		return this._action;
	endfunction

	function SetImage(this, image)
		this._image := image;
	endfunction

	function SetPressedImage(this, pressedImage)
		this._pressedImage := pressedImage;
	endfunction

	function SetButtonType(this, buttonType)
		this._buttonType := buttonType;
	endfunction

	function AddAction(this, action)
		this._action.append(action);
	endfunction

	function SelfCreate(this, byRef gump)
		GFAddButton(gump, this.GetX(), this.GetY(), this.GetUnpressedImage(), this.GetPressedImage(), this.GetButtonType(), this.GetGumpId());
	endfunction

	function ToString(this)
		return "Button(gumpId=" + this.GetGumpId()
			+ ", name=" + this._GetName()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", unpressedImage=" + this.GetUnpressedImage()
			+ ", pressedImage=" + this.GetPressedImage()
			+ ", action=" + this.GetAction()
			+ ")";
	endfunction
endclass

enum class BUTTON_TYPE
	PAGE := GF_PAGE_BTN,
	CLOSE := GF_CLOSE_BTN,
	DUMMY := GF_DUMMY_BTN
endenum

enum class BUTTON_IMAGE
	CLOSE_UNPRESSED := 2103,
	CLOSE_PRESSED := 2104
endenum