include "ITranslatableText";

class TranslatableText(ITranslatableText)
	var _sourceText := "";
	var _sourceLang := LANGUAGE::PORTUGUESE;
	var _targetText := "";
	var _targetLang := LANGUAGE::ENGLISH;
	
	function TranslatableText(this, text := "", langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._sourceText := text;
		this._sourceLang := langSource;
		this._targetText := text;
		this._targetLang := langTarget;
	endfunction

	function GetTextSource(this)
		return this._sourceText;
	endfunction

	function GetTextTarget(this)
		return this._targetText;
	endfunction

	function GetLanguageSource(this)
		return this._sourceLang;
	endfunction

	function GetLanguageTarget(this)
		return this._targetLang;
	endfunction

	function SetTextSource(this, text)
		this._sourceText := text;
		return this;
	endfunction

	function SetTextTarget(this, text)
		this._targetText := text;
		return this;
	endfunction

	function SetSourceLanguage(this, lang)
		this._sourceLang := lang;
		return this;
	endfunction

	function SetTargetLanguage(this, lang)
		this._targetLang := lang;
		return this;
	endfunction
	
	function ToString(this)
		return $"Texto original ({this._sourceLang}): {this._sourceText} -> Texto traduzido ({this._targetLang}): {this._targetText}";
	endfunction
endclass

enum class LANGUAGE
	PORTUGUESE := "pt-BR",
	ENGLISH := "en",
	SPANISH := "es"
endenum