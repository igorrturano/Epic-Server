include "system/exception/Exception";

class InvalidTemplateException(Exception)
	function InvalidTemplateException(this, message := "Invalid template provided.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
