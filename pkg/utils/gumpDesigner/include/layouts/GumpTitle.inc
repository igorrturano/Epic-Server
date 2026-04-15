include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:factories/GumpFactory";
include ":gumpDesigner:layouts/GumpLayout";

class GumpTitle(Container)
	function GumpTitle(this, title := "")
		Super("Gump Title Container", 0, 0, 0, 0);

		this.AddControl(GumpFactory().CreateImageGump()._SetName("Gump Title Background"));
		this.AddControl(GumpFactory().CreateText()._SetName("Gump Title Text").SetText(title));
	endfunction

	function GetBackground(this)
		return this.GetControls()[1];
	endfunction

	function SetBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Title background must be of type ImageGump.");
		endif

		this.GetControls()[1] := imageGump;

		return this;
	endfunction

	function GetText(this)
		return this.GetControls()[2];
	endfunction

	function SetText(this, text)
		if (!(text is @Text))
			return InvalidParameterException("Title text must be of type Text.");
		endif
		
		this.GetControls()[2] := text;

		return this;
	endfunction

	function SetHeight(this, height)
		this.GetBackground().SetHeight(height);
		Super::SetHeight(this, height);

		return this;
	endfunction
	
	function SetWidth(this, width)
		this.GetBackground().SetWidth(width);
		this.GetText().SetWidth(width);
		Super::SetWidth(this, width);

		return this;
	endfunction

	function Resize(this, width, height)
		this.SetWidth(width);
		this.SetHeight(height);

		return this;
	endfunction

	function ResizeToLayout(this, layout)
		if (!(layout is @GumpLayout))
			return InvalidParameterException("Layout must be of type 'GumpLayout'.");
		endif

		this.SetX(layout.GetTitleArea_X());
		this.SetY(layout.GetTitleArea_Y());
		this.SetWidth(layout.GetTitleArea_Width());
		this.SetHeight(layout.GetTitleArea_Height());

		return this;
	endfunction

	function ToString(this)
		return "(GumpTitle) " + Super::ToString(this)
			+ ", background=" + this.GetBackground().ToString()
			+ ", text=" + this.GetText().ToString();
	endfunction
endclass