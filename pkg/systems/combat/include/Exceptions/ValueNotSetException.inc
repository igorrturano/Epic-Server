include "system/exception/Exception";

class ValueNotSetException(Exception)
	function ValueNotSetException(this, message := "Value not set.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
