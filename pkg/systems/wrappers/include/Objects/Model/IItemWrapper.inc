class IItemWrapper()
	uninit function IItemWrapper(this, byRef itemRef);

	uninit function GetCurrentHP(this);
	uninit function SetCurrentHP(this, hp);
	uninit function GetMaxHP(this);
endclass