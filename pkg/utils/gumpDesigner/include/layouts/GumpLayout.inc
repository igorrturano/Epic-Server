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
			+ ", titleLayout=" + (this.GetTitleLayout() ? this.GetTitleLayout().ToString() : "null")
			+ ", pageLayout=" + (this.GetPageLayout() ? this.GetPageLayout().ToString() : "null");
	endfunction

	// uninit function PositionTitle(this);
	// uninit function PositionBody(this);
endclass
