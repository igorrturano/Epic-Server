include "Control";

class Button(Control)
	var unpressedImage := 0x0;
	var pressedImage := 0x0;
	var closableButton := true;

	var actions := array{}; // Função lambda a ser executada no clique

	function Button(this, name, x, y, unpressedImage := ButtonImage::CloseUnpressed, pressedImage := ButtonImage::ClosePressed)
		Super(GumpControl::Button, name, x, y);
		this.unpressedImage := unpressedImage;
		this.pressedImage := pressedImage;
		this.closableButton := Button::closableButton;
		this.actions := Button::actions;
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

	function Dispose(this)
		Super::Dispose(this);
		this.actions := Button::actions;
	endfunction

	function ToString(this)
		return "(Button) " + Super::ToString(this)
			+ ", unpressedImage=" + this.GetUnpressedImage()
			+ ", pressedImage=" + this.GetPressedImage()
			+ ", closableButton=" + this.IsClosable()
			+ ", actionsCount=" + this.GetActions().size();
	endfunction
endclass

enum class ButtonType
	NoClose,
	Close
endenum

enum class ButtonImage
	CloseUnpressed := 2103,
	ClosePressed := 2104,

	Skill1Unpressed := 0x4D8,
	Skill1Pressed := 0x4D7,
	Skill2Unpressed := 0x4DB,
	Skill2Pressed := 0x4DA,
	Skill3Unpressed := 0x4DE,
	Skill3Pressed := 0x4DD,

	ModernEyeUnpressed := 0x5FC,
	ModernEyePressed := 0x5FB,
	ModernEditUnpressed := 0x5FE,
	ModernEditPressed := 0x5FD,
	ModernPreviousUnpressed := 0x60A,
	ModernPreviousPressed := 0x609,
	ModernDoublePreviousUnpressed := 0x606,
	ModernDoublePreviousPressed := 0x605,
	ModernNextUnpressed := 0x609,
	ModernNextPressed := 0x608,
	ModernDoubleNextUnpressed := 0x604,
	ModernDoubleNextPressed := 0x603,
	ModernXUnpressed := 0x600,
	ModernXPressed := 0x5FF,
	ModernVUnpressed := 0x602,
	ModernVPressed := 0x601,
endenum