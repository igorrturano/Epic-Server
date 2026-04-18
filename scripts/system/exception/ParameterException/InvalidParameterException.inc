include "system/exception/Exception";

class InvalidParameterException(Exception)
	function InvalidParameterException(this, message := "Invalid Parameter.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass