include "IGumpFactory";

include ":gumpDesigner:controls/Button";
include ":gumpDesigner:controls/ImageGump";
include ":gumpDesigner:controls/ImageGumpTiled";
include ":gumpDesigner:controls/ImageTile";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/Tooltip";
include ":gumpDesigner:controls/containers/ContainerGridLayout";
include ":gumpDesigner:layouts/TitleLayout";
include ":gumpDesigner:layouts/BodyLayout";

class GumpFactory(IGumpFactory)
	function GumpFactory(this)
	endfunction

	function CreateButton(this)
		return Button("Button", 0, 0);
	endfunction

	function CreateImageGump(this)
		return ImageGump("ImageGump", 0, 0, 0);
	endfunction

	function CreateImageGumpTiled(this)
		return ImageGumpTiled("ImageGumpTiled", 0, 0, 0, 0, 0);
	endfunction

	function CreateImageTile(this)
		return ImageTile("", 0, 0, 0);
	endfunction

	function CreateText(this)
		return Text("Text", 0, 0, "");
	endfunction

	function CreateTooltip(this)
		return Tooltip("Tooltip", "");
	endfunction

	function CreateContainer(this)
		return Container("Container", 0, 0, 0, 0);
	endfunction
	
	function CreateContainerGridLayout(this, rows, columns)
		return ContainerGridLayout("Container Grid Layout", 0, 0, 0, 0, rows, columns);
	endfunction

	function CreateTitleLayout(this, title := "<No text inserted>")
		return TitleLayout(title);
	endfunction

	function CreateBodyLayout(this)
		return BodyLayout();
	endfunction
endclass