include "system/exception/Exception";

class InvalidParameterException(Exception)
	function InvalidParameterException(this, message := "Invalid parameter.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
