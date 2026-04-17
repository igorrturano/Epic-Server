include "TitleLayout";

class PaperTitleLayout(TitleLayout)
	function PaperTitleLayout(this, title := "")
		Super(title);
		this._SetName("PaperTitleLayout");
		this.ConfigureControls();
	endfunction

	// Override
	function InitializeControls(this)
		Super::InitializeControls(this);
		
		this.SetBackground(GumpFactory().CreateImageGump()._SetName("Title Background"));
	endfunction

	function ConfigureControls(this)
		this.GetBackground().SetImageId(9350);
	endfunction

	function PositionControls(this)
		this.GetText()
			.SetPosition(this.GetX() + 10, this.GetY() + 10)
			.SetWidth(this.GetWidth() - 20);

		this.GetHelpButton()
			.SetPosition(this.GetX() + this.GetWidth() - 30, this.GetY() + 10);
	endfunction

	function ToString(this)
		return "(PaperTitleLayout) " + Super::ToString(this);
	endfunction
endclass