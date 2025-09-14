include "Container";

class ContainerGridLayout(Container)
	var _maxRows := 1;
	var _maxColumns := 1;
	var _row := array{}; // Array de Array de Container - o de dentro é a coluna, o de fora é a linha.

	function ContainerGridLayout(this, name, x, y, width, height, maxRows, maxColumns)
		Super(name, x, y, width, height);
		this._maxRows := CInt(maxRows);
		this._maxColumns := CInt(maxColumns);
		this._row := array{};
		
		for row := 1 to (this._maxRows)
			var column := array{};
			for col := 1 to (this._maxColumns)
				print ("Creating cell " + row + "x" + col);
				var cellContainer := Container($"Cell {row}x{col}", 
					CInt((col - 1) * (width / this._maxColumns)), 
					CInt((row - 1) * (height / this._maxRows)), 
					CInt(width / this._maxColumns), 
					CInt(height / this._maxRows)
				);
				print("cell->" + cellContainer.ToString());
				column.append(cellContainer);
			endfor
			this._row.append(column);
		endfor
	endfunction

	function AddControl(this, byRef control, row, column)
		if (this._controls.size() >= this._maxColumns * this._maxRows)
			Print("Trying to add Control in a full ContainerGridLayout...");
			return error { errortext := "ContainerGridLayout is full!" };
		endif
		
		this._row[row][column].AddControl(control);
	endfunction

	function SelfCreate(this, byRef gump)
		foreach row in (this._row)
			foreach cell in (row)
				var container := cell.GetControls();
				if (container.size() < 1)
					continue; // Não tem container nessa célula. Pula pra ganhar performance.
				else
					container := container[1]; // Só pode ter 1 container em cada célula, melhor acessar direto.
				endif

				container.SetX(this.GetX() + cell.GetX() + container.GetX());
				container.SetY(this.GetY() + cell.GetY() + container.GetY());
				container.SetWidth(cell.GetWidth());
				container.SetHeight(cell.GetHeight());
				container.SelfCreate(gump);

				// Esse código abaixo tem menos performance, mas é mais genérico, permitindo múltiplos containers por célula.
				// Vou deixar ele aqui pra caso precise no futuro. Vou testar primeiro com o de cima, que é mais performático.
				// foreach container in (cell.GetControls())
					// container.SetX(this.GetX() + cell.GetX() + container.GetX());
					// container.SetY(this.GetY() + cell.GetY() + container.GetY());
					// container.SetWidth(cell.GetWidth());
					// container.SetHeight(cell.GetHeight());
					// container.SelfCreate(gump);
					// print("Criando container dentro do cell " + cell.GetName() + ": " + container.ToString());
				// endforeach
			endforeach
		endforeach

		Super::SelfCreate(this, gump);
	endfunction
endclass