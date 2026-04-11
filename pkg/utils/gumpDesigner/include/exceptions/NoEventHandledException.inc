include "system/exception/Exception";

class NoEventHandledException(Exception)
	function NoEventHandledException(this, message := "Invalid Parameter.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass