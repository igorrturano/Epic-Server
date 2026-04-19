include "IGumpFactory";

include ":gumpDesigner:gumps/Gump";
include ":gumpDesigner:controls/Button";
include ":gumpDesigner:controls/ImageGump";
include ":gumpDesigner:controls/ImageGumpTiled";
include ":gumpDesigner:controls/ImageTile";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/Tooltip";
include ":gumpDesigner:controls/containers/ContainerGridLayout";
include ":gumpDesigner:layouts/TitleLayout";
include ":gumpDesigner:layouts/PageLayout";

class GumpFactory(IGumpFactory)
	function GumpFactory(this)
	endfunction

	function CreateGump(this, parent, title, width, height, options := array{})
		return Gump(parent, title, width, height, options);
	endfunction

	function CreateButton(this, name := "Button", x := 0, y := 0, unpressedImage := BUTTON_IMAGE::CLOSE_UNPRESSED, pressedImage := BUTTON_IMAGE::CLOSE_PRESSED, buttonType := BUTTON_TYPE::CLOSE)
		return Button(name, x, y, unpressedImage, pressedImage, buttonType);
	endfunction

	function CreateImageGump(this, name := "ImageGump", x := 0, y := 0, imageId := 0)
		return ImageGump(name, x, y, imageId);
	endfunction

	function CreateImageGumpTiled(this, name := "ImageGumpTiled", x := 0, y := 0, width := 0, height := 0, imageId := 0)
		return ImageGumpTiled(name, x, y, width, height, imageId);
	endfunction

	function CreateImageTile(this, name := "ImageTile", x := 0, y := 0, imageId := 0)
		return ImageTile(name, x, y, imageId);
	endfunction

	function CreateText(this, name := "Text", x := 0, y := 0, text := "")
		return Text(name, x, y, text);
	endfunction

	function CreateTooltip(this, name := "Tooltip", text := "")
		return Tooltip(name, text);
	endfunction

	function CreateContainer(this, name := "Container", x := 0, y := 0, width := 0, height := 0)
		return Container(name, x, y, width, height);
	endfunction
	
	function CreateContainerGridLayout(this, name := "Container Grid Layout", x := 0, y := 0, width := 0, height := 0, rows := 1, columns := 1)
		return ContainerGridLayout(name, x, y, width, height, rows, columns);
	endfunction
endclass