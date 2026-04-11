class IControl()
	// uninit function IControl(this, type, name, x, y);

	uninit function GetId(this);
	uninit function SetId(this, id);
	uninit function GetType(this);
	uninit function SetType(this, type);
	uninit function GetName(this);
	uninit function _SetName(this, name);
	uninit function GetTooltip(this);
	uninit function SetTooltip(this, tooltip);
	uninit function GetX(this);
	uninit function SetX(this, x);
	uninit function GetY(this);
	uninit function SetY(this, y);
	uninit function GetXOffset(this);
	uninit function SetXOffset(this, xOffset);
	uninit function GetYOffset(this);
	uninit function SetYOffset(this, yOffset);

	uninit function Draw(this, byRef gump);
	uninit function Equals(this, byRef other);
endclass