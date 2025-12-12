include "include/client";

class Color()
	var _name := ""; // ENUM da cor
	var _value := 0;
	var _htmlValue := "";

	function Color(this, colorEnum := COLOR_NAME::_DEFAULT)
		this._value := colorEnum;
		this._InitValues();
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function GetHtmlValue(this)
		return this._htmlValue;
	endfunction

	function ToString(this)
		return $"Color: Name={this._name}, BaseValue={this._value}, HTMLValue={this._htmlValue}";
	endfunction

	function _InitValues(this)
		if (this._value == COLOR_NAME::BLACK)
			this._name := "BLACK";
			this._htmlValue := "#000000";
		elseif (this._value == COLOR_NAME::TRUE_BLACK)
			this._name := "TRUE_BLACK";
			this._htmlValue := "#000000";
		elseif (this._value == COLOR_NAME::WHITE)
			this._name := "WHITE";
			this._htmlValue := "#FFFFFF";
		elseif (this._value == COLOR_NAME::RED)
			this._name := "RED";
			this._htmlValue := "#FF0000";
		elseif (this._value == COLOR_NAME::GREEN)
			this._name := "GREEN";
			this._htmlValue := "#00FF00";
		elseif (this._value == COLOR_NAME::BLUE)
			this._name := "BLUE";
			this._htmlValue := "#0000FF";
		elseif (this._value == COLOR_NAME::YELLOW)
			this._name := "YELLOW";
			this._htmlValue := "#FFFF00";
		elseif (this._value == COLOR_NAME::ORANGE)
			this._name := "ORANGE";
			this._htmlValue := "#FFA500";
		elseif (this._value == COLOR_NAME::PURPLE)
			this._name := "PURPLE";
			this._htmlValue := "#800080";
		elseif (this._value == COLOR_NAME::INDIGO_DARK)
			this._name := "INDIGO_DARK";
			this._htmlValue := "#2E0854";
		elseif (this._value == COLOR_NAME::INDIGO)
			this._name := "INDIGO";
			this._htmlValue := "#4B0082";
		elseif (this._value == COLOR_NAME::INDIGO_LIGHT)
			this._name := "INDIGO_LIGHT";
			this._htmlValue := "#6A0DAD";
		elseif (this._value == COLOR_NAME::VIOLET_DARK)
			this._name := "VIOLET_DARK";
			this._htmlValue := "#5D1A8C";
		elseif (this._value == COLOR_NAME::VIOLET)
			this._name := "VIOLET";
			this._htmlValue := "#8B00FF";
		elseif (this._value == COLOR_NAME::VIOLET_LIGHT)
			this._name := "VIOLET_LIGHT";
			this._htmlValue := "#9F7FFF";
		elseif (this._value == COLOR_NAME::MAGENTA_DARK)
			this._name := "MAGENTA_DARK";
			this._htmlValue := "#8B008B";
		elseif (this._value == COLOR_NAME::MAGENTA)
			this._name := "MAGENTA";
			this._htmlValue := "#FF00FF";
		elseif (this._value == COLOR_NAME::MAGENTA_LIGHT)
			this._name := "MAGENTA_LIGHT";
			this._htmlValue := "#FF77FF";
		elseif (this._value == COLOR_NAME::RED_DARK)
			this._name := "RED_DARK";
			this._htmlValue := "#8B0000";
		elseif (this._value == COLOR_NAME::RED_LIGHT)
			this._name := "RED_LIGHT";
			this._htmlValue := "#FF6B6B";
		elseif (this._value == COLOR_NAME::ORANGE_DARK)
			this._name := "ORANGE_DARK";
			this._htmlValue := "#CC8400";
		elseif (this._value == COLOR_NAME::ORANGE_LIGHT)
			this._name := "ORANGE_LIGHT";
			this._htmlValue := "#FFB347";
		elseif (this._value == COLOR_NAME::YELLOW_DARK)
			this._name := "YELLOW_DARK";
			this._htmlValue := "#CCCC00";
		elseif (this._value == COLOR_NAME::YELLOW_LIGHT)
			this._name := "YELLOW_LIGHT";
			this._htmlValue := "#FFFF99";
		elseif (this._value == COLOR_NAME::GREEN_DARK)
			this._name := "GREEN_DARK";
			this._htmlValue := "#006400";
		elseif (this._value == COLOR_NAME::GREEN_LIGHT)
			this._name := "GREEN_LIGHT";
			this._htmlValue := "#90EE90";
		elseif (this._value == COLOR_NAME::CYAN_DARK)
			this._name := "CYAN_DARK";
			this._htmlValue := "#008B8B";
		elseif (this._value == COLOR_NAME::CYAN)
			this._name := "CYAN";
			this._htmlValue := "#00FFFF";
		elseif (this._value == COLOR_NAME::CYAN_LIGHT)
			this._name := "CYAN_LIGHT";
			this._htmlValue := "#7FFFD4";
		elseif (this._value == COLOR_NAME::BLUE_DARK)
			this._name := "BLUE_DARK";
			this._htmlValue := "#00008B";
		elseif (this._value == COLOR_NAME::BLUE_LIGHT)
			this._name := "BLUE_LIGHT";
			this._htmlValue := "#87CEEB";
		elseif (this._value == COLOR_NAME::GRAY_DARK)
			this._name := "GRAY_DARK";
			this._htmlValue := "#404040";
		elseif (this._value == COLOR_NAME::GRAY)
			this._name := "GRAY";
			this._htmlValue := "#808080";
		elseif (this._value == COLOR_NAME::GRAY_LIGHT)
			this._name := "GRAY_LIGHT";
			this._htmlValue := "#C0C0C0";
		elseif (this._value == COLOR_NAME::BLACKMETAL)
			this._name := "BLACKMETAL";
			this._htmlValue := "#2C2C2C";
		elseif (this._value == COLOR_NAME::GOLDMETAL)
			this._name := "GOLDMETAL";
			this._htmlValue := "#D4AF37";
		elseif (this._value == COLOR_NAME::BSHIMMER)
			this._name := "BSHIMMER";
			this._htmlValue := "#B8860B";
		elseif (this._value == COLOR_NAME::STONE)
			this._name := "STONE";
			this._htmlValue := "#696969";
		elseif (this._value == COLOR_NAME::SPECTRAL)
			this._name := "SPECTRAL";
			this._htmlValue := "#9370DB";
		elseif (this._value == COLOR_NAME::TRANSLUCENT)
			this._name := "TRANSLUCENT";
			this._htmlValue := "#E0E0E080";
		elseif (this._value == COLOR_NAME::ELEMENTAL_FIRE)
			this._name := "ELEMENTAL_FIRE";
			this._htmlValue := "#FF4500";
		elseif (this._value == COLOR_NAME::ELEMENTAL_FIRE_LIGHT)
			this._name := "ELEMENTAL_FIRE_LIGHT";
			this._htmlValue := "#FF6347";
		elseif (this._value == COLOR_NAME::ELEMENTAL_ICE)
			this._name := "ELEMENTAL_ICE";
			this._htmlValue := "#00CED1";
		elseif (this._value == COLOR_NAME::ELEMENTAL_ICE_LIGHT)
			this._name := "ELEMENTAL_ICE_LIGHT";
			this._htmlValue := "#AFEEEE";
		elseif (this._value == COLOR_NAME::ELEMENTAL_ENERGY)
			this._name := "ELEMENTAL_ENERGY";
			this._htmlValue := "#9400D3";
		elseif (this._value == COLOR_NAME::ELEMENTAL_ENERGY_LIGHT)
			this._name := "ELEMENTAL_ENERGY_LIGHT";
			this._htmlValue := "#DA70D6";
		elseif (this._value == COLOR_NAME::NEUTRAL)
			this._name := "NEUTRAL";
			this._htmlValue := "#A9A9A9";
		elseif (this._value == COLOR_NAME::LAVENDER)
			this._name := "LAVENDER";
			this._htmlValue := "#E6E6FA";
		elseif (this._value == COLOR_NAME::MAUVE)
			this._name := "MAUVE";
			this._htmlValue := "#E0B0FF";
		endif
	endfunction
endclass

enum class COLOR_NAME
	BLACK := 1,
	TRUE_BLACK := 1279,
	WHITE := 2,
	RED := 38,
	GREEN := 68,
	BLUE := 90,
	YELLOW := 53,
	ORANGE := 3,
	PURPLE := 4,
	INDIGO_DARK := 0x0007,
	INDIGO := 0x0008,
	INDIGO_LIGHT := 0x0009,
	VIOLET_DARK := 0x000c,
	VIOLET := 0x000d,
	VIOLET_LIGHT := 0x000e,
	MAGENTA_DARK := 0x000f,
	MAGENTA := 0x0010,
	MAGENTA_LIGHT := 0x0011,
	RED_DARK := 0x0012,
	RED_LIGHT := 0x0013,
	ORANGE_DARK := 0x0014,
	ORANGE_LIGHT := 0x0015,
	YELLOW_DARK := 0x0016,
	YELLOW_LIGHT := 0x0017,
	GREEN_DARK := 0x0018,
	GREEN_LIGHT := 0x0019,
	CYAN_DARK := 0x001a,
	CYAN := 0x001b,
	CYAN_LIGHT := 0x001c,
	BLUE_DARK := 0x001d,
	BLUE_LIGHT := 0x001e,
	GRAY_DARK := 0x001f,
	GRAY := 0x0020,
	GRAY_LIGHT := 0x0021,
	LAVENDER := 20,
	MAUVE := 30,
	BLACKMETAL := 0x0022,
	GOLDMETAL := 0x0023,
	BSHIMMER := 0x0024,
	STONE := 0x0025,
	SPECTRAL := 0x0026,
	TRANSLUCENT := 0x0027,
	ELEMENTAL_FIRE := 0x0028,
	ELEMENTAL_FIRE_LIGHT := 0x0029,
	ELEMENTAL_ICE := 0x002a,
	ELEMENTAL_ICE_LIGHT := 0x002b,
	ELEMENTAL_ENERGY := 0x002c,
	ELEMENTAL_ENERGY_LIGHT := 0x002d,

	FAIL := RED,
	ERRO := RED,
	BAD := RED,
	SUCCESS := BLUE,
	GOOD := BLUE,
	INFO := BLUE,
	ALERT := YELLOW,
	NEUTRAL := GRAY,

	_DEFAULT := BLUE
endenum