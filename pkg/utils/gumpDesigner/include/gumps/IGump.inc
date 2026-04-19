class IGump()
	uninit function IGump(this, parent, title, width, height, default options);
	
	uninit function GetParent(this);
	uninit function SetParent(this, parent);
	uninit function _GetPid(this);
	uninit function SetPid(this, pid);
	uninit function GetGumpString(this);
	uninit function GetTitleName(this);
	uninit function SetTitleName(this, titleName);
	uninit function GetTitle(this);
	uninit function SetTitle(this, title);
	uninit function GetWidth(this);
	uninit function SetWidth(this, width);
	uninit function GetHeight(this);
	uninit function SetHeight(this, height);
	uninit function GetLayout(this);
	uninit function SetLayout(this, layout);
	uninit function GetPages(this);
	uninit function GetPage(this, pageNumber);
	uninit function SetPage(this, pageNumber, page);
	uninit function AddPage(this, page);
	uninit function GoToPage(this, pageNumber);
	uninit function GetCurrentPage(this);
	uninit function GetOptions(this);
	uninit function AddOption(this, option);
	
	uninit function SetHelpAction(this, byRef helpAction);
	// uninit function GetControls(this);
	// uninit function AddControl(this, control);

	uninit function Show(this);
endclass