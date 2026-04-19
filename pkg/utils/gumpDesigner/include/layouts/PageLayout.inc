include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class PageLayout(Layout)
	function PageLayout(this)
		Super();
		this._SetName("PageLayout");
		this.InitializeControls();
		this.InsertControls();
	endfunction

	function AddControl(this, control)
		Super::AddControl(this, control);
		print("Added control to page: " + control.ToString());
	endfunction

	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBackground(factory.CreateImageGumpTiled()._SetName("Page Background"));
	endfunction
		
endclass