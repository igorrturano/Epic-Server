class IPositionWrapper()
	uninit function IPositionWrapper(this, default x, default y, default z, default realm);
	
	uninit function GetX(this);
	uninit function GetY(this);
	uninit function GetZ(this);
	uninit function GetRealm(this);
	
	uninit function SetPosition(this, x, y, z, default realm);
	uninit function SetX(this, x);
	uninit function SetY(this, y);
	uninit function SetZ(this, z);
	uninit function SetRealm(this, realm);
endclass