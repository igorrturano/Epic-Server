use math;
use os;

include "include/say";

const _SYS_MESSAGE_MAX_SIZE := 200;
const _ABOVE_MESSAGE_MAX_SIZE := 60;

class Message()
	var _font := _DEFAULT_TEXT_FONT;
	var _color := _DEFAULT_TEXT_COLOR;
	var _originalMessage := "";
	var _newMessageFragments := array{}; // Array de strings

	function Message(this, originalMessage, SSM_TYPE := _DEFAULT_TEXT_COLOR, font_number := _DEFAULT_TEXT_FONT)
		this._font := font_number;
		this._color := SSM_TYPE;
		this._originalMessage := originalMessage;
		this._newMessageFragments := array{};
		this.AddNewMsg(originalMessage);
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

	function AddNewMsg(this, newMsg)
		var newMsgArray := array{};
		var msgFragmentsAmt := CInt(newMsg.length() / _SYS_MESSAGE_MAX_SIZE) + 1;
		
		for i := 1 to (msgFragmentsAmt)
			var start := ((i - 1) * _SYS_MESSAGE_MAX_SIZE) + 1;
			var length := Min(newMsg.length() - start + 1, _SYS_MESSAGE_MAX_SIZE);
			newMsgArray.append(newMsg[start, length]);
		endfor
		
		foreach part in (newMsgArray)
			this._newMessageFragments.append(part);
		endforeach
	endfunction

	function SendSys(this, who)
		foreach part in (this._newMessageFragments)
			SendSysMessageUNI(who, part, this._color, this._font, "ENG");
			SleepMS(2);
		endforeach
	endfunction

	function SendAbove(this, above_object)
		foreach part in (this._newMessageFragments)
			PrintText(above_object, part, this._color, this._font, "ENG");
			SleepMS(2);
		endforeach
	endfunction
endclass