include "GumpLayout";
include "PaperTitleLayout";
include "PaperBodyLayout";

include ":gumpDesigner:factories/GumpFactory";

class GoldenGumpLayout(GumpLayout)
	function GoldenGumpLayout(this)
		Super();
		this.InitializeControls();
		this.ConfigureControls();
		this.InsertControls();
	endfunction

	function ConfigureControls(this)
		this.GetBorder_TopLeft().SetImageId(39925);
		this.GetBorder_Top().SetImageId(39926);
		this.GetBorder_TopRight().SetImageId(39927);
		this.GetBorder_Left().SetImageId(39928);
		this.GetBorder_Right().SetImageId(39930);
		this.GetBorder_BottomLeft().SetImageId(39931);
		this.GetBorder_Bottom().SetImageId(39932);
		this.GetBorder_BottomRight().SetImageId(39933);
		this.GetBackground().SetImageId(39929);
	endfunction

	function InitializeControls(this)
		Super::InitializeControls(this);
		this.SetTitleLayout(PaperTitleLayout());
		this.SetPageLayout(PaperBodyLayout());
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

		title.SetX(20);
		title.SetY(27);
		title.Resize(this.GetWidth() - (2 * title.GetX()), 40);
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
		var title := this.GetTitleLayout();
		var body := this.GetBodyLayout();
		var separator := 5;

		body.SetX(title.GetX());
		body.SetY(title.GetY() + title.GetHeight() + separator);
		body.Resize(this.GetWidth() - (2 * body.GetX()), this.GetHeight() - title.GetY() - title.GetHeight() - (2 * separator));
	endfunction

	function ToString(this)
		return "(GoldenGumpLayout) " + Super::ToString(this);
	endfunction
endclass
