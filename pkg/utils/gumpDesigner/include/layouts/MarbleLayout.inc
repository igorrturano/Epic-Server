include "GumpLayout";

class MarbleLayout(GumpLayout)
	function MarbleLayout(this, width, height)
		Super(width, height);
		this._InitializeValues();
	endfunction

	function _InitializeValues(this)
		this.GetBorder_TopLeft().SetImageId(40000);
		this.GetBorder_Top().SetImageId(40001);
		this.GetBorder_TopRight().SetImageId(40002);
		this.GetBorder_Left().SetImageId(40003);
		this.GetBorder_Right().SetImageId(40005);
		this.GetBorder_BottomLeft().SetImageId(40006);
		this.GetBorder_Bottom().SetImageId(40007);
		this.GetBorder_BottomRight().SetImageId(40008);
		this.GetBackground().SetImageId(40004);

		this._InsertControls();
	endfunction

	function _InsertControls(this)
		this.AddControl(this.GetBackground());
		this.AddControl(this.GetBorder_Left());
		this.AddControl(this.GetBorder_Right());
		this.AddControl(this.GetBorder_Top());
		this.AddControl(this.GetBorder_TopLeft());
		this.AddControl(this.GetBorder_TopRight());
		this.AddControl(this.GetBorder_Bottom());
		this.AddControl(this.GetBorder_BottomLeft());
		this.AddControl(this.GetBorder_BottomRight());
	endfunction

endclass
