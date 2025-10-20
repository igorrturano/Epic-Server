class IMessage()
	uninit function Message(text, default color, default font_number, default langSource, default langTarget);
	
	uninit function AddMessage(text, default color, default font, default langSource, default langTarget);
	uninit function SetColor(color);
	uninit function SetFont(font);
	uninit function SetSourceLanguage(lang);
	uninit function SetTargetLanguage(lang);
	
	uninit function Translate();
	
	uninit function SendSys(who);
	uninit function SendSysAll();
	uninit function SendAbove(who);
	uninit function SendAbovePrivate(who, default whoSee);
endclass