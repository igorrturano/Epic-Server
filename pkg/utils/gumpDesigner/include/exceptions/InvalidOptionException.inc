include "system/exception/ParameterException/InvalidParameterException";

class InvalidOptionException(InvalidParameterException)
	function InvalidOptionException(this, message := "Invalid Option.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass