include "system/exception/Exception";

class FileNotFoundException(Exception)
	function FileNotFoundException(this, message := "File not found", innerException := 0)
		Super(message, innerException);
	endfunction
endclass
