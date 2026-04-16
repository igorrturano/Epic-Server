include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class TitleLayout(Layout)
	var text;
	var helpButton;
	
	function TitleLayout(this, title := "")
		Super();
		this._SetName("TitleLayout");

		this.SetBackground(GumpFactory().CreateImageGump()._SetName("Gump Title Background"));
		this.SetText(GumpFactory().CreateText()._SetName("Gump Title Text").SetText(title));
		this.SetHelpButton(GumpFactory().CreateButton()._SetName("Gump Title Help Button"));
	endfunction

	function GetText(this)
		return this.text;
	endfunction

	function SetText(this, text)
		if (!(text is @Text))
			return InvalidParameterException("Title text must be of type Text.");
		endif
		
		this.text := text;

		return this;
	endfunction

	function GetHelpButton(this)
		return this.helpButton;
	endfunction
	
	function SetHelpButton(this, helpButton)
		if (!(helpButton is @Button))
			return InvalidParameterException("Help button must be of type Button.");
		endif
		
		this.helpButton := helpButton;

		return this;
	endfunction

	function ToString(this)
		return "(GumpTitle) " + Super::ToString(this)
			+ ", text=" + this.GetText().ToString()
			+ ", helpButton=" + this.GetHelpButton().ToString();
	endfunction
endclass