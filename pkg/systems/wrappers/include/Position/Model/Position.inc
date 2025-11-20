class Position()
	var _x := 0;
	var _y := 0;
	var _z := 0;
	var _realm := "";

	function Position(this, x, y, z, realm)
		this._x := x;
		this._y := y;
		this._z := z;
		this._realm := realm;
	endfunction
endclass