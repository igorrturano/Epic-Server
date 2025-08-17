include ":gumps:gumps_ex";

class Gump()
	var title;
	var width;
	var height;

	function Gump(this, title, width, height)
		this.title := title;
		this.width := width;
		this.height := height;
		this.elements := array{};
	endfunction

	function Show(this)
		
	endfunction
endclass