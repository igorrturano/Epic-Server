// include ":gumps:gumps_ex";
include ":gumpDesigner:controls/Text";
include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";
include ":gumpDesigner:controls/ImageGumpTiled";

class DiagFactory()
	var gump;

	function DiagFactory(this, byRef gump)
		this.gump := gump;
	endfunction

	function MountTitle(this)
		print("Mounting title...");
		var options := this.gump.GetOptions();
		
		if (GumpOptions::HideTitle in options)
			return;
		endif

		var container := Container("Title Container", 10, 10, this.gump.GetWidth() - 20, 35);
		var bg := ImageGump("Title Background", 0, 0, 9350, container.GetWidth(), container.GetHeight());
		var title := Text("Title Text", 0, 0, this.gump.GetTitle(), TEXT_ALIGN::ALIGN_CENTER);

		container.AddControl(bg);
		container.AddControl(title);
		this.gump.AddControl(container);

		return this;
	endfunction

	function MountBody(this)
		print("Mounting body...");
		var options := this.gump.GetOptions();
		
		if (GumpOptions::HidePage in options)
			return;
		endif

		var container := Container("Body Container", 10, 50, this.gump.GetWidth() - 20, this.gump.GetHeight() - 60);
		var body := ImageGump("Body Background", 10, 50, 9350, this.container.GetWidth(), this.container.GetHeight());
		container.AddControl(body);
		this.gump.AddControl(container);

		return this;
	endfunction

	function MountStyle(this)
		print("Mounting style...");
		var width := this.gump.GetWidth();
		var height := this.gump.GetHeight();
		var options := this.gump.GetOptions();
		
		var molduras := array{};
		var background := 0;
		var border := GumpStyle::DefaultBorder;

		if (GumpStyle::Golden in options)
			border := GumpStyle::Golden;
		elseif (GumpStyle::Marble in options)
			border := GumpStyle::Marble;
		elseif (GumpStyle::Blue in options)
			border := GumpStyle::Blue;
		endif

		if (border == GumpStyle::DefaultBorder) // Separado pra facilitar a troca do default
			border := GumpStyle::Golden;
		endif

		if (border == GumpStyle::Golden)
			background := 39929;
			molduras := array{
				// SIDES
				struct{ x := 0, y := 15, width := 35, height := height - 15, img := 39928 }, // LEFT
				struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 39930 }, // RIGHT

				// TOP
				struct{ x := 25, y := 0, width := width - 41, height := 35, img := 39926 }, // TOPMIDDLE
				struct{ x := 0, y := 0, width := 35, height := 35, img := 39925 }, // TOPLEFT
				struct{ x := width - 25, y := 0, width := 35, height := 35, img := 39927 }, // TOPRIGHT

				// BOTTOM
				struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 39932 }, // BOTTOMMIDDLE
				struct{ x := 0, y := height - 25, width := 35, height := 35, img := 39931 }, // BOTTOMLEFT
				struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 39933 } // BOTTOMRIGHT
			};
		elseif (border == GumpStyle::Blue)
			background := 40004;
			molduras := array{
				// SIDES
				struct{ x := 0, y := 15, width := 35, height := height - 15, img := 40003 }, // LEFT
				struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 40005 }, // RIGHT

				// TOP
				struct{ x := 25, y := 0, width := width - 41, height := 15, img := 40001 }, // TOPMIDDLE
				struct{ x := 0, y := 0, width := 35, height := 15, img := 40000 }, // TOPLEFT
				struct{ x := width - 25, y := 0, width := 35, height := 15, img := 40002 }, // TOPRIGHT

				// BOTTOM
				struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 40007 }, // BOTTOMMIDDLE
				struct{ x := 0, y := height - 25, width := 35, height := 35, img := 40006 }, // BOTTOMLEFT
				struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 40008 } // BOTTOMRIGHT
			};
		elseif (border == GumpStyle::Marble)
			background := 3504;
			molduras := array{
				// SIDES
				struct{ x := 0, y := 15, width := 35, height := height - 15, img := 3503 }, // LEFT
				struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 3505 }, // RIGHT

				// TOP
				struct{ x := 25, y := 0, width := width - 41, height := 15, img := 3501 }, // TOPMIDDLE
				struct{ x := 0, y := 0, width := 35, height := 15, img := 3500 }, // TOPLEFT
				struct{ x := width - 25, y := 0, width := 35, height := 15, img := 3502 }, // TOPRIGHT

				// BOTTOM
				struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 3507 }, // BOTTOMMIDDLE
				struct{ x := 0, y := height - 25, width := 35, height := 35, img := 3506 }, // BOTTOMLEFT
				struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 3508 } // BOTTOMRIGHT
			};
		endif

		var container := Container("Gump Container", 0, 0, this.gump.GetWidth(), this.gump.GetHeight());
		var bg := ImageGump("Gump Background", 0, 0, background, container.GetWidth(), container.GetHeight());
		container.AddControl(bg);

		foreach moldura in molduras
			var img := ImageGump("Moldura " + CStr(moldura.img), moldura.x, moldura.y, moldura.img, moldura.width, moldura.height);
			container.AddControl(img);
		endforeach

		this.gump.AddControl(container);

		return this;
	endfunction

	function CreateGump(this, width, height, x := 0, y := 0)
		this.gumpStr := GFCreateGump(x, y, width, height);
	endfunction
endclass

enum class GumpOptions
	NoClosable := "option_noclosable",
	NoMovable := "option_nomovable",
	HideBorder := "option_hideborder",
	HideTitle := "option_hidetitle",
	HidePage := "option_hidepage"
endenum


