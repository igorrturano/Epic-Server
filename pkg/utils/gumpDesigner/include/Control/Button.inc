include "Control";

class Button(Control)
	var _image; // Imagem do botão
	var _pressedImage; // Imagem do botão quando pressionado
	var _buttonType;

	var _action; // Passar função lambda a ser executada no clique

	function Button(this, name, x, y, image, pressedImage, buttonType, action := 0, gumpId := 0)
		Super(GUMP_CONTROL::BUTTON, name, x, y, gumpId);
		this._image := image;
		this._pressedImage := pressedImage;
		this._buttonType := buttonType;

		this._action := action;
	endfunction

	function GetImage(this)
		return this._image;
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

	function SetAction(this, action)
		this._action := action;
	endfunction

	function SelfCreate(this, byRef gump)
		GFAddButton(gump, this.GetX(), this.GetY(), this.GetImage(), this.GetPressedImage(), this.GetButtonType(), this.GetGumpId());
	endfunction

	function ToString(this)
		return "Button(gumpId=" + this.GetGumpId()
			+ ", name=" + this._GetName()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", image=" + this.GetImage()
			+ ", pressedImage=" + this.GetPressedImage()
			+ ", action=" + this.GetAction()
			+ ")";
	endfunction
endclass

enum class GUMP_CONTROL_BUTTON_IMAGE
	CLOSE_UNPRESSED := 2103,
	CLOSE_PRESSED := 2104
endenum