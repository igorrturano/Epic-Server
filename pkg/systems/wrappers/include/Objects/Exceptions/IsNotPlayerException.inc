include "system/exception/Exception";

class IsNotPlayerException(Exception)
	function IsNotPlayerException(this, message := "This mobile is not a player!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
