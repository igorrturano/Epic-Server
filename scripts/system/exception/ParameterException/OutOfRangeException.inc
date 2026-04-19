include "system/exception/Exception";

class OutOfRangeException(Exception)
	function OutOfRangeException(this, message := "Parameter out of range", innerException := 0)
		Super(message, innerException);
	endfunction
endclass