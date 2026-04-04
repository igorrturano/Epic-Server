include "system/exception/Exception";

class InvalidMasteryException(Exception)
	function InvalidMasteryException(this, message := "Invalid mastery.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
