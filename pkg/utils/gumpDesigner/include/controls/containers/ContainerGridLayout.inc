include "Container";
include ":gumpDesigner:exceptions/WrongSizeException";

// TODO: criar opção pra escolher a proporção em % do tamanho da coluna/linha
class ContainerGridLayout(Container)
	var _rows := 1;
	var _columns := 1;
	var _containers := array{ array{} }; // Matriz que armazena os containers internos, organizados por linha e coluna

	function ContainerGridLayout(this, name, x, y, width, height, rows, columns)
		Super(name, x, y, width, height);
		this._type := GumpControl::ContainerGridLayout;
		this._rows := CInt(rows);
		this._columns := CInt(columns);
		this._containers := array{ array{} };
		
		this._AddContainers();
	endfunction

	function GetRows(this)
		return this._rows;
	endfunction

	function GetColumns(this)
		return this._columns;
	endfunction

	// TODO: Revisar essa função depois, deve ter algum erro, fiz nas pressas
	function SetColumnWidth(this, column, width)
		if (column < 1 || column > this._columns)
			return InvalidParameterException("Column number out of range.");
		endif

		for row := 1 to (this._rows)
			this._containers[row][column].SetWidth(width);
			SleepMS(2);
		endfor

		var remainingWidth := this.GetWidth(); // Largura restante para as outras colunas
		for col := 1 to (this._columns)
			remainingWidth -= this._containers[1][col].GetWidth();
			if (remainingWidth < 0)
				return WrongSizeException("Total column width exceeds container width.");
			endif
		endfor

		this.SetColumnWidth(this._columns, remainingWidth); // Ajusta a última coluna para preencher o espaço restante

		return this;
	endfunction

	
	// TODO: Revisar essa função depois, deve ter algum erro, fiz nas pressas
	function SetRowHeight(this, row, height)
		if (row < 1 || row > this._rows)
			return InvalidParameterException("Row number out of range.");
		endif

		for column := 1 to (this._columns)
			this._containers[row][column].SetHeight(height);
			SleepMS(2);
		endfor

		var remainingHeight := this.GetHeight();
		for currentRow := 1 to (this._rows)
			remainingHeight -= this._containers[currentRow][1].GetHeight();
			if (remainingHeight < 0)
				return WrongSizeException("Total row height exceeds container height.");
			endif
		endfor

		for col := 1 to (this._columns)
			this._containers[this._rows][col].SetHeight(remainingHeight);
			SleepMS(2);
		endfor

		return this;
	endfunction

	// Override
	function SetVisible(this, isVisible)
		Super::SetVisible(this, isVisible);

		foreach row in (this._containers)
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

		for row := 1 to (this._containers.size())
			for column := 1 to (this._containers[row].size())
				var containerControls := this._containers[row][column].GetControls();

				foreach control in (containerControls)
					controls.append(control);
				endforeach
			endfor
		endfor

		return controls;
	endfunction

	// Override
	function AddControl(this, control, row, column)
		this._containers[row][column].AddControl(control);

		return this;
	endfunction

	// Override
	function Draw(this, byRef gump)
		foreach row in (this._containers)
			foreach container in (row)
				container.Draw(gump);
				SleepMS(2);
			endforeach
		endforeach
	endfunction

	// Override
	function Dispose(this)
		foreach row in (this._containers)
			foreach control in (row)
				control.Dispose();
				SleepMS(2);
			endforeach
		endforeach

		this._controls := array{};
		this._CreateContainers(); // Recria os containers internos após o descarte
	endfunction

	function _CreateContainers(this)
		this._containers := array{ array{} };
		
		for row := 1 to (this._rows)
			for column := 1 to (this._columns)
				var cellContainer := Container($"Cell {row}x{column}",
					CInt((column - 1) * (this._width / this._columns)) + this.GetX(),
					CInt((row - 1) * (this._height / this._rows)) + this.GetY(),
					CInt(this._width / this._columns),
					CInt(this._height / this._rows)
				);

				this._containers[row].append(cellContainer);
			endfor
		endfor
	endfunction

	function ToString(this)
		return "(ContainerGridLayout) " + Super::ToString(this)
			+ ", rows=" + this.GetRows()
			+ ", columns=" + this.GetColumns()
			+ ", controls=" + this.GetControls().size();
	endfunction
endclass