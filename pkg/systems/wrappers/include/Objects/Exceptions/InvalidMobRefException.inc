include "system/exception/Exception";

class InvalidMobRefException(Exception)
	function InvalidMobRefException(this, message := "Invalid mobRef. Maybe a template?", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
