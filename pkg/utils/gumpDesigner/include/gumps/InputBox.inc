include "Gump";

class InputBox()
	var title;
	var text;
	var maxLength;
	var isNumeric;
	var isPassword;
	var textColor;
	var value;

	function InputBox(this, title, text, maxLength := 100, isNumeric := false, isPassword := false, textColor := 0)
		this.title := title;
		this.text := text;
		this.maxLength := maxLength;
		this.isNumeric := isNumeric;
		this.isPassword := isPassword;
		this.textColor := textColor;
		this.value := "";
	endfunction
endclass