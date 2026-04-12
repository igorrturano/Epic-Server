include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class GumpLayout(Container)
	var borders := struct{
		topLeft := 0,
		top := 0,
		topRight := 0,
		
		left := 0,
		right := 0,

		bottomLeft := 0,
		bottom := 0,
		bottomRight := 0
	};
	var background := 0;
	
	function GumpLayout(this, width, height)
		Super("Gump Layout", 0, 0, width, height);
		this.borders := GumpLayout::borders;
		this.background := GumpLayout::background;
		this._InitializeStructure();
	endfunction

	function GetBorder_TopLeft(this)
		return this.borders.topLeft;
	endfunction

	function SetBorder_TopLeft(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top Left must be an ImageGump.");
		endif

		this.borders.topLeft := imageGump;

		return this;
	endfunction

	function GetBorder_Top(this)
		return this.borders.top;
	endfunction

	function SetBorder_Top(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top must be an ImageGump.");
		endif

		this.borders.top := imageGump;
	endfunction

	function GetBorder_TopRight(this)
		return this.borders.topRight;
	endfunction

	function SetBorder_TopRight(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top Right must be an ImageGump.");
		endif

		this.borders.topRight := imageGump;
	endfunction

	function GetBorder_Left(this)
		return this.borders.left;
	endfunction

	function SetBorder_Left(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Left must be an ImageGump.");
		endif

		this.borders.left := imageGump;
	endfunction

	function GetBorder_Right(this)
		return this.borders.right;
	endfunction

	function SetBorder_Right(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Right must be an ImageGump.");
		endif

		this.borders.right := imageGump;
	endfunction

	function GetBorder_BottomLeft(this)
		return this.borders.bottomLeft;
	endfunction

	function SetBorder_BottomLeft(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom Left must be an ImageGump.");
		endif

		this.borders.bottomLeft := imageGump;
	endfunction

	function GetBorder_Bottom(this)
		return this.borders.bottom;
	endfunction

	function SetBorder_Bottom(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom must be an ImageGump.");
		endif

		this.borders.bottom := imageGump;
	endfunction

	function GetBorder_BottomRight(this)
		return this.borders.bottomRight;
	endfunction

	function SetBorder_BottomRight(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom Right must be an ImageGump.");
		endif

		this.borders.bottomRight := imageGump;
	endfunction

	function GetBackground(this)
		return this.background;
	endfunction
	
	function SetBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Background must be an ImageGump.");
		endif

		this.background := imageGump;

		return this;
	endfunction

	function _InitializeStructure(this)
		this.SetBackground(ImageGump("Background", 0, 0, 0, this.GetWidth(), this.GetHeight()));

		this.SetBorder_TopLeft(ImageGump("Top Left Border", 0, 0, 0, 35, 35));
		this.SetBorder_Top(ImageGump("Top Border", this.GetX() + 25, 0, 0, this.GetWidth() - 41, 35));
		this.SetBorder_TopRight(ImageGump("Top Right Border", this.GetWidth() - 35, 0, 0, 35, 35));

		this.SetBorder_Left(ImageGump("Left Border", 0, 35, 0, 35, this.GetHeight() - 70));
		this.SetBorder_Right(ImageGump("Right Border", this.GetWidth() - 35, 35, 0, 35, this.GetHeight() - 70));

		this.SetBorder_BottomLeft(ImageGump("Bottom Left Border", 0, this.GetHeight() - 35, 0, 35, 35));
		this.SetBorder_Bottom(ImageGump("Bottom Border", 35, this.GetHeight() - 35, 0, this.GetWidth() - 70, 35));
		this.SetBorder_BottomRight(ImageGump("Bottom Right Border", this.GetWidth() - 35, this.GetHeight() - 35, 0, 35, 35));
	endfunction

	function ToString(this)
		return "(GumpLayout) " + Super::ToString(this)
			+ " width=" + this.GetWidth()
			+ ", height=" + this.GetHeight()
			+ ", background=" + this.GetBackground().ToString()
			+ ", borders={" 
				+ "topLeft=" + this.GetBorder_TopLeft().ToString() + ", "
				+ "top=" + this.GetBorder_Top().ToString() + ", "
				+ "topRight=" + this.GetBorder_TopRight().ToString() + ", "
				+ "left=" + this.GetBorder_Left().ToString() + ", "
				+ "right=" + this.GetBorder_Right().ToString() + ", "
				+ "bottomLeft=" + this.GetBorder_BottomLeft().ToString() + ", "
				+ "bottom=" + this.GetBorder_Bottom().ToString() + ", "
				+ "bottomRight=" + this.GetBorder_BottomRight().ToString()
			+ "}";
	endfunction
endclass
