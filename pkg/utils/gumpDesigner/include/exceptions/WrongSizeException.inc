include "system/exception/Exception";

class WrongSizeException(Exception)
	function WrongSizeException(this, message := "Invalid size", innerException := 0)
		Super(message, innerException);
	endfunction
endclass