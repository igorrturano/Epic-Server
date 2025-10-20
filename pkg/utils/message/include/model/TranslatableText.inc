include "ITranslatable";

class TranslatableText(ITranslatable)
	var _source := struct{
		text := "",
		lang := LANGUAGE::PORTUGUESE
	};
	var _target := struct{
		text := "",
		lang := LANGUAGE::ENGLISH
	};

	function TranslatableText(this, text := "", langSource := "", langTarget := "")
		this._source := struct{
			text := text,
			lang := langSource
		};
		this._target := struct{
			text := text,
			lang := langTarget
		};
	endfunction

	function GetTextSource(this)
		return this._source.text;
	endfunction

	function GetTextTarget(this)
		return this._target.text;
	endfunction

	function GetLanguageSource(this)
		return this._source.lang;
	endfunction

	function GetLanguageTarget(this)
		return this._target.lang;
	endfunction

	function SetTextSource(this, text)
		this._source.text := text;
		return this;
	endfunction

	function SetTextTarget(this, text)
		this._target.text := text;
		return this;
	endfunction

	function SetSourceLanguage(this, lang)
		this._source.lang := lang;
		return this;
	endfunction

	function SetTargetLanguage(this, lang)
		this._target.lang := lang;
		return this;
	endfunction
	
	function ToString(this)
		return $"Texto original ({this._source.lang}): {this._source.text} -> Texto traduzido ({this._target.lang}): {this._target.text}";
	endfunction
endclass

enum class LANGUAGE
	PORTUGUESE := "pt-BR",
	ENGLISH := "en",
	SPANISH := "es"
endenum