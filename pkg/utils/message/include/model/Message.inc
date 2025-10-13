use math;
use os;

include "include/say";
include "TranslatableText";
include ":message:Translator";

class Message()
	var _asyncInfo := struct{
		sending := false, // Em situações de script critico precisa ser assincrono. Definir isso com SetAsync().
		translation := false // Se ele é assincrono, a tradução também, e é feita no processo do sender.
	}; 
	var _messages := array{ struct{ // Array de structs com as informações de cada mensagem: string, cor, fonte, lingua
		text := TranslatableText(),
		color := _DEFAULT_TEXT_COLOR,
		font := _DEFAULT_TEXT_FONT
	}};
	var _translator := Translator(); // Objeto tradutor

	function Message(this, text, color := _DEFAULT_TEXT_COLOR, font_number := _DEFAULT_TEXT_FONT, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._asyncInfo := struct{
			sending := false,
			translation := false
		};
		this._messages := array{};
		this._translator := Translator();
		this.AddMessage(text, color, font_number, langSource, langTarget);
	endfunction

	function SetAsync(this, sending := true, translation := true)
		this._asyncInfo.sending := sending;
		this._asyncInfo.translation := translation;
		return this;
	endfunction

	function AddMessage(this, text, color := _DEFAULT_TEXT_COLOR, font := _DEFAULT_TEXT_FONT, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._messages.append(struct{
			text := TranslatableText(text, langSource, langTarget),
			color := color,
			font := font
		});

		return this;
	endfunction

	function SetColor(this, color)
		foreach msg in (this._messages)
			msg.color := color;
		endforeach

		return this;
	endfunction
	
	function SetFont(this, font)
		foreach msg in (this._messages)
			msg.font := font;
		endforeach

		return this;
	endfunction

	function SetLanguage(this, langSource, langTarget)
		foreach msg in (this._messages)
			msg.text.SetLanguage(langSource, langTarget);
		endforeach

		return this;
	endfunction

	function Translate(this)
		print("Translating all messages...");
		foreach msg in (this._messages)
			msg.text := this._translator.SetTranslatableText(msg.text).Translate().GetTranslatableText();
		endforeach

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
			print("Sending message...");

			foreach part in (msgSplitted)
				print("Sending message part...");
				print("Text: " + part);
				PrintText(who, part.textTarget, part.color, part.font);
			endforeach
		else
			print("Sending async message...");
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

	function _WantTranslation(who)
		return true; // Temporário, depois fazer verificação se o char quer tradução ou não
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