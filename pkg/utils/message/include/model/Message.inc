use math;
use os;

include "IMessage";
include ":message:Translator";
include "include/say";
include "include/client";
include "system/text/_all";
include "system/exception/FormatException/InvalidFormatException";

class Message(IMessage)
	var _messageTranslatableText := "";
	var _messageColor; // Color class
	var _messageFont; // Font class
	var _asyncInfo := struct{
		sending := false, // Em situações de script critico precisa ser assincrono. Definir isso com SetAsync().
		translation := false // Se ele é assincrono, a tradução também, e é feita no processo do sender.
	};

	function Message(this, text, color := false, font := false, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._messageTranslatableText := text ?: TranslatableText("", langSource, langTarget);
		this._messageColor := color ?: Color();
		this._messageFont := font ?: Font();

		this._asyncInfo := struct{
			sending := false,
			translation := false
		};
	endfunction

	function GetText(this)
		return this._messageTranslatableText;
	endfunction

	function GetColor(this)
		return this._messageColor;
	endfunction

	function GetFont(this)
		return this._messageFont;
	endfunction

	function SetText(this, text)
		if (!(text is @TranslatableText))
			return InvalidFormatException("Parameter 'text' must be of type TranslatableText.");
		endif
		
		this._messageTranslatableText := text;

		return this;
	endfunction

	function SetColor(this, color)
		if (!(color is @Color))
			return InvalidFormatException("Parameter 'color' must be of type Color.");
		endif

		this._messageColor := color;

		return this;
	endfunction
	
	function SetFont(this, font)
		if (!(font is @Font))
			return InvalidFormatException("Parameter 'font' must be of type Font.");
		endif

		this._messageFont := font;

		return this;
	endfunction

	function SetSourceLanguage(this, lang)
		if (!(this._messageTranslatableText is @TranslatableText))
			return InvalidFormatException("Internal message text is not of type TranslatableText.");
		endif
		
		this._messageTranslatableText.SetSourceLanguage(lang);

		return this;
	endfunction

	function SetTargetLanguage(this, lang)
		if (!(this._messageTranslatableText is @TranslatableText))
			return InvalidFormatException("Internal message text is not of type TranslatableText.");
		endif

		this._messageTranslatableText.SetTargetLanguage(lang);

		return this;
	endfunction

	function IsAsync(this)
		return this._asyncInfo.sending;
	endfunction
	
	function SetAsync(this, sending := true, translation := true)
		this._asyncInfo.sending := sending;
		this._asyncInfo.translation := translation;
		
		return this;
	endfunction

	function Translate(this)
		var wantTranslation := false; // TODO: Pegar prop do player

		if (wantTranslation)
			foreach msg in (this._messages)
				msg.text := this._translator.SetTranslatableText(msg.text).Translate().GetTranslatableText();
			endforeach
		endif

		return this;
	endfunction

	function SendSys(this, who)
		if (!this._asyncInfo.sending)
			if (_WantTranslation(who)) this.Translate(); endif

			var msgSplitted := this._SplitMessages(MSG_MAXSIZE::SYSMSG);

			foreach part in (msgSplitted)
				SendSysMessageUNI(who, part.textTarget, part.color, part.font);
			endforeach
		else
			return this._SendAsync(struct{
				who := who,
				msg_type := MSG_TYPE::SYSMSG
			});
		endif

		return this;
	endfunction

	function SendSysAll(this)
		if (!this._asyncInfo.sending)
			var msgSplitted := this._SplitMessages(MSG_MAXSIZE::SYSMSG);

			var wantTranslation := 0;
			foreach who in (EnumerateOnlineCharacters())
				if (_WantTranslation(who)) wantTranslation++; endif // Conta quantos querem tradução
			endforeach

			if (wantTranslation > 0) this.Translate(); endif // Se pelo menos 1 quiser tradução, vai ter que traduzir...

			foreach who in (EnumerateOnlineCharacters())
				foreach part in (msgSplitted)
					if (_WantTranslation(who)) // Se quiser tradução, envia o texto traduzido
						SendSysMessageUNI(who, part.textTarget, part.color, part.font);
					else // Se não quiser tradução, envia o texto original
						SendSysMessageUNI(who, part.textSource, part.color, part.font);
					endif
				endforeach
			endforeach
		else
			return this._SendAsync(struct{
				msg_type := MSG_TYPE::SYSMSG
			});
		endif

		return this;
	endfunction

	function SendAbove(this, who)
		if (!this._asyncInfo.sending)
			if (_WantTranslation(who)) this.Translate(); endif
			var msgSplitted := this._SplitMessages(MSG_MAXSIZE::PRINTTEXT);

			foreach part in (msgSplitted)
				PrintText(who, part.textTarget, part.color, part.font);
			endforeach
		else
			return this._SendAsync(struct{
				who := who,
				msg_type := MSG_TYPE::PRINTTEXT
			});
			
		endif

		return this;
	endfunction

	function SendAbovePrivate(this, who, whoSee := false)
		if (!this._asyncInfo.sending)
			if (_WantTranslation(who)) this.Translate(); endif
			var msgSplitted := this._SplitMessages(MSG_MAXSIZE::PRINTTEXT);

			foreach part in (msgSplitted)
				if (whoSee)
					PrintTextPrivate(who, part.textTarget, whoSee, part.color, part.font);
				else
					PrintTextPrivate(who, part.textTarget, who, part.color, part.font);
				endif
			endforeach
		else
			return this._SendAsync(struct{
				who := whoSee ?: who,
				msg_type := MSG_TYPE::PRINTTEXTPRIVATE
			});
		endif

		return this;
	endfunction

	function SendEmote(this, who)
		if (!this._asyncInfo.sending)
			if (_WantTranslation(who)) this.Translate(); endif
			var msgSplitted := this._SplitMessages(MSG_MAXSIZE::PRINTTEXT);

			foreach part in (msgSplitted)
				PrintText(who, $"*{part.textTarget}*", part.color, part.font);
			endforeach
		else
			return this._SendAsync(struct{
				who := who,
				msg_type := MSG_TYPE::PRINTTEXT
			});
		endif

		return this;
	endfunction

	// Private functions
	function _SplitMessages(this, maxSize)
		var msgSplitted := array{};

		foreach msg in (this._messages)
			// var msgFragmentsAmtSource := this._CountFragments(msg.text.GetTextSource(), maxSize);
			var msgFragmentsAmtTarget := this._CountFragments(msg.text.GetTextTarget(), maxSize);

			for i := 1 to (msgFragmentsAmtTarget)
				var start := ((i - 1) * maxSize) + 1;
				var length := Min(msg.text.GetTextTarget().length() - start + 1, maxSize);
				msgSplitted.append(struct{
					textSource := msg.text.GetTextSource()[start, length],
					textTarget := msg.text.GetTextTarget()[start, length],
					color := msg.color,
					font := msg.font
				});
			endfor
		endforeach

		return msgSplitted;
	endfunction

	function _CountFragments(this, text, maxSize)
		return CInt(text.length() / maxSize) + 1;
	endfunction

	function _SendAsync(this, params)
		params.+message := this;
		params.+translation := this._asyncInfo.translation;
		return Start_Script(":message:sender", params);
	endfunction

	function _WantTranslation(this)
		return false; // Temporário, depois fazer verificação se o char quer tradução ou não
	endfunction
endclass

enum class MSG_TYPE
	SYSMSG,
	SYSMSGALL,
	PRINTTEXT,
	PRINTTEXTPRIVATE
endenum

enum class MSG_MAXSIZE
	SYSMSG := 200,
	PRINTTEXT := 60
endenum