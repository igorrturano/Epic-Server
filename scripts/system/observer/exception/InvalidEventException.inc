include "system/exception/Exception";

class InvalidEventException(Exception)
	function InvalidEventException(this, message := "Invalid or missing event!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass