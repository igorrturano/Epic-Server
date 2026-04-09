class IItemWrapper()
	uninit function IItemWrapper(this, byRef itemRef);

	uninit function GetHP(this);
	uninit function SetHP(this, hp);
	uninit function GetMaxHP(this);
	uninit function GetOwner(this);
endclass