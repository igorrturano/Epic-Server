include "Container";
include "system/exception/ParameterException/OutOfRangeException";

// TODO: criar opção pra escolher a proporção em % do tamanho da coluna/linha
class ContainerGridLayout(Container)
	var rows := 1;
	var columns := 1;
	var containers := array{ array{} }; // Matriz que armazena os containers internos, organizados por linha e coluna

	function ContainerGridLayout(this, name, x, y, width, height, rows, columns)
		Super(name, x, y, width, height);
		this.SetType(GumpControl::ContainerGridLayout);
		this.SetRows(rows);
		this.SetColumns(columns);
	endfunction

	function GetRows(this)
		return this.rows;
	endfunction

	function SetRows(this, rows)
		this.rows := CInt(rows);

		if (this.GetRows() > 0 && this.GetColumns() > 0)
			this._CreateContainers();
		endif

		return this;
	endfunction

	function GetColumns(this)
		return this.columns;
	endfunction

	function SetColumns(this, columns)
		this.columns := CInt(columns);
		
		if (this.GetRows() > 0 && this.GetColumns() > 0)
			this._CreateContainers();
		endif

		return this;
	endfunction

	function SetColumnWidth(this, column, width)
		if (column < 1 || column > this.GetColumns())
			return OutOfRangeException("Column number out of range.");
		endif

		// TODO: Ajustar largura da columa informada
		// TODO: Ajustar coluna da última coluna para preencher o restante do espaço, caso a largura total das colunas seja menor que a largura do container

		return this;
	endfunction

	function SetRowHeight(this, row, height)
		if (row < 1 || row > this.GetRows())
			return OutOfRangeException("Row number out of range.");
		endif

		// TODO: Ajustar altura da linha informada
		// TODO: Ajustar altura da última linha para preencher o restante do espaço, caso a altura total das linhas seja menor que a altura do container

		return this;
	endfunction

	// Override
	function SetVisible(this, isVisible)
		Super::SetVisible(this, isVisible);

		foreach row in (this.containers)
			foreach container in (row)
				container.SetVisible(isVisible);
				SleepMS(2);
			endforeach
		endforeach

		return this;
	endfunction

	// Override
	function GetControls(this)
		var controls := array{};

		for row := 1 to (this.containers.size())
			for column := 1 to (this.containers[row].size())
				var containerControls := this.containers[row][column].GetControls();

				foreach control in (containerControls)
					controls.append(control);
				endforeach
			endfor
		endfor

		return controls;
	endfunction

	// Override
	function AddControl(this, control, row, column)
		if (!(control is @Control))
			return InvalidParameterException("Control must be of type 'Control'.");
		endif
		
		if (row < 1 || row > this.GetRows() || column < 1 || column > this.GetColumns())
			return OutOfRangeException("Row or column number out of range.");
		endif

		this.containers[row][column].AddControl(control);

		return this;
	endfunction

	// Override
	function HowToDraw(this, byRef gump)
		foreach row in (this.containers)
			foreach container in (row)
				container.SetXOffset(this.GetX() + this.GetXOffset());
				container.SetYOffset(this.GetY() + this.GetYOffset());
				container.Draw(gump);
				SleepMS(2);
			endforeach
		endforeach
	endfunction

	// Override
	function Dispose(this)
		foreach row in (this.containers)
			foreach control in (row)
				control.Dispose();
				SleepMS(2);
			endforeach
		endforeach

		this._controls := array{};
		this.containers := ContainerGridLayout::containers;
	endfunction

	function _CreateContainers(this)
		this.containers := ContainerGridLayout::containers; // Reseta a matriz de containers
		
		for row := 1 to (this.GetRows())
			for column := 1 to (this.GetColumns())
				var cellContainer := Container($"Cell {row}x{column}",
					CInt((column - 1) * (this.GetWidth() / this.GetColumns())) + this.GetX() + this.GetXOffset(),
					CInt((row - 1) * (this.GetHeight() / this.GetRows())) + this.GetY() + this.GetYOffset(),
					CInt(this.GetWidth() / this.GetColumns()),
					CInt(this.GetHeight() / this.GetRows())
				);

				this.containers[row].append(cellContainer);
				SleepMS(2);
			endfor
		endfor
	endfunction

	function ToString(this)
		return "(ContainerGridLayout) " + Super::ToString(this)
			+ ", rows=" + this.GetRows()
			+ ", columns=" + this.GetColumns()
			+ ", containers=" + this.containers;
	endfunction
endclass