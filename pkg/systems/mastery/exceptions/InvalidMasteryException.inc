include "system/exception/Exception";

class InvalidMasteryException(Exception)
	function InvalidMasteryException(this, message)
		Super(message);
	endfunction
endclass