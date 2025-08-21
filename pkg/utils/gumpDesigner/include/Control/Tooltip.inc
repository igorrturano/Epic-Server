include "Control";

class Tooltip(Control)
	function Tooltip(this, name)
		Super(GUMP_CONTROL::TOOLTIP, name, 0, 0);
	endfunction
endclass