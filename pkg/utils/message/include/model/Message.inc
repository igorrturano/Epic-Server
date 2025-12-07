use math;
use os;

include "IMessage";
include "include/say";
include "include/client";
include "system/text/_all";
include "system/exception/FormatException/InvalidFormatException";

// TODO: Fazer receber apenas 1 mensagem, para evitar bugs de concatenação
class Message(IMessage)
	var _isAsync := false;
	var _translate := false; // Boolean indicando se a mensagem deve ser traduzida
	var _text; // TranslatableText object
	var _color; // Color object
	var _font; // Font object
	var _fromWhom; // Mobile object
	var _toWhom; // Mobile object
	var _type; // MSG_TYPE enum
	
	function Message(this, text, color := false, font := false, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._isAsync := false;
		this._translate := false;
		this._text := TranslatableText(text, langSource, langTarget);
		this._color := color ?: Color();
		this._font := font ?: Font();
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

	function SendSys(this, who)
		this._toWhom := who;
		this._type := MSG_TYPE::SYSMSG;
		this._translate := _WantTranslation(who);

		this._TranslateAndSend();

		return this;
	endfunction

	function SendSysAll(this)
		this._type := MSG_TYPE::SYSMSGALL;

		return this;
	endfunction

	function SendAbove(this, who)
		this._toWhom := who;
		this._type := MSG_TYPE::PRINTTEXT;
		this._translate := _WantTranslation(who);

		this._TranslateAndSend();

		return this;
	endfunction

	function SendAbovePrivate(this, who, whoSee := false)
		this._toWhom := who;
		this._fromWhom := whoSee ?: who;
		this._type := MSG_TYPE::PRINTTEXTPRIVATE;
		this._translate := _WantTranslation(who);

		this._TranslateAndSend();

		return this;
	endfunction

	function SendEmote(this, who)
		this._toWhom := who;
		this._type := MSG_TYPE::EMOTE;
		this._translate := _WantTranslation(who);

		this._TranslateAndSend();

		return this;
	endfunction

	function ToString(this)
		var result := "Message{";
		result += "text: " + this._text.ToString();
		result += ", color: " + this._color.ToString();
		result += ", font: " + this._font.ToString();
		result += ", type: " + this._type;
		result += ", isAsync: " + this._isAsync;
		result += ", translate: " + this._translate;
		if (this._fromWhom)
			result += ", fromWhom: " + this._fromWhom.name;
		endif
		if (this._toWhom)
			result += ", toWhom: " + this._toWhom.name;
		endif
		result += "}";
		return result;
	endfunction

	// Private functions
	function _TranslateAndSend(this)
		if (this.IsAsync())
			Start_Script(":message:sender", this);
		else
			Run_Script(":message:sender", this);
		endif
	endfunction

	function _WantTranslation(who)
		return false; // TODO: Pegar prop se o char quer tradução ou não
	endfunction
endclass

enum class MSG_TYPE
	SYSMSG,
	SYSMSGALL,
	PRINTTEXT,
	PRINTTEXTPRIVATE,
	EMOTE
endenum

enum class MSG_MAXSIZE
	SYSMSG := 200,
	PRINTTEXT := 60,
	EMOTE := PRINTTEXT
endenum