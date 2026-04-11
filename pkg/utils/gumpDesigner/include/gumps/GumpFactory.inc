// include ":gumps:gumps_ex";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";
include ":gumpDesigner:controls/ImageGumpTiled";
include ":gumpDesigner:layouts/GumpLayout";

class DiagFactory()
	function DiagFactory(this)
	endfunction

	function GetGump(this)
		return this.gump;
	endfunction

	function CreateTitle(this)
		return this;
	endfunction

	function CreatePage(this)
		return this;
	endfunction

	function CreateLayout(this)
		return this;
	endfunction

	function CreateGump(this, width, height, x := 0, y := 0)
		
	endfunction
endclass
