include "system/exception/Exception";

class InvalidEquipmentException(Exception)
	function InvalidEquipmentException(this, message := "Invalid Equipment Type.", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
