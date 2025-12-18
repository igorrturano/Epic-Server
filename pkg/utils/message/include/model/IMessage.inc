class IMessage()
	uninit function IMessage(this, text, default color, default font_number, default langSource, default langTarget);
	
	uninit function SetColor(this, color);
	uninit function SetFont(this, font);
	uninit function SetSourceLanguage(this, lang);
	uninit function SetTargetLanguage(this, lang);
	
	uninit function SendSys(this, who);
	uninit function SendSysAll(this);
	uninit function SendAbove(this, who);
	uninit function SendAbovePrivate(this, who, default whoSee);
endclass