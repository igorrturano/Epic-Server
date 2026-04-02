include "system/exception/Exception";

class InvalidAbilityException(Exception)
	function InvalidAbilityException(this, message := "Invalid ability.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
