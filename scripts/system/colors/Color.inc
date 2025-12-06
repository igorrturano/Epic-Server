include "include/client";

class Color()
	var _name := ""; // ENUM da cor
	var _fontValue := 0;
	var _htmlValue := "";

	function Color(this, name)
		this._name := name;
		this._Initialize();
	endfunction

	function ToString(this)
		return $"Color: Name={this._name}, BaseValue={this._baseValue}, HTMLValue={this._htmlValue}";
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetFontValue(this)
		return this._fontValue;
	endfunction

	function GetHtmlValue(this)
		return this._htmlValue;
	endfunction

	function _Initialize(this)
		if (this._name == COLOR_ENUM::BLACK)
			this._fontValue := FONTCOLOR_BLACK;
			this._htmlValue := "#000000";
		elseif (this._name == COLOR_ENUM::WHITE)
			// this._fontValue := FONTCOLOR_WHITE;
			this._htmlValue := "#FFFFFF";
		elseif (this._name == COLOR_ENUM::RED || this._name == COLOR_ENUM::FAIL || this._name == COLOR_ENUM::ERRO || this._name == COLOR_ENUM::BAD)
			this._fontValue := FONTCOLOR_RED;
			this._htmlValue := "#FF0000";
		elseif (this._name == COLOR_ENUM::GREEN)
			this._fontValue := FONTCOLOR_GREEN;
			this._htmlValue := "#00FF00";
		elseif (this._name == COLOR_ENUM::BLUE || this._name == COLOR_ENUM::GOOD || this._name == COLOR_ENUM::SUCCESS || this._name == COLOR_ENUM::INFO)
			this._fontValue := FONTCOLOR_BLUE;
			this._htmlValue := "#0000FF";
		elseif (this._name == COLOR_ENUM::YELLOW || this._name == COLOR_ENUM::ALERT)
			this._fontValue := FONTCOLOR_YELLOW;
			this._htmlValue := "#FFFF00";
		elseif (this._name == COLOR_ENUM::ORANGE)
			this._fontValue := 0xFFA500;
			this._htmlValue := "#FFA500";
		elseif (this._name == COLOR_ENUM::PURPLE)
			this._fontValue := 0x800080;
			this._htmlValue := "#800080";
		endif
	endfunction
endclass

enum class COLOR_ENUM
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
	ORANGE,
	PURPLE,

	FAIL,
	ERRO,
	BAD,
	SUCCESS,
	GOOD,
	INFO,
	ALERT,
	NEUTRAL
endenum