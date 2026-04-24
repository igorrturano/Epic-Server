class IUObjectWrapper()
	uninit function GetObjRef(this);
	uninit function GetSerial(this);
	uninit function GetObjType(this);
	uninit function GetName(this);
	uninit function GetGraphic(this);
	uninit function GetPosition(this);

	uninit function _SetName(this, name);
	uninit function SetGraphic(this, graphic);
	uninit function SetColor(this, color);

	uninit function ToString(this);
endclass