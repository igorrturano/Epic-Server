include "system/exception/Exception";

class IsNPCException(Exception)
	function IsNPCException(this, message := "This mobile is a NPC!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
