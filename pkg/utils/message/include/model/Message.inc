use math;
use os;

include "include/say";
include "TranslatableText";
// include ":message:TranslateController";
include ":message:Translator";

class Message()
	var _messages := array{ struct{ // Array de structs com as informações de cada mensagem: string, cor, fonte, lingua
		text := TranslatableText(),
		color := _DEFAULT_TEXT_COLOR,
		font := _DEFAULT_TEXT_FONT
	}};
	var _translator := Translator(); // Objeto tradutor

	function Message(this, text, color := _DEFAULT_TEXT_COLOR, font_number := _DEFAULT_TEXT_FONT, langSource := LANGUAGE::PORTUGUESE, langTarget := LANGUAGE::ENGLISH)
		this._messages := array{};
		this._translator := Translator();
		this.AddMessage(text, color, font_number, langSource, langTarget);
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
		foreach msg in (this._messages)
			msg.text := this._translator.SetText(msg.text).Translate();
		endforeach

		return this;
	endfunction

	function _SplitMessages(this, maxSize)
		var msgSplitted := array{};
		this.Translate();

		foreach msg in (this._messages)
			var msgFragmentsAmt := CInt(msg.text.GetTextTarget().length() / maxSize) + 1;
			
			for i := 1 to (msgFragmentsAmt)
				var start := ((i - 1) * maxSize) + 1;
				var length := Min(msg.text.GetTextTarget().length() - start + 1, maxSize);
				msgSplitted.append(struct{
					text := msg.text.GetTextTarget()[start, length],
					color := msg.color,
					font := msg.font
				});
			endfor
		endforeach

		return msgSplitted;
	endfunction

	function SendSys(this, who)
		var msgSplitted := this._SplitMessages(MSG_MAXSIZE::SYSMSG);

		foreach part in (msgSplitted)
			SendSysMessageUNI(who, part.text, part.color, part.font);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function SendAbove(this, above_object)
		var msgSplitted := this._SplitMessages(MSG_MAXSIZE::PRINTTEXT);

		foreach part in (msgSplitted)
			PrintText(above_object, part.text, part.color, part.font);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function SendAbovePrivate(this, above_object, whoSee := false)
		var msgSplitted := this._SplitMessages(MSG_MAXSIZE::PRINTTEXT);

		foreach part in (msgSplitted)
			if (!whoSee)
				PrintTextPrivate(above_object, part.text, above_object, part.color, part.font);
			else
				PrintTextPrivate(above_object, part.text, whoSee, part.color, part.font);
			endif
			
			SleepMS(2);
		endforeach

		return this;
	endfunction

endclass

enum class MSG_TYPE
	SYSMSG,
	PRINTTEXT,
endenum

enum class MSG_MAXSIZE
	SYSMSG := 200,
	PRINTTEXT := 60
endenum