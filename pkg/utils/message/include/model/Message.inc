use math;
use os;

include "IMessage";
include "include/say";
include "include/client";
include ":text:_all";
include "system/exception/FormatException/InvalidFormatException";

class Message(IMessage)
	var _isAsync := false;
	var _translate := false; // Boolean indicando se a mensagem deve ser traduzida
	var _text; // TranslatableText object
	var _color; // Color object
	var _font; // Font object
	var _fromWhom; // Mobile object
	var _toWhom; // Mobile object
	var _type; // MSG_TYPE enum
	var _packetLanguage := "ENG"; // Marcador no campo "language" do pacote 0xAE — útil para hooks pularem nossos próprios envios

	function Message(this, text, color := false, font := false, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._isAsync := false;
		this._translate := false;
		this._text := TranslatableText(text, langSource, langTarget);
		this._color := color ?: Color();
		this._font := font ?: Font();
		this._packetLanguage := "ENG";
	endfunction

	function GetText(this)
		return this._text;
	endfunction
	
	function GetColor(this)
		return this._color;
	endfunction

	function GetFont(this)
		return this._font;
	endfunction

	function GetFromWhom(this)
		return this._fromWhom;
	endfunction

	function GetToWhom(this)
		return this._toWhom;
	endfunction

	function GetType(this)
		return this._type;
	endfunction

	function SetText(this, text)
		if (!(text is @TranslatableText))
			return InvalidFormatException("Parameter 'text' must be of type TranslatableText.");
		endif
		
		this._text := text;

		return this;
	endfunction

	function SetColor(this, color)
		if (!(color is @Color))
			return InvalidFormatException("Parameter 'color' must be of type Color.");
		endif

		this._color := color;

		return this;
	endfunction
	
	function SetFont(this, font)
		if (!(font is @Font))
			return InvalidFormatException("Parameter 'font' must be of type Font.");
		endif

		this._font := font;

		return this;
	endfunction

	function SetSourceLanguage(this, lang)
		if (!(this._messageTranslatableText is @TranslatableText))
			return InvalidFormatException("Internal message text is not of type TranslatableText.");
		endif
		
		this._text.SetSourceLanguage(lang);

		return this;
	endfunction

	function SetTargetLanguage(this, lang)
		if (!(this._messageTranslatableText is @TranslatableText))
			return InvalidFormatException("Internal message text is not of type TranslatableText.");
		endif

		this._text.SetTargetLanguage(lang);

		return this;
	endfunction

	function IsAsync(this)
		return this._isAsync;
	endfunction
	
	/**
	 * Define se a mensagem será processada de forma assíncrona.
	 * 
	 * @param setToAsync Boolean indicando se a mensagem deve ser assíncrona (true) ou não (false)
	 *
	 * @return Message Retorna a própria instância para permitir encadeamento de métodos
	 */
	function SetAsync(this, setToAsync)
		this._isAsync := setToAsync;
		
		return this;
	endfunction

	function WantTranslation(this)
		return this._translate;
	endfunction

	/**
	 * Explicitly opt this message into translation. Sets source/target languages,
	 * marks the message async (so the HTTP call doesn't block the caller),
	 * and turns on the _translate flag so _WantTranslation honors it.
	 */
	function RequestTranslation(this, sourceLang, targetLang)
		this._text.SetSourceLanguage(sourceLang);
		this._text.SetTargetLanguage(targetLang);
		this._translate := true;
		this.SetAsync(true);

		return this;
	endfunction

	function GetPacketLanguage(this)
		return this._packetLanguage;
	endfunction

	/**
	 * Define o campo "language" do pacote 0xAE (4 bytes). Usado para marcar
	 * pacotes gerados pelo servidor (ex.: tradução) para que hooks de fala
	 * possam diferenciar de mensagens reais do cliente e evitar loops.
	 */
	function SetPacketLanguage(this, langcode)
		this._packetLanguage := langcode;
		return this;
	endfunction

	function SendSys(this, who)
		if (!(who.isA(POLCLASS_MOBILE)))
			return InvalidFormatException("Parameter 'who' must be of type MobileRef.");
		endif
		
		this._toWhom := who;
		this._type := MESSAGE_TYPE::SYSTEM;
		if (this._color.GetValue() == GENERAL_COLOR::_DEFAULT)
			this._color := Color(MESSAGE_COLOR::SYSTEM);
		endif
		this._translate := this._WantTranslation(who);

		this._Send();

		return this;
	endfunction

	function SendSysAll(this)
		this._type := MESSAGE_TYPE::SYSTEM;

		// TODO

		return this;
	endfunction

	function SendAbove(this, who)
		if (!(who.isA(POLCLASS_MOBILE)))
			return InvalidFormatException("Parameter 'who' must be of type MobileRef.");
		endif
		
		this._toWhom := who;
		this._type := MESSAGE_TYPE::NORMAL;
		if (this._color.GetValue() == GENERAL_COLOR::_DEFAULT)
			this._color := Color(MESSAGE_COLOR::NORMAL);
		endif

		this._translate := this._WantTranslation(who);

		this._Send();

		return this;
	endfunction

	function SendAbovePrivate(this, who, whoSee := false)
		if (!(who.isA(POLCLASS_MOBILE)))
			return InvalidFormatException("Parameter 'who' must be of type MobileRef.");
		endif

		this._toWhom := who;
		this._fromWhom := whoSee ?: who;
		this._type := MESSAGE_TYPE::NORMAL_PRIVATE;
		if (this._color.GetValue() == GENERAL_COLOR::_DEFAULT)
			this._color := Color(MESSAGE_COLOR::PRIVATE);
		endif
		this._translate := this._WantTranslation(who);

		this._Send();

		return this;
	endfunction

	function SendEmote(this, who)
		if (!(who.isA(POLCLASS_MOBILE)))
			return InvalidFormatException("Parameter 'who' must be of type MobileRef.");
		endif

		this._toWhom := who;
		this._type := MESSAGE_TYPE::EMOTE;
		if (this._color.GetValue() == GENERAL_COLOR::_DEFAULT)
			this._color := Color(MESSAGE_COLOR::EMOTE);
		endif
		this._translate := this._WantTranslation(who);

		this._Send();

		return this;
	endfunction

	function ToString(this)
		var result := "Message{";
		result += "text: " + this._text.ToString();
		result += ", color: " + this._color.ToString();
		result += ", font: " + this._font.ToString();
		result += ", type: " + CStr(this._type);
		result += ", isAsync: " + CStr(this._isAsync);
		result += ", translate: " + CStr(this._translate);
		if (this._fromWhom)
			result += ", fromWhom: " + CStr(this._fromWhom.name);
		endif
		if (this._toWhom)
			result += ", toWhom: " + CStr(this._toWhom.name);
		endif
		result += "}";
		return result;
	endfunction

	// Private functions
	function _Send(this)
		if (this.IsAsync())
			Start_Script(":message:sender", this);
		else
			Run_Script(":message:sender", this);
		endif
	endfunction

	function _WantTranslation(this, who)
		if (this._translate)
			this.SetAsync(true);
			return true;
		endif
		return false;
	endfunction
endclass

enum class MESSAGE_TYPE
	NORMAL := 0x00,
	BROADCAST := 0x01,
	EMOTE := 0x02,
	NORMAL_PRIVATE := 0x03, // CUSTOM do Message()
	SYSTEM := 0x06, // SysMessage
	MESSAGE_CORNER := 0x07, // Mensagem no canto da tela, sem aparecer em cima do char
	WHISPER := 0x08, // Sussurro
	YELL := 0x09, // Grita
	SPELL := 0x0A, // PowerWords
	GUILD := 0x0D,
	ALLIANCE := 0x0E,
	COMMAND_PROMPTS := 0x0F,
endenum

enum class MESSAGE_MAXSIZE
	SYSTEM := 200,
	NORMAL := 60,
	EMOTE := NORMAL
endenum
