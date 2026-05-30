include "system/exception/Exception";

class IsPlayerException(Exception)
	function IsPlayerException(this, message := "This mobile is a player!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
