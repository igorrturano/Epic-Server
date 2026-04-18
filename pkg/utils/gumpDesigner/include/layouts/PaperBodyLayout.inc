include "BodyLayout";

class PaperBodyLayout(BodyLayout)
	function PaperBodyLayout(this)
		Super();
		this._SetName("PaperBodyLayout");
		this.ConfigureControls();
	endfunction

	// Override
	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBackground(factory.CreateImageGump()._SetName("Body Background"));
	endfunction
	
	function ConfigureControls(this)
		this.GetBackground().SetImageId(9350);
	endfunction

	function PositionControls(this)
		this.GetBackground()
			.Resize(this.GetWidth(), this.GetHeight());
	endfunction

	function ToString(this)
		return "(PaperBodyLayout) " + Super::ToString(this);
	endfunction
endclass