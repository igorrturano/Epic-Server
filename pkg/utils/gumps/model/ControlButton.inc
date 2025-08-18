include "Control";

class ControlButton(Control)
	var _image; // Imagem do botão
	var _pressedImage; // Imagem do botão quando pressionado

	var _action; // Passar função lambda a ser executada no clique

	function ControlButton(this, name, x, y, image, pressedImage, action, gumpId := 0)
		Super(GUMP_CONTROL_BUTTON, name, x, y, gumpId);
		this._image := image;
		this._pressedImage := pressedImage;

		this._action := action;
	endfunction

	function GetImage(this)
		return this._image;
	endfunction

	function GetPressedImage(this)
		return this._pressedImage;
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

	function SetAction(this, action)
		this._action := action;
	endfunction

	function ToString(this)
		return "ControlButton(name=" + this._GetName()
			+ ", gumpId=" + this.GetGumpId()
			+ ", type=" + this.GetType()
			+ ", x=" + this.GetX()
			+ ", y=" + this.GetY()
			+ ", image=" + this._image
			+ ", pressedImage=" + this._pressedImage
			+ ", action=" + this._action
			+ ")";
	endfunction
endclass

enum class GUMP_CONTROL_BUTTON_IMG
	CLOSE_UNPRESSED := 2103,
	CLOSE_PRESSED := 2104
endenum