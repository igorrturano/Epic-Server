include "include/client";

class Color()
	var _name := ""; // ENUM da cor
	var _value := 0;
	var _htmlValue := "";

	function Color(this, colorEnum := 0)
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
		var value := this._value;
		var name := "";
		var htmlValue := "";

		if (value == COLOR_NAME::_DEFAULT)
			name := "DEFAULT";
			htmlValue := "#000000";
		elseif (value == COLOR_NAME::BLACK)
			name := "BLACK";
			htmlValue := "#000000";
		elseif (value == COLOR_NAME::TRUE_BLACK)
			name := "TRUE_BLACK";
			htmlValue := "#000000";
		elseif (value == COLOR_NAME::WHITE)
			name := "WHITE";
			htmlValue := "#FFFFFF";
		elseif (value == COLOR_NAME::RED)
			name := "RED";
			htmlValue := "#FF0000";
		elseif (value == COLOR_NAME::GREEN)
			name := "GREEN";
			htmlValue := "#00FF00";
		elseif (value == COLOR_NAME::BLUE)
			name := "BLUE";
			htmlValue := "#0000FF";
		elseif (value == COLOR_NAME::YELLOW)
			name := "YELLOW";
			htmlValue := "#FFFF00";
		elseif (value == COLOR_NAME::ORANGE)
			name := "ORANGE";
			htmlValue := "#FFA500";
		elseif (value == COLOR_NAME::PURPLE)
			name := "PURPLE";
			htmlValue := "#800080";
		elseif (value == COLOR_NAME::INDIGO_DARK)
			name := "INDIGO_DARK";
			htmlValue := "#2E0854";
		elseif (value == COLOR_NAME::INDIGO)
			name := "INDIGO";
			htmlValue := "#4B0082";
		elseif (value == COLOR_NAME::INDIGO_LIGHT)
			name := "INDIGO_LIGHT";
			htmlValue := "#6A0DAD";
		elseif (value == COLOR_NAME::VIOLET_DARK)
			name := "VIOLET_DARK";
			htmlValue := "#5D1A8C";
		elseif (value == COLOR_NAME::VIOLET)
			name := "VIOLET";
			htmlValue := "#8B00FF";
		elseif (value == COLOR_NAME::VIOLET_LIGHT)
			name := "VIOLET_LIGHT";
			htmlValue := "#9F7FFF";
		elseif (value == COLOR_NAME::MAGENTA_DARK)
			name := "MAGENTA_DARK";
			htmlValue := "#8B008B";
		elseif (value == COLOR_NAME::MAGENTA)
			name := "MAGENTA";
			htmlValue := "#FF00FF";
		elseif (value == COLOR_NAME::MAGENTA_LIGHT)
			name := "MAGENTA_LIGHT";
			htmlValue := "#FF77FF";
		elseif (value == COLOR_NAME::RED_DARK)
			name := "RED_DARK";
			htmlValue := "#8B0000";
		elseif (value == COLOR_NAME::RED_LIGHT)
			name := "RED_LIGHT";
			htmlValue := "#FF6B6B";
		elseif (value == COLOR_NAME::ORANGE_DARK)
			name := "ORANGE_DARK";
			htmlValue := "#CC8400";
		elseif (value == COLOR_NAME::ORANGE_LIGHT)
			name := "ORANGE_LIGHT";
			htmlValue := "#FFB347";
		elseif (value == COLOR_NAME::YELLOW_DARK)
			name := "YELLOW_DARK";
			htmlValue := "#CCCC00";
		elseif (value == COLOR_NAME::YELLOW_LIGHT)
			name := "YELLOW_LIGHT";
			htmlValue := "#FFFF99";
		elseif (value == COLOR_NAME::GREEN_DARK)
			name := "GREEN_DARK";
			htmlValue := "#006400";
		elseif (value == COLOR_NAME::GREEN_LIGHT)
			name := "GREEN_LIGHT";
			htmlValue := "#90EE90";
		elseif (value == COLOR_NAME::CYAN_DARK)
			name := "CYAN_DARK";
			htmlValue := "#008B8B";
		elseif (value == COLOR_NAME::CYAN)
			name := "CYAN";
			htmlValue := "#00FFFF";
		elseif (value == COLOR_NAME::CYAN_LIGHT)
			name := "CYAN_LIGHT";
			htmlValue := "#7FFFD4";
		elseif (value == COLOR_NAME::BLUE_DARK)
			name := "BLUE_DARK";
			htmlValue := "#00008B";
		elseif (value == COLOR_NAME::BLUE_LIGHT)
			name := "BLUE_LIGHT";
			htmlValue := "#87CEEB";
		elseif (value == COLOR_NAME::GRAY)
			name := "GRAY";
			htmlValue := "#808080";
		elseif (value == COLOR_NAME::GRAY_DARK)
			name := "GRAY_DARK";
			htmlValue := "#404040";
		elseif (value == COLOR_NAME::GRAY_LIGHT)
			name := "GRAY_LIGHT";
			htmlValue := "#C0C0C0";
		elseif (value == COLOR_NAME::BLACKMETAL)
			name := "BLACKMETAL";
			htmlValue := "#2C2C2C";
		elseif (value == COLOR_NAME::GOLDMETAL)
			name := "GOLDMETAL";
			htmlValue := "#D4AF37";
		elseif (value == COLOR_NAME::BSHIMMER)
			name := "BSHIMMER";
			htmlValue := "#B8860B";
		elseif (value == COLOR_NAME::STONE)
			name := "STONE";
			htmlValue := "#696969";
		elseif (value == COLOR_NAME::SPECTRAL)
			name := "SPECTRAL";
			htmlValue := "#9370DB";
		elseif (value == COLOR_NAME::TRANSLUCENT)
			name := "TRANSLUCENT";
			htmlValue := "#E0E0E080";
		elseif (value == COLOR_NAME::ELEMENTAL_FIRE)
			name := "ELEMENTAL_FIRE";
			htmlValue := "#FF4500";
		elseif (value == COLOR_NAME::ELEMENTAL_FIRE_LIGHT)
			name := "ELEMENTAL_FIRE_LIGHT";
			htmlValue := "#FF6347";
		elseif (value == COLOR_NAME::ELEMENTAL_ICE)
			name := "ELEMENTAL_ICE";
			htmlValue := "#00CED1";
		elseif (value == COLOR_NAME::ELEMENTAL_ICE_LIGHT)
			name := "ELEMENTAL_ICE_LIGHT";
			htmlValue := "#AFEEEE";
		elseif (value == COLOR_NAME::ELEMENTAL_ENERGY)
			name := "ELEMENTAL_ENERGY";
			htmlValue := "#9400D3";
		elseif (value == COLOR_NAME::ELEMENTAL_ENERGY_LIGHT)
			name := "ELEMENTAL_ENERGY_LIGHT";
			htmlValue := "#DA70D6";
		elseif (value == COLOR_NAME::LAVENDER)
			name := "LAVENDER";
			htmlValue := "#E6E6FA";
		elseif (value == COLOR_NAME::MAUVE)
			name := "MAUVE";
			htmlValue := "#E0B0FF";
		endif

		this._name := name;
		this._htmlValue := htmlValue;
	endfunction
endclass

enum class COLOR_NAME
	_DEFAULT := 0,
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
endenum

enum class SYSTEM_COLOR
	INFO := COLOR_NAME::BLUE,
	GOOD := COLOR_NAME::BLUE,
	SUCCESS := COLOR_NAME::BLUE,

	ALERT := COLOR_NAME::YELLOW,

	FAIL := COLOR_NAME::RED,
	ERRO := COLOR_NAME::RED,
	BAD := COLOR_NAME::RED
endenum

enum class COMBAT_COLOR
	PHYSICAL := COLOR_NAME::YELLOW,
	CRITICAL := COLOR_NAME::RED,
	FIRE := COLOR_NAME::ELEMENTAL_FIRE,
	COLD := COLOR_NAME::ELEMENTAL_ICE,
	POISON := COLOR_NAME::GREEN,
	ENERGY := COLOR_NAME::ELEMENTAL_ENERGY,
	HEAL := COLOR_NAME::BLUE
endenum