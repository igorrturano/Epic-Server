include "Container";

include ":gumpDesigner:Control/Control";

class ContainerGridLayout(Container)
	var _columns;
	var _rows;
	var _isVisible;

	function ContainerGridLayout(this, parent, x, y, width, height, columns, rows)
		Super(parent, x, y, width, height);
		this._columns := columns;
		this._rows := rows;
	endfunction
endclass