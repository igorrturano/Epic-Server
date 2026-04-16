include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class TitleLayout(Layout)
	var text;
	
	function TitleLayout(this, title := "")
		Super();
		this._SetName("TitleLayout");

		this.SetBackground(GumpFactory().CreateImageGump()._SetName("Gump Title Background"));
		this.SetText(GumpFactory().CreateText()._SetName("Gump Title Text").SetText(title));
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

	function ToString(this)
		return "(GumpTitle) " + Super::ToString(this)
			+ ", text=" + this.GetText().ToString();
	endfunction
endclass