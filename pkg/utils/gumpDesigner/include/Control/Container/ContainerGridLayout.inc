include "Container";

class ContainerGridLayout(Container)
	var _columns;
	var _rows;
	var _isVisible;

	function ContainerGridLayout(this, name, x, y, width, height, columns, rows)
		Super(name, x, y, width, height);
		this._columns := columns;
		this._rows := rows;
	endfunction
endclass