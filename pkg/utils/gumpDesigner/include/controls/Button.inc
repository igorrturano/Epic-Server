include "Control";

class Button(Control)
	var unpressedImage := 0x0;
	var pressedImage := 0x0;
	var closableButton := true;

	var actions := array{}; // Função lambda a ser executada no clique

	function Button(this, name, x, y, unpressedImage := BUTTON_IMAGE::CLOSE_UNPRESSED, pressedImage := BUTTON_IMAGE::CLOSE_PRESSED)
		Super(GumpControl::Button, name, x, y);
		this.unpressedImage := unpressedImage;
		this.pressedImage := pressedImage;
		this.closableButton := Button::closableButton;
		this.actions := array{};
	endfunction

	function GetUnpressedImage(this)
		return this.unpressedImage;
	endfunction

	function SetUnpressedImage(this, unpressedImage)
		this.unpressedImage := unpressedImage;

		return this;
	endfunction

	function GetPressedImage(this)
		return this.pressedImage;
	endfunction

	function SetPressedImage(this, pressedImage)
		this.pressedImage := pressedImage;

		return this;
	endfunction

	function SetClosable(this, closable)
		this.closableButton := closable;

		return this;
	endfunction

	function IsClosable(this)
		return this.closableButton == true;
	endfunction
	
	function GetActions(this)
		return this.actions;
	endfunction

	function AddAction(this, action)
		this.actions.append(action);

		return this;
	endfunction

	function ExecuteActions(this)
		foreach action in (this.actions)
			action();
		endforeach
	endfunction

	function HowToDraw(this, byRef gump)
		GFAddButton(
			gump.GetGumpString(),
			this.GetX() + this.GetXOffset(),
			this.GetY() + this.GetYOffset(),
			this.GetUnpressedImage(),
			this.GetPressedImage(),
			this.IsClosable() ? GF_CLOSE_BTN : GF_DUMMY_BTN,
			this.GetId()
		);
	endfunction

	function ToString(this)
		return "(Button) " + Super::ToString(this)
			+ ", unpressedImage=" + this.GetUnpressedImage()
			+ ", pressedImage=" + this.GetPressedImage()
			+ ", buttonType=" + this.GetButtonType()
			+ ", actionsCount=" + this.GetActions().size();
	endfunction
endclass

enum class ButtonType
	NoClose,
	Close
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