include "system/exception/Exception";

class IsNotNPCException(Exception)
	function IsNotNPCException(this, message := "This mobile is not a NPC!", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
