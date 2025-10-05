include "Container";

// TODO: criar opção pra escolher a proporção em % do tamanho da coluna/linha
class ContainerGridLayout(Container)
	var _maxRows := 1;
	var _maxColumns := 1;

	function ContainerGridLayout(this, name, x, y, width, height, maxRows, maxColumns)
		Super(name, x, y, width, height);
		this._type := GUMP_CONTROL::CONTAINERGRIDLAYOUT;
		this._maxRows := CInt(maxRows);
		this._maxColumns := CInt(maxColumns);
		
		for row := 1 to (this._maxRows)
			var column := array{};
			for col := 1 to (this._maxColumns)
				var cellContainer := Container($"Cell {row}x{col}",
					CInt((col - 1) * (width / this._maxColumns)) + this.GetX(),
					CInt((row - 1) * (height / this._maxRows)) + this.GetY(),
					CInt(width / this._maxColumns),
					CInt(height / this._maxRows)
				);
				column.append(cellContainer);
			endfor
			this._controls.append(column);
		endfor
	endfunction

	function AddControl(this, byRef control, row, column)
		if (this._controls.size() >= this._maxColumns * this._maxRows)
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
				container.SetVisible(thisIsVisible); // O container interno deve ter a mesma visibilidade que o grid
				container.SelfCreate(gump);
			endforeach
		endforeach

		Super::SelfCreate(this, gump);
	endfunction
endclass