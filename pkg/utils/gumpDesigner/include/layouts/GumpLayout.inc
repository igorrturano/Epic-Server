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

	function InitializeElements(this)
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

		this.SetTitleLayout(factory.CreateTitleLayout("Layout Title"));
		this.SetPageLayout(factory.CreateBodyLayout());
	endfunction
	
	function InsertControls(this)
		Super::InsertControls(this);

		this.AddControl(this.GetTitleLayout());
		this.AddControl(this.GetPageLayout());
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

	uninit function ConfigureControls(this);
	uninit function PositionControls(this);
	uninit function PositionTitle(this);
	uninit function PositionBody(this);
endclass
