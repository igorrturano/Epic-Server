include "system/exception/Exception";

class IsTemplateException(Exception)
	function IsTemplateException(this, message := "This is a template!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
