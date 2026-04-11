include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class GumpBody(Container)
	var _background;
	
	function GumpBody(this)
		Super("Gump Body Area", 0, 0, 0, 0);
		this.SetBackground(ImageGump("Gump Body Background", 0, 0, 0, 0));
	endfunction

	function GetBackground(this)
		return this._background;
	endfunction

	function SetBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Body background must be of type ImageGump.");
		endif
		
		this._background := imageGump;

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