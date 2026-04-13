include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class GumpBody(Container)
	function GumpBody(this)
		Super("Gump Body Area", 0, 0, 0, 0);
		this.AddControl(ImageGump("Gump Body Background", 0, 0, 0, 0));
	endfunction

	function GetBackground(this)
		return this.GetControls()[1];
	endfunction

	function SetBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Body background must be of type ImageGump.");
		endif
		
		this.GetControls()[1] := imageGump;

		return this;
	endfunction

	function SetWidth(this, width)
		this.GetBackground().SetWidth(width);
		Super::SetWidth(this, width);

		return this;
	endfunction

	function SetHeight(this, height)
		this.GetBackground().SetHeight(height);
		Super::SetHeight(this, height);

		return this;
	endfunction
endclass