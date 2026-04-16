include "GumpLayout";

class BlueGumpLayout(GumpLayout)
	function BlueGumpLayout(this)
		Super();
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

	function PositionBorders(this)
		var borderSize := 35;
		
		this.GetBackground()
			.SetPosition(this.GetX() + borderSize, this.GetY() + borderSize)
			.Resize(this.GetWidth() - (2 * borderSize), this.GetHeight() - (2 * borderSize));

		this.GetBorder_TopLeft()
			.SetPosition(this.GetX(), this.GetY())
			.Resize(borderSize, borderSize);

		this.GetBorder_Top()
			.SetPosition(this.GetX() + borderSize, this.GetY())
			.Resize(this.GetWidth() - (2 * borderSize), borderSize);

		this.GetBorder_TopRight()
			.SetPosition(this.GetWidth() - borderSize, this.GetY())
			.Resize(borderSize, borderSize);

		this.GetBorder_Left()
			.SetPosition(this.GetX(), this.GetY() + borderSize)
			.Resize(borderSize, this.GetHeight() - (2 * borderSize));

		this.GetBorder_Right()
			.SetPosition(this.GetX() + this.GetWidth() - borderSize, this.GetY() + borderSize)
			.Resize(borderSize, this.GetHeight() - (2 * borderSize));

		this.GetBorder_BottomLeft()
			.SetPosition(this.GetX(), this.GetY() + this.GetHeight() - borderSize)
			.Resize(borderSize, borderSize);

		this.GetBorder_Bottom()
			.SetPosition(this.GetX() + borderSize, this.GetY() + this.GetHeight() - borderSize)
			.Resize(this.GetWidth() - (2 * borderSize), borderSize);

		this.GetBorder_BottomRight()
			.SetPosition(this.GetX() + this.GetWidth() - borderSize, this.GetY() + this.GetHeight() - borderSize)
			.Resize(borderSize, borderSize);
	endfunction

	function ToString(this)
		return "(BlueGumpLayout) " + Super::ToString(this);
	endfunction
endclass
