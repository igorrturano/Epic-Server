include "system/exception/Exception";

class InvalidProcessException(Exception)
	function InvalidProcessException(this, message := "Invalid process.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
