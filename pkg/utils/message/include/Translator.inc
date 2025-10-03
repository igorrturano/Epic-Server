use os;

include "model/TranslatableText";
include "controller/TranslateController";

class Translator()
	var _translatableText := TranslatableText(); // Objeto de texto traduzível
	
	function Translator(this, translatableText := false)
		this._translatableText := translatableText;
	endfunction

	function SetText(this, text)
		this._translatableText := text;
		return this;
	endfunction

	function Translate(this)
		if (!this._translatableText) return error{ errortext := "There is no text to translate"}; endif
		
		// var translated := Run_Script(":message:translate", this._translatableText);
		var translated := TranslateObj(this._translatableText);
		this._translatableText := translated;
		print ("Translated text Translator(): " + this._translatableText);
		
		return this._translatableText;
	endfunction
endclass