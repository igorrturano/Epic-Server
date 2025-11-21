include "system/exception/Exception";

class InvalidPropException(Exception)
	function InvalidPropException(this, message := "Invalid or missing property!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass