include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class BodyLayout(Layout)
	function BodyLayout(this)
		Super();
		this._SetName("BodyLayout");
		this.InitializeControls();
		this.InsertControls();
	endfunction

	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBackground(factory.CreateImageGumpTiled()._SetName("Body Background"));
	endfunction
		
endclass