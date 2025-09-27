use math;
use os;

include "include/say";

class Message()
	var _originalMessage := struct{
		text := "",
		lang := ""
	};
	var _newMessageFragments := array{ struct{ // Array de structs com as informações de cada fragmento da mensagem: string, cor, fonte
		str := "",
		color := _DEFAULT_TEXT_COLOR,
		font := _DEFAULT_TEXT_FONT
	}};

	function Message(this, originalMessage, color := _DEFAULT_TEXT_COLOR, font_number := _DEFAULT_TEXT_FONT)
		this._font := font_number;
		this._color := color;
		this._originalMessage := originalMessage;
		this._newMessageFragments := array{};
		this.AddMessage(originalMessage, color, font_number);
	endfunction

	function GetFont(this)
		return this._font;
	endfunction

	function GetColor(this)
		return this._color;
	endfunction

	function GetOriginalMessage(this)
		return this._originalMessage;
	endfunction

	function GetFragments(this)
		return this._newMessageFragments;
	endfunction

	function AddMessage(this, msgType, text, color := _DEFAULT_TEXT_COLOR, font := _DEFAULT_TEXT_FONT)
		var newMsgArray := array{};
		var maxSize := 1;
		if (msgType == MSG_TYPE::SYSMSG) maxSize := MSG_MAXSIZE::SYSMSG;
		elseif (msgType == MSG_TYPE::PRINTTEXT) maxSize := MSG_MAXSIZE::PRINTTEXT;
		endif
		
		var msgFragmentsAmt := CInt(text.length() / maxSize) + 1;
		
		for i := 1 to (msgFragmentsAmt)
			var start := ((i - 1) * maxSize) + 1;
			var length := Min(text.length() - start + 1, maxSize);
			newMsgArray.append(struct{
				text := text[start, length],
				color := color,
				font := font
			});
		endfor
		
		foreach part in (newMsgArray)
			this._newMessageFragments.append(part);
		endforeach

		return this;
	endfunction

	function SendSys(this, who)
		foreach part in (this._newMessageFragments)
			SendSysMessageUNI(who, part.text, part.color, part.font);
			SleepMS(2);
		endforeach
	endfunction

	function SendAbove(this, above_object)
		foreach part in (this._newMessageFragments)
			PrintText(above_object, part.text, part.color, part.font);
			SleepMS(2);
		endforeach
	endfunction

	function SendAbovePrivate(this, above_object, whoSee := false)
		foreach part in (this._newMessageFragments)
			if (!whoSee)
				PrintTextPrivate(above_object, part.text, above_object, part.color, part.font);
			else
				PrintTextPrivate(above_object, part.text, whoSee, part.color, part.font);
			endif
			
			SleepMS(2);
		endforeach
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