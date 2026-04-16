include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class BodyLayout(Layout)
	function BodyLayout(this)
		Super();
		this._SetName("BodyLayout");
		this.SetBackground(ImageGump("Gump Body Background", 0, 0, 0, 0));
	endfunction
endclass