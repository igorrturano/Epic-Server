include "include/epicLoggerUtils";

class TranslatableText()
	var _source := struct{
		text := "",
		lang := LANGUAGE::PORTUGUESE
	};
	var _target := struct{
		text := "",
		lang := LANGUAGE::ENGLISH
	};

	function TranslatableText(this, text := "", langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._source := struct{
			text := text,
			lang := langSource
		};
		this._target := struct{
			text := text,
			lang := langTarget
		};
	endfunction

	function GetTextOriginal(this)
		return this._source.text;
	endfunction

	function GetTextTranslated(this)
		return this._target.text;
	endfunction

	function SetText(this, text)
		this._source.text := text;
		return this;
	endfunction

	function SetLangSource(this, langSource)
		this._source.lang := langSource;
		return this;
	endfunction

	function SetLangTarget(this, langTarget)
		this._target.lang := langTarget;
		return this;
	endfunction

	function ToString(this)
		return $"Texto original ({this._source.lang}): {this._source.text}
			Texto traduzido ({this._target.lang}): {this._target.text}";
	endfunction

	function _Translate(this)
		this._target.text := TranslateText(this._source.text, this._target.lang, this._source.lang);
		return this;
	endfunction
endclass

enum class LANGUAGE
	PORTUGUESE := "pt-BR",
	ENGLISH := "en-US",
	SPANISH := "es-ES"
endenum