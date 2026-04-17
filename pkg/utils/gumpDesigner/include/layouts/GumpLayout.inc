include "system/exception/ParameterException/InvalidParameterException";

include "Layout";
include ":gumpDesigner:factories/GumpFactory";

class GumpLayout(Layout)
	var titleLayout;
	var pageLayout;
	
	function GumpLayout(this)
		Super();
		this._SetName("GumpLayout");
	endfunction

	function GetTitleLayout(this)
		return this.titleLayout;
	endfunction

	function SetTitleLayout(this, titleLayout)
		if (!(titleLayout is @TitleLayout))
			return InvalidParameterException("Title layout must be of type TitleLayout.");
		endif
		
		this.titleLayout := titleLayout;

		return this;
	endfunction

	function GetPageLayout(this)
		return this.pageLayout;
	endfunction

	function SetPageLayout(this, pageLayout)
		if (!(pageLayout is @BodyLayout))
			return InvalidParameterException("Page layout must be of type PageLayout.");
		endif
		
		this.pageLayout := pageLayout;

		return this;
	endfunction

	function Resize(this, width, height)
		Super::Resize(this, width, height);
		this.PositionControls();
		
		return this;
	endfunction

	// Se for modificar a estrutura do gump, dar override na classe filha
	function InitializeControls(this)
		var factory := GumpFactory();
		this.SetBorder_TopLeft(factory.CreateImageGumpTiled()._SetName("Top Left Border"));
		this.SetBorder_Top(factory.CreateImageGumpTiled()._SetName("Top Border"));
		this.SetBorder_TopRight(factory.CreateImageGumpTiled()._SetName("Top Right Border"));
		this.SetBorder_Left(factory.CreateImageGumpTiled()._SetName("Left Border"));
		this.SetBorder_Right(factory.CreateImageGumpTiled()._SetName("Right Border"));
		this.SetBorder_BottomLeft(factory.CreateImageGumpTiled()._SetName("Bottom Left Border"));
		this.SetBorder_Bottom(factory.CreateImageGumpTiled()._SetName("Bottom Border"));
		this.SetBorder_BottomRight(factory.CreateImageGumpTiled()._SetName("Bottom Right Border"));
		this.SetBackground(factory.CreateImageGumpTiled()._SetName("Background"));
	endfunction
	
	function InsertControls(this)
		Super::InsertControls(this);

		if (this.GetTitleLayout())
			this.AddControl(this.GetTitleLayout());
		endif

		if (this.GetPageLayout())
			this.AddControl(this.GetPageLayout());
		endif
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

	// uninit function PositionTitle(this);
	// uninit function PositionBody(this);
endclass
