include "GumpLayout";

include ":gumpDesigner:factories/GumpFactory";

class GoldenGumpLayout(GumpLayout)
	function GoldenGumpLayout(this)
		Super();
		this._InitializeElements();
		this._InsertControls();
	endfunction

	function _InitializeElements(this)
		var factory := GumpFactory();
		this.SetBorder_TopLeft(factory.CreateImageGumpTiled()._SetName("Top Left Border").SetImageId(39925));
		this.SetBorder_Top(factory.CreateImageGumpTiled()._SetName("Top Border").SetImageId(39926));
		this.SetBorder_TopRight(factory.CreateImageGumpTiled()._SetName("Top Right Border").SetImageId(39927));
		this.SetBorder_Left(factory.CreateImageGumpTiled()._SetName("Left Border").SetImageId(39928));
		this.SetBorder_Right(factory.CreateImageGumpTiled()._SetName("Right Border").SetImageId(39930));
		this.SetBorder_BottomLeft(factory.CreateImageGumpTiled()._SetName("Bottom Left Border").SetImageId(39931));
		this.SetBorder_Bottom(factory.CreateImageGumpTiled()._SetName("Bottom Border").SetImageId(39932));
		this.SetBorder_BottomRight(factory.CreateImageGumpTiled()._SetName("Bottom Right Border").SetImageId(39933));
		this.SetBackground(factory.CreateImageGumpTiled()._SetName("Background").SetImageId(39929));
		this.Set
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

	function PositionControls(this)
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

		this.PositionTitle();
		this.PositionBody();
	endfunction
	
	/**
	 * Posiciona e redimensiona o título do layout.
	 * 
	 * Define a posição e dimensões do layout de título com valores pré-configurados:
	 * - Posição X: 25 pixels da borda esquerda
	 * - Posição Y: 15 pixels da borda superior
	 * - Altura: 40 pixels
	 * - Largura: calculada automaticamente para manter margens simétricas
	 * 
	 * @param this Referência à instância atual da classe
	 * 
	 * @return void
	 */
	function PositionTitle(this)
		var title := this.GetTitleLayout();
		title.SetX(25);
		title.SetY(15);
		title.SetHeight(40);
		title.SetWidth(this.GetWidth() - (2 * title.GetX()));
	endfunction

	/**
	 * Posiciona e redimensiona o corpo do layout em relação ao título.
	 * 
	 * Este método calcula automaticamente a posição e dimensões do corpo (body)
	 * levando em consideração a posição e tamanho do layout de título, com um
	 * separador visual entre eles.
	 * 
	 * @param this Referência à instância atual da classe
	 * 
	 * @return void
	 */
	function PositionBody(this)
		var separator := 5;
		var body := this.GetBodyLayout();
		body.SetX(this.GetTitleLayout().GetX());
		body.SetY(this.GetTitleLayout().GetY() + this.GetTitleLayout().GetHeight() + separator);
		body.SetWidth(this.GetTitleLayout().GetWidth());
		body.SetHeight(this.GetHeight() - this.GetTitleLayout().GetY() - this.GetTitleLayout().GetHeight() - (2 * separator));
	endfunction

	function ToString(this)
		return "(GoldenGumpLayout) " + Super::ToString(this);
	endfunction
endclass
