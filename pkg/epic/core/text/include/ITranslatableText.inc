class ITranslatableText()
	uninit function GetTextSource(this);
	uninit function GetTextTarget(this);
	uninit function GetLanguageSource(this);
	uninit function GetLanguageTarget(this);

	uninit function SetTextSource(this, text);
	uninit function SetTextTarget(this, text);
	uninit function SetSourceLanguage(this, lang);
	uninit function SetTargetLanguage(this, lang);
endclass