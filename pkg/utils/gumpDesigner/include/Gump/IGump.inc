class IGump()
	uninit function IGump(this, parent, title, width, height, default options);
	uninit function GetParent(this);
	uninit function _GetPid(this);
	uninit function GetGump(this);
	uninit function GetGumpResult(this);
	uninit function GetTitle(this);
	uninit function GetWidth(this);
	uninit function GetHeight(this);
	uninit function GetOptions(this);

	uninit function SetParent(this, parent);
	uninit function SetTitle(this, title);
	uninit function SetWidth(this, width);
	uninit function SetHeight(this, height);
	uninit function SetOptions(this, options);
	uninit function AddOption(this, option);
	uninit function AddCustomization(this, byRef customization);
	uninit function AddControl(this, byRef control);

	uninit function Show(this);
endclass