include "system/exception/Exception";

class InvalidModTypeException(Exception)
	function InvalidModTypeException(this, message := "Invalid Modificator Type.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
