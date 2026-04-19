include "PageLayout";

class PaperPageLayout(PageLayout)
	function PaperPageLayout(this)
		Super();
		this._SetName("PaperPageLayout");
		this.ConfigureControls();
	endfunction

	// Override
	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBackground(factory.CreateImageGump()._SetName("Page Background"));
	endfunction
	
	function ConfigureControls(this)
		this.GetBackground().SetImageId(9350);
	endfunction

	function PositionControls(this)
		this.GetBackground()
			.Resize(this.GetWidth(), this.GetHeight());
	endfunction

	function ToString(this)
		return "(PaperPageLayout) " + Super::ToString(this);
	endfunction
endclass