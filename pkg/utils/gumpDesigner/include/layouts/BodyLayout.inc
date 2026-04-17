include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class BodyLayout(Layout)
	function BodyLayout(this)
		Super();
		this._SetName("BodyLayout");
	endfunction
endclass