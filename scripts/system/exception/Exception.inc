use os;
include "system/diagnostics/StackTrace/StackTrace";

class Exception()
	var _message := "";
	var _innerException;
	var _stackTrace;

	function Exception(this, message := "", innerException := 0)
		this._message := message;
		this._innerException := innerException;
		this._stackTrace := StackTrace();
	endfunction

	function GetMessage(this)
		return this._message;
	endfunction

	function GetInnerException(this)
		return this._innerException;
	endfunction

	function GetStackTrace(this)
		return this._stackTrace;
	endfunction

	function printStackTrace(this)
		SysLog(this._stackTrace.ToString());
	endfunction

	function ToString(this)
		var result := "Exception: " + this._message;

		if (this._stackTrace)
			result += "\nStack Trace:\n" + this._stackTrace.ToString();
		endif
		
		if (this._innerException)
			result += "\nInner Exception:\n" + this._innerException.ToString();
		endif

		return result;
	endfunction
endclass
