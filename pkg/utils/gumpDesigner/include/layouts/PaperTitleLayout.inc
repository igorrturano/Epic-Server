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
		this.GetBackground()
			.Resize(this.GetWidth(), this.GetHeight());
		
		this.GetText()
			.SetAlign(TextAlign::Center)
			.SetPosition(10, 10)
			.SetWidth(this.GetWidth() - (2 * this.GetText().GetX()));

		this.GetHelpButton()
			.SetPosition(CInt(this.GetWidth() - 25), CInt(this.GetHeight() / 2) - 8);
	endfunction

	function ToString(this)
		return "(PaperTitleLayout) " + Super::ToString(this);
	endfunction
endclass