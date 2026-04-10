include "Container";

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

	function AddControl(this, byRef control, row, column)
		this._containers[row][column].AddControl(control);

		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		foreach row in (this._containers)
			foreach container in (row)
				container.SetVisible(this._isVisible); // O container interno deve ter a mesma visibilidade que o grid
				container.SelfCreate(gump);
				sleepms(2);
			endforeach
		endforeach
	endfunction

	function Dispose(this)
		foreach row in (this._containers)
			foreach control in (row)
				control.Dispose();
				sleepms(2);
			endforeach
		endforeach

		this._controls := array{};
		this._AddContainers(); // Recria os containers internos após o descarte
	endfunction

	function _AddContainers(this)
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