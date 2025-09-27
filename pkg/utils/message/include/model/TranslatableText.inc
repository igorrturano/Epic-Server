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

	function TranslatableText(this, text := "", langSource := "", langTarget := "")
		this._source := struct{
			text := text,
			lang := langSource
		};
		this._target := struct{
			text := text,
			lang := langTarget
		};

		if (langTarget != "") this.Translate(); endif
	endfunction

	function GetTextSource(this)
		return this._source.text;
	endfunction

	function GetTextTarget(this)
		return this._target.text;
	endfunction

	function SetTextSource(this, text)
		this._source.text := text;
		return this;
	endfunction

	function SetLanguageSource(this, langSource)
		this._source.lang := langSource;
		return this;
	endfunction

	function SetLanguageTarget(this, langTarget)
		this._target.lang := langTarget;
		return this;
	endfunction

	function Translate(this)
		this._target.text := TranslateText(this._source.text, this._target.lang, this._source.lang);
		return this;
	endfunction

	function ToString(this)
		return $"Texto original ({this._source.lang}): {this._source.text}
			Texto traduzido ({this._target.lang}): {this._target.text}";
	endfunction
endclass

enum class LANGUAGE
	PORTUGUESE := "pt-BR",
	ENGLISH := "en-US",
	SPANISH := "es-ES"
endenum