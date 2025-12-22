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
		var cfgFile := CFGFile("text", "configs/Color").SetElementName(this._value);

		this._name := cfgFile.GetPropertyValue("Name");
		this._htmlValue := cfgFile.GetPropertyValue("HTMLValue");
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

enum class MESSAGE_COLOR
	NORMAL := 690,
	EMOTE := 33,
	WHISPER := 51,
	YELL := 33,
	SYSTEM := COLOR_NAME::YELLOW,
	PRIVATE := COLOR_NAME::MAGENTA
endenum