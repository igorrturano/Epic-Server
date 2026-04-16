include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class GumpLayout(Layout)
	var borders := struct{
		topLeft,
		top,
		topRight,
		
		left,
		right,

		bottomLeft,
		bottom,
		bottomRight
	};
	var background;
	var titleLayout;
	var pageLayout;
	
	function GumpLayout(this)
		Super();
		this._SetName("GumpLayout");
		this.borders := GumpLayout::borders;
		this._InitializeTitle();
		this._InitializeBody();

		// this._InitializeStructure();
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

	function GetTitleLayout(this)
		return this.titleLayout;
	endfunction

	function SetTitleLayout(this, titleLayout)
		if (!(titleLayout is @TitleLayout))
			return InvalidParameterException("Title layout must be an TitleLayout.");
		endif

		this.titleLayout := titleLayout;

		return this;
	endfunction

	function GetPageLayout(this)
		return this.pageLayout;
	endfunction

	function SetPageLayout(this, bodyLayout)
		if (!(bodyLayout is @BodyLayout))
			return InvalidParameterException("Page layout must be an BodyLayout.");
		endif

		this.pageLayout := bodyLayout;

		return this;
	endfunction

	function SetPageBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Page background must be an ImageGump.");
		endif

		this.pageBackground := imageGump;

		return this;
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

	function _InitializeStructure(this)
		this.SetBackground(ImageGumpTiled("Background", 0, 0, 0, 0, 0));
		this.SetBorder_TopLeft(ImageGumpTiled("Top Left Border", 0, 0, 0, 0, 0));
		this.SetBorder_Top(ImageGumpTiled("Top Border", 0, 0, 0, 0, 0));
		this.SetBorder_TopRight(ImageGumpTiled("Top Right Border", 0, 0, 0, 0, 0));
		this.SetBorder_Left(ImageGumpTiled("Left Border", 0, 0, 0, 0, 0));
		this.SetBorder_Right(ImageGumpTiled("Right Border", 0, 0, 0, 0, 0));
		this.SetBorder_BottomLeft(ImageGumpTiled("Bottom Left Border", 0, 0, 0, 0, 0));
		this.SetBorder_Bottom(ImageGumpTiled("Bottom Border", 0, 0, 0, 0, 0));
		this.SetBorder_BottomRight(ImageGumpTiled("Bottom Right Border", 0, 0, 0, 0, 0));
		this.SetTitleBackground(ImageGumpTiled("Title Background", 0, 0, 0, 0, 0));
		this.SetPageBackground(ImageGumpTiled("Page Background", 0, 0, 0, 0, 0));
	endfunction

	function _InitializeTitle(this)
		this.SetTitleLayout(GumpFactory().CreateGumpTitle("Golden Layout Title"));
	endfunction

	function _InitializeBody(this)
		this.SetBodyLayout(GumpFactory().CreateGumpBody());
	endfunction

	// uninit function PositionControls(this);
	// uninit function PositionTitle(this);
	// uninit function PositionBody(this);
endclass
