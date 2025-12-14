class Font()
	var _fontType := FONT_TYPE::_DEFAULT; // FONT_TYPE enum
	var _value := 0;
	var _fontName := FONT_UNICODE_NAME::_DEFAULT; // FONT_NAME enum

	function Font(this, fontName := FONT_UNICODE_NAME::_DEFAULT, fontType := FONT_TYPE::_DEFAULT)
		this._fontType := fontType;
		this._value := fontName;
		this._Initialize();
	endfunction

	function GetFontType(this)
		return this._fontType;
	endfunction

	function GetFontName(this)
		return this._fontName;
	endfunction

	function GetValue(this)
		return this._value;
	endfunction

	function ToString(this)
		return $"Font {this._fontName}: Type={this._fontType}, ID={this._value} ({CStr(Hex(this._value))})";
	endfunction

	function _Initialize(this)
		if (this._fontType == FONT_TYPE::ASCII)
			case (this._value)
				FONT_ASCII_NAME::BOLD: this._fontName := "BOLD";
				FONT_ASCII_NAME::SHADOW: this._fontName := "SHADOW";
				FONT_ASCII_NAME::BOLD_SHAD: this._fontName := "BOLD_SHAD";
				FONT_ASCII_NAME::NORMAL: this._fontName := "NORMAL";
				FONT_ASCII_NAME::GOTHIC: this._fontName := "GOTHIC";
				FONT_ASCII_NAME::ITALIC: this._fontName := "ITALIC";
				FONT_ASCII_NAME::SMALL_DARK: this._fontName := "SMALL_DARK";
				FONT_ASCII_NAME::COLOR: this._fontName := "COLOR";
				FONT_ASCII_NAME::RUNE: this._fontName := "RUNE";
				FONT_ASCII_NAME::SMALL_LIGHT: this._fontName := "SMALL_LIGHT";
			endcase
		elseif (this._fontType == FONT_TYPE::UNICODE)
			case (this._value)
				FONT_UNICODE_NAME::UO: this._fontName := "UO";
				FONT_UNICODE_NAME::ARIAL_BOLD: this._fontName := "ARIAL_BOLD";
				FONT_UNICODE_NAME::ARIAL: this._fontName := "ARIAL";
				FONT_UNICODE_NAME::FIXEDSYS: this._fontName := "FIXEDSYS";
				FONT_UNICODE_NAME::FIXEDSYS_BIG: this._fontName := "FIXEDSYS_BIG";
				FONT_UNICODE_NAME::THIN: this._fontName := "THIN";
				FONT_UNICODE_NAME::THIN_BIG: this._fontName := "THIN_BIG";
				FONT_UNICODE_NAME::RUNE: this._fontName := "RUNE";
				FONT_UNICODE_NAME::RUNE_BOLD: this._fontName := "RUNE_BOLD";
				FONT_UNICODE_NAME::RUNE_BOLD_BIG: this._fontName := "RUNE_BOLD_BIG";
				FONT_UNICODE_NAME::RUNE_WITHNUMBER: this._fontName := "RUNE_WITHNUMBER";
				FONT_UNICODE_NAME::RUNE_WITHNUMBER_BOLD: this._fontName := "RUNE_WITHNUMBER_BOLD";
				FONT_UNICODE_NAME::RUNE_WITHNUMBER_BOLD_BIG2: this._fontName := "RUNE_WITHNUMBER_BOLD_BIG2";
			endcase
		endif
	endfunction
endclass

enum class FONT_TYPE
	ASCII := "ASCII",
	UNICODE := "UNICODE",

	_DEFAULT := UNICODE
endenum

enum class FONT_ASCII_NAME
	BOLD,
	SHADOW,
	BOLD_SHAD,
	NORMAL,
	GOTHIC,
	ITALIC,
	SMALL_DARK,
	COLOR,
	RUNE,
	SMALL_LIGHT,
	
	_DEFAULT := NORMAL
endenum

enum class FONT_UNICODE_NAME
	UO,
	ARIAL_BOLD,
	ARIAL,
	FIXEDSYS,
	FIXEDSYS_BIG,
	THIN,
	THIN_BIG,
	RUNE,
	RUNE_BOLD,
	RUNE_BOLD_BIG,
	RUNE_WITHNUMBER,
	RUNE_WITHNUMBER_BOLD,
	RUNE_WITHNUMBER_BOLD_BIG2,

	_DEFAULT := UO
endenum