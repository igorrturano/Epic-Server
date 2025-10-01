include ":message:TranslateController";

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
	endfunction

	function GetTextSource(this)
		return this._source.text;
	endfunction

	function GetLanguageSource(this)
		return this._source.lang;
	endfunction

	function GetLanguageTarget(this)
		return this._target.lang;
	endfunction

	function GetTextTarget(this)
		return this._target.text;
	endfunction

	function SetTextSource(this, text)
		this._source.text := text;
		return this;
	endfunction

	function SetLanguage(this, langSource, langTarget)
		this._source.lang := langSource;
		this._target.lang := langTarget;
		return this;
	endfunction

	function Translate(this)
		print("Chamou Translate!");
		// this._target.text := TranslateText(this._source.text, this._source.lang, this._target.lang);
		var translation := (Run_Script(":message:translate", this)).GetTextTarget();
		Wait_For_Event(10);
		this._target.text := translation;
		print(this.ToString());
		return this;
	endfunction

	function ToString(this)
		return $"Texto original ({this._source.lang}): {this._source.text}
			Texto traduzido ({this._target.lang}): {this._target.text}";
	endfunction
endclass

enum class LANGUAGE
	PORTUGUESE := "pt-BR",
	ENGLISH := "en",
	SPANISH := "es"
endenum