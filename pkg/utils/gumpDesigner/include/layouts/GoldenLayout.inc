include "GumpLayout";

class GoldenLayout(GumpLayout)
	function GoldenLayout(this)
		Super();
		this._InitializeLayout();
	endfunction

	function _InitializeLayout(this)
		this.GetBorder_TopLeft().SetImageId(39925);
		this.GetBorder_Top().SetImageId(39926);
		this.GetBorder_TopRight().SetImageId(39927);
		this.GetBorder_Left().SetImageId(39928);
		this.GetBorder_Right().SetImageId(39930);
		this.GetBorder_BottomLeft().SetImageId(39931);
		this.GetBorder_Bottom().SetImageId(39932);
		this.GetBorder_BottomRight().SetImageId(39933);
		this.GetBackground().SetImageId(39929);

		this._AddLayout();
	endfunction

	function _AddLayout(this)
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
