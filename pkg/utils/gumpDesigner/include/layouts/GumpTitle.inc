include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/ImageGump";

class GumpTitle(Container)
	function GumpTitle(this, title := "")
		Super("Gump Title Area", 0, 0, 0, 0);

		this.AddControl(ImageGump("Gump Title Background", 0, 0, 0, 0));
		this.AddControl(Text("Gump Title Text", 0, 0, title, TEXT_ALIGN::ALIGN_CENTER));
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

	function SetWidth(this, width)
		this.GetBackground().SetWidth(width);
		this.GetText().SetWidth(width);
		Super::SetWidth(this, width);

		return this;
	endfunction

	function ToString(this)
		return "(GumpTitle) " + Super::ToString(this)
			+ ", background=" + this.GetBackground().ToString()
			+ ", text=" + this.GetText().ToString();
	endfunction
endclass