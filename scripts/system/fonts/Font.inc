class Font()
	var _fontName; // FONT_NAME enum
	var _textValue := 0;

	function Font(this, fontName)
		this._fontName := fontName;
		this._Initialize();
	endfunction

	function ToString(this)
		return $"Font: Type={this._fontType}, Name={this._fontName}";
	endfunction
endclass

enum class FONT_TYPE
	ASCII := "ASCII",
	UNICODE := "UNICODE"
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