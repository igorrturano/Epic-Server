include "Container";

// TODO: criar opção pra escolher a proporção em % do tamanho da coluna/linha
class ContainerGridLayout(Container)
	var _rows := 1;
	var _columns := 1;

	function ContainerGridLayout(this, name, x, y, width, height, rows, columns)
		Super(name, x, y, width, height);
		this._type := GUMP_CONTROL::CONTAINERGRIDLAYOUT;
		this._rows := CInt(rows);
		this._columns := CInt(columns);
		
		for row := 1 to (this._rows)
			var column := array{};
			for col := 1 to (this._columns)
				var cellContainer := Container($"Cell {row}x{col}",
					CInt((col - 1) * (width / this._columns)) + this.GetX(),
					CInt((row - 1) * (height / this._rows)) + this.GetY(),
					CInt(width / this._columns),
					CInt(height / this._rows)
				);
				column.append(cellContainer);
			endfor
			this._controls.append(column);
		endfor
	endfunction

	function AddControl(this, byRef control, row, column)
		if (this._controls.size() >= this._columns * this._rows)
			return error { errortext := "ContainerGridLayout is full!" };
		endif

		var container := this._controls[row][column];
		container.AddControl(control);

		return this;
	endfunction

	function SelfCreate(this, byRef gump)
		var thisIsVisible := this._isVisible; // Salva o estado de visibilidade atual
		this._isVisible := false; // Desativa pra não sobrepor ao dos containers filhos
		foreach row in (this._controls)
			foreach container in (row)
				if (thisIsVisible) container.SetVisible(); endif // O container interno deve ter a mesma visibilidade que o grid
				container.SelfCreate(gump);
			endforeach
		endforeach

		Super::SelfCreate(this, gump);
	endfunction
endclass