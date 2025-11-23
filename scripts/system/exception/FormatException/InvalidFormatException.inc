include "system/exception/Exception";

class InvalidFormatException(Exception)
	function InvalidFormatException(this, message := "Invalid format provided.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
