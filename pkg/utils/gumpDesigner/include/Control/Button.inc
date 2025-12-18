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

	function SetUnpressedImage(this, unpressedImage)
		this._unpressedImage := unpressedImage;
		return this;
	endfunction

	function SetPressedImage(this, pressedImage)
		this._pressedImage := pressedImage;
		return this;
	endfunction

	function SetButtonType(this, buttonType)
		this._buttonType := buttonType;
		return this;
	endfunction

	function AddAction(this, action)
		this._action.append(action);
		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		GFAddButton(gump, this.GetX() + this.GetXOffset(), this.GetY() + this.GetYOffset(), this.GetUnpressedImage(), this.GetPressedImage(), this.GetButtonType(), this.GetGumpId());
		Super::SelfCreate(this, gump);
	endfunction

	function ToString(this)
		return "Button(gumpId=" + this.GetGumpId()
			+ ", name=" + this.GetName()
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
	CLOSE_PRESSED := 2104,

	SKILL1_UNPRESSED := 0x4D8,
	SKILL1_PRESSED := 0x4D7,
	SKILL2_UNPRESSED := 0x4DB,
	SKILL2_PRESSED := 0x4DA,
	SKILL3_UNPRESSED := 0x4DE,
	SKILL3_PRESSED := 0x4DD,

	MODERN_EYE_UNPRESSED := 0x5FC,
	MODERN_EYE_PRESSED := 0x5FB,
	MODERN_EDIT_UNPRESSED := 0x5FE,
	MODERN_EDIT_PRESSED := 0x5FD,
	MODERN_PREVIOUS_UNPRESSED := 0x60A,
	MODERN_PREVIOUS_PRESSED := 0x609,
	MODERN_DOUBLEPREVIOUS_UNPRESSED := 0x606,
	MODERN_DOUBLEPREVIOUS_PRESSED := 0x605,
	MODERN_NEXT_UNPRESSED := 0x609,
	MODERN_NEXT_PRESSED := 0x608,
	MODERN_DOUBLENEXT_UNPRESSED := 0x604,
	MODERN_DOUBLENEXT_PRESSED := 0x603,
	MODERN_X_UNPRESSED := 0x600,
	MODERN_X_PRESSED := 0x5FF,
	MODERN_V_UNPRESSED := 0x602,
	MODERN_V_PRESSED := 0x601,
endenum