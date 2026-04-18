include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class TitleLayout(Layout)
	var titleString;
	var text;
	var helpButton;
	
	function TitleLayout(this, title := "")
		Super();
		this._SetName("TitleLayout");
		this.SetTitleString(title);

		this.InitializeControls();
		this.InsertControls();
	endfunction

	function GetTitleString(this)
		return this.titleString;
	endfunction

	function SetTitleString(this, title)
		this.titleString := CStr(title);

		return this;
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

	function InsertControls(this)
		Super::InsertControls(this);

		this.AddControl(this.GetText());
		this.AddControl(this.GetHelpButton());
	endfunction
	
	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBackground(factory.CreateImageGumpTiled()._SetName("Title Background"));
		this.SetText(factory.CreateText()._SetName("Gump Title Text").SetValue(this.GetTitleString()));
		this.SetHelpButton(factory.CreateButton()._SetName("Gump Title Help Button").SetUnpressedImage(0x5689).SetPressedImage(0x568A));
	endfunction

	function ToString(this)
		return "(GumpTitle) " + Super::ToString(this)
			+ ", titleString=" + this.GetTitleString()
			+ ", text=" + this.GetText().ToString()
			+ ", helpButton=" + this.GetHelpButton().ToString();
	endfunction
endclass