include "Layout";

class PaperBodyLayout(Layout)
	function PaperBodyLayout(this)
		Super();
	endfunction

	function ConfigureControls(this)
		this.GetBackground().SetImageId(9350);
	endfunction

	function PositionControls(this)
	endfunction

	function ToString(this)
		return "(PaperBodyLayout) " + Super::ToString(this);
	endfunction
endclass