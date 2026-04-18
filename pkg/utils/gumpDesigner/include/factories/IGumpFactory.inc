class IGumpFactory()
	uninit function CreateButton(this, default x, default y, default unpressedImage, default pressedImage, default buttonType);
	uninit function CreateImageGump(this, default x, default y, default imageId);
	uninit function CreateImageGumpTiled(this, default x, default y, default width, default height, default imageId);
	uninit function CreateImageTile(this, default x, default y, default imageId);
	uninit function CreateText(this, default x, default y, default text);
	uninit function CreateTooltip(this, default name, default text);
	uninit function CreateContainer(this, default x, default y, default width, default height);
	uninit function CreateContainerGridLayout(this, default name, default x, default y, default width, default height, default rows, default columns);
endclass