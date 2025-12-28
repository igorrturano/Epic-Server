include "system/exception/Exception";

class InvalidElementException(Exception)
	function InvalidElementException(this, message := "Invalid or missing element!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass