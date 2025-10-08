class ITranslatable()
	uninit function GetTextSource(this);
	uninit function GetTextTarget(this);
	uninit function GetLanguageSource(this);
	uninit function GetLanguageTarget(this);

	uninit function SetTextSource(this, text);
	uninit function SetTextTarget(this, text);
	uninit function SetLanguage(this, langSource, langTarget);
endclass