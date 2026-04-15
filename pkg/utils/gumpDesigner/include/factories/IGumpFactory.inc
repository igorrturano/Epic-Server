class IGumpFactory()
	uninit function CreateButton(this);
	uninit function CreateImageGump(this);
	uninit function CreateImageGumpTiled(this);
	uninit function CreateImageTile(this);
	uninit function CreateText(this);
	uninit function CreateTooltip(this);
	uninit function CreateContainer(this);
	uninit function CreateContainerGridLayout(this, rows, columns);
	uninit function CreateGumpTitle(this, title);
	uninit function CreateGumpBody(this);
endclass