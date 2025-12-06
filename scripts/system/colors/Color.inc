include "include/client";

class Color()
	var _name := ""; // ENUM da cor
	var _textValue := 0;
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

	function GetTextValue(this)
		return this._textValue;
	endfunction

	function GetHtmlValue(this)
		return this._htmlValue;
	endfunction

	function _Initialize(this)
		if (this._name == COLOR_ENUM::BLACK)
			this._textValue := FONTCOLOR_BLACK;
			this._htmlValue := "#000000";
		elseif (this._name == COLOR_ENUM::WHITE)
			// this._textValue := FONTCOLOR_WHITE;
			this._htmlValue := "#FFFFFF";
		elseif (this._name == COLOR_ENUM::RED || this._name == COLOR_ENUM::FAIL || this._name == COLOR_ENUM::ERRO || this._name == COLOR_ENUM::BAD)
			this._textValue := FONTCOLOR_RED;
			this._htmlValue := "#FF0000";
		elseif (this._name == COLOR_ENUM::GREEN)
			this._textValue := FONTCOLOR_GREEN;
			this._htmlValue := "#00FF00";
		elseif (this._name == COLOR_ENUM::BLUE || this._name == COLOR_ENUM::GOOD || this._name == COLOR_ENUM::SUCCESS || this._name == COLOR_ENUM::INFO)
			this._textValue := FONTCOLOR_BLUE;
			this._htmlValue := "#0000FF";
		elseif (this._name == COLOR_ENUM::YELLOW || this._name == COLOR_ENUM::ALERT)
			this._textValue := FONTCOLOR_YELLOW;
			this._htmlValue := "#FFFF00";
		elseif (this._name == COLOR_ENUM::ORANGE)
			// this._textValue := 0xFFA500;
			this._htmlValue := "#FFA500";
		elseif (this._name == COLOR_ENUM::PURPLE)
			// this._textValue := 0x800080;
			this._htmlValue := "#800080";
		elseif (this._name == COLOR_ENUM::INDIGO_DARK)
			this._textValue := COLOR_INDIGO_DARK;
			this._htmlValue := "#2E0854";
		elseif (this._name == COLOR_ENUM::INDIGO)
			this._textValue := COLOR_INDIGO;
			this._htmlValue := "#4B0082";
		elseif (this._name == COLOR_ENUM::INDIGO_LIGHT)
			this._textValue := COLOR_INDIGO_LIGHT;
			this._htmlValue := "#6A0DAD";
		elseif (this._name == COLOR_ENUM::VIOLET_DARK)
			this._textValue := COLOR_VIOLET_DARK;
			this._htmlValue := "#5D1A8C";
		elseif (this._name == COLOR_ENUM::VIOLET)
			this._textValue := COLOR_VIOLET;
			this._htmlValue := "#8B00FF";
		elseif (this._name == COLOR_ENUM::VIOLET_LIGHT)
			this._textValue := COLOR_VIOLET_LIGHT;
			this._htmlValue := "#9F7FFF";
		elseif (this._name == COLOR_ENUM::MAGENTA_DARK)
			this._textValue := COLOR_MAGENTA_DARK;
			this._htmlValue := "#8B008B";
		elseif (this._name == COLOR_ENUM::MAGENTA)
			this._textValue := COLOR_MAGENTA;
			this._htmlValue := "#FF00FF";
		elseif (this._name == COLOR_ENUM::MAGENTA_LIGHT)
			this._textValue := COLOR_MAGENTA_LIGHT;
			this._htmlValue := "#FF77FF";
		elseif (this._name == COLOR_ENUM::RED_DARK)
			this._textValue := COLOR_RED_DARK;
			this._htmlValue := "#8B0000";
		elseif (this._name == COLOR_ENUM::RED_LIGHT)
			this._textValue := COLOR_RED_LIGHT;
			this._htmlValue := "#FF6B6B";
		elseif (this._name == COLOR_ENUM::ORANGE_DARK)
			this._textValue := COLOR_ORANGE_DARK;
			this._htmlValue := "#CC8400";
		elseif (this._name == COLOR_ENUM::ORANGE_LIGHT)
			this._textValue := COLOR_ORANGE_LIGHT;
			this._htmlValue := "#FFB347";
		elseif (this._name == COLOR_ENUM::YELLOW_DARK)
			this._textValue := COLOR_YELLOW_DARK;
			this._htmlValue := "#CCCC00";
		elseif (this._name == COLOR_ENUM::YELLOW_LIGHT)
			this._textValue := COLOR_YELLOW_LIGHT;
			this._htmlValue := "#FFFF99";
		elseif (this._name == COLOR_ENUM::GREEN_DARK)
			this._textValue := COLOR_GREEN_DARK;
			this._htmlValue := "#006400";
		elseif (this._name == COLOR_ENUM::GREEN_LIGHT)
			this._textValue := COLOR_GREEN_LIGHT;
			this._htmlValue := "#90EE90";
		elseif (this._name == COLOR_ENUM::CYAN_DARK)
			this._textValue := COLOR_CYAN_DARK;
			this._htmlValue := "#008B8B";
		elseif (this._name == COLOR_ENUM::CYAN)
			this._textValue := COLOR_CYAN;
			this._htmlValue := "#00FFFF";
		elseif (this._name == COLOR_ENUM::CYAN_LIGHT)
			this._textValue := COLOR_CYAN_LIGHT;
			this._htmlValue := "#7FFFD4";
		elseif (this._name == COLOR_ENUM::BLUE_DARK)
			this._textValue := COLOR_BLUE_DARK;
			this._htmlValue := "#00008B";
		elseif (this._name == COLOR_ENUM::BLUE_LIGHT)
			this._textValue := COLOR_BLUE_LIGHT;
			this._htmlValue := "#87CEEB";
		elseif (this._name == COLOR_ENUM::GRAY_DARK)
			this._textValue := COLOR_GRAY_DARK;
			this._htmlValue := "#404040";
		elseif (this._name == COLOR_ENUM::GRAY)
			this._textValue := COLOR_GRAY;
			this._htmlValue := "#808080";
		elseif (this._name == COLOR_ENUM::GRAY_LIGHT)
			this._textValue := COLOR_GRAY_LIGHT;
			this._htmlValue := "#C0C0C0";
		elseif (this._name == COLOR_ENUM::BLACKMETAL)
			this._textValue := COLOR_BLACKMETAL;
			this._htmlValue := "#2C2C2C";
		elseif (this._name == COLOR_ENUM::GOLDMETAL)
			this._textValue := COLOR_GOLDMETAL;
			this._htmlValue := "#D4AF37";
		elseif (this._name == COLOR_ENUM::BSHIMMER)
			this._textValue := COLOR_BSHIMMER;
			this._htmlValue := "#B8860B";
		elseif (this._name == COLOR_ENUM::STONE)
			this._textValue := COLOR_STONE;
			this._htmlValue := "#696969";
		elseif (this._name == COLOR_ENUM::SPECTRAL)
			this._textValue := COLOR_SPECTRAL;
			this._htmlValue := "#9370DB";
		elseif (this._name == COLOR_ENUM::TRANSLUCENT)
			this._textValue := COLOR_TRANSLUCENT;
			this._htmlValue := "#E0E0E080";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_FIRE)
			this._textValue := COLOR_ELEMENTAL_FIRE;
			this._htmlValue := "#FF4500";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_FIRE_LIGHT)
			this._textValue := COLOR_ELEMENTAL_FIRE_LIGHT;
			this._htmlValue := "#FF6347";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_ICE)
			this._textValue := COLOR_ELEMENTAL_ICE;
			this._htmlValue := "#00CED1";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_ICE_LIGHT)
			this._textValue := COLOR_ELEMENTAL_ICE_LIGHT;
			this._htmlValue := "#AFEEEE";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_ENERGY)
			this._textValue := COLOR_ELEMENTAL_ENERGY;
			this._htmlValue := "#9400D3";
		elseif (this._name == COLOR_ENUM::ELEMENTAL_ENERGY_LIGHT)
			this._textValue := COLOR_ELEMENTAL_ENERGY_LIGHT;
			this._htmlValue := "#DA70D6";
		elseif (this._name == COLOR_ENUM::NEUTRAL)
			this._textValue := FONTCOLOR_NEUTRAL;
			this._htmlValue := "#A9A9A9";
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
	INDIGO_DARK,
	INDIGO,
	INDIGO_LIGHT,
	VIOLET_DARK,
	VIOLET,
	VIOLET_LIGHT,
	MAGENTA_DARK,
	MAGENTA,
	MAGENTA_LIGHT,
	RED_DARK,
	RED_LIGHT,
	ORANGE_DARK,
	ORANGE_LIGHT,
	YELLOW_DARK,
	YELLOW_LIGHT,
	GREEN_DARK,
	GREEN_LIGHT,
	CYAN_DARK,
	CYAN,
	CYAN_LIGHT,
	BLUE_DARK,
	BLUE_LIGHT,
	GRAY_DARK,
	GRAY,
	GRAY_LIGHT,
	BLACKMETAL,
	GOLDMETAL,
	BSHIMMER,
	STONE,
	SPECTRAL,
	TRANSLUCENT,
	ELEMENTAL_FIRE,
	ELEMENTAL_FIRE_LIGHT,
	ELEMENTAL_ICE,
	ELEMENTAL_ICE_LIGHT,
	ELEMENTAL_ENERGY,
	ELEMENTAL_ENERGY_LIGHT,

	FAIL,
	ERRO,
	BAD,
	SUCCESS,
	GOOD,
	INFO,
	ALERT,
	NEUTRAL,

	
endenum