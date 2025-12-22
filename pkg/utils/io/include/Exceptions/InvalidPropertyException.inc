include "system/exception/Exception";

class InvalidPropertyException(Exception)
	function InvalidPropertyException(this, message := "Invalid or missing property!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass