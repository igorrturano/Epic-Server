include "system/exception/ParameterException/InvalidParameterException";

include ":gumpDesigner:controls/containers/Container";
include ":gumpDesigner:controls/ImageGump";

class GumpLayout(Container)
	var layoutType := LayoutType::DefaultBorder;
	var borders := struct{ // É um struct porque a ordem afeta a sobreposição das imagens.
		sides := array{},
		top := array{},
		bottom := array{}
	};
	var bgImg := 0;
	
	function GumpLayout(this, layoutType := LayoutType::DefaultBorder)
		Super("Gump Layout", 0, 0, 0, 0);
		this.layoutType := layoutType;
		this.borders := GumpLayout::borders;
		this.bgImg := GumpLayout::bgImg;

		this._InitializeLayout();
	endfunction

	function SetLayout(this, layoutType)
		this.layoutType := layoutType;
		
		return this;
	endfunction

	function SetBorder_TopLeft(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top Left must be an ImageGump.");
		endif

		this.borders.top[1] := imageGump;

		return this;
	endfunction

	function SetBorder_Top(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top must be an ImageGump.");
		endif

		this.borders.top[2] := imageGump;
	endfunction

	function SetBorder_TopRight(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Top Right must be an ImageGump.");
		endif

		this.borders.top[3] := imageGump;
	endfunction

	function SetBorder_Right(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Right must be an ImageGump.");
		endif

		this.borders.sides[2] := imageGump;
	endfunction

	function SetBorder_BottomRight(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom Right must be an ImageGump.");
		endif

		this.borders.bottom[3] := imageGump;
	endfunction

	function SetBorder_Bottom(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom must be an ImageGump.");
		endif

		this.borders.bottom[2] := imageGump;
	endfunction

	function SetBorder_BottomLeft(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Bottom Left must be an ImageGump.");
		endif

		this.borders.bottom[1] := imageGump;
	endfunction

	function SetBorder_Left(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Border Left must be an ImageGump.");
		endif

		this.borders.sides[1] := imageGump;
	endfunction

	function SetBackground(this, imageGump)
		if (!(imageGump is @ImageGump))
			return InvalidParameterException("Background must be an ImageGump.");
		endif

		this.bgImg := imageGump;
	endfunction

	function _SetLayout(this)
		if (this.layoutType == LayoutType::DefaultBorder)
			this.layoutType := LayoutType::Golden; // Default é Golden
		endif
		
		var borderTopLeft := ImageGump("Top Left Border", 0, 0, 0);
		var borderTop := ImageGump("Top Border", 0, 0, 0);
		var borderTopRight := ImageGump("Top Right Border", 0, 0, 0);
		var borderRight := ImageGump("Right Border", 0, 0, 0);
		var borderBottomRight := ImageGump("Bottom Right Border", 0, 0, 0);
		var borderBottom := ImageGump("Bottom Border", 0, 0, 0);
		var borderBottomLeft := ImageGump("Bottom Left Border", 0, 0, 0);
		var borderLeft := ImageGump("Left Border", 0, 0, 0);
		var background := ImageGump("Background", 0, 0, 0);
		
		case (this.layoutType)
			LayoutType::Golden:
				borderTopLeft.SetImageId(39925);
				borderTop.SetImageId(39926);
				borderTopRight.SetImageId(39927);
				borderRight.SetImageId(39930);
				borderBottomRight.SetImageId(39933);
				borderBottom.SetImageId(39932);
				borderBottomLeft.SetImageId(39931);
				borderLeft.SetImageId(39928);
				background.SetImageId(39929);
				break;

			LayoutType::Blue:
				borderTopLeft.SetImageId(40000);
				borderTop.SetImageId(40001);
				borderTopRight.SetImageId(40002);
				borderRight.SetImageId(40005);
				borderBottomRight.SetImageId(40008);
				borderBottom.SetImageId(40007);
				borderBottomLeft.SetImageId(40006);
				borderLeft.SetImageId(40003);
				background.SetImageId(40004);
				break;

			LayoutType::Marble:
				borderTopLeft.SetImageId(3500);
				borderTop.SetImageId(3501);
				borderTopRight.SetImageId(3502);
				borderRight.SetImageId(3505);
				borderBottomRight.SetImageId(3508);
				borderBottom.SetImageId(3507);
				borderBottomLeft.SetImageId(3506);
				borderLeft.SetImageId(3503);
				background.SetImageId(3504);
				break;
			
			default:
				return InvalidParameterException("DefaultBorder is not a valid layout type. Please choose a specific layout.");
		endcase

		this.SetBorder_TopLeft(borderTopLeft);
		this.SetBorder_Top(borderTop);
		this.SetBorder_TopRight(borderTopRight);
		this.SetBorder_Right(borderRight);
		this.SetBorder_BottomRight(borderBottomRight);
		this.SetBorder_Bottom(borderBottom);
		this.SetBorder_BottomLeft(borderBottomLeft);
		this.SetBorder_Left(borderLeft);

		this._AddLayout();
	endfunction

	function _AddLayout(this)
		if (this.bgImg)
			this.AddControl(this.bgImg);
		endif

		foreach sideBorder in (this.borders.sides)
			if (sideBorder)
				this.AddControl(sideBorder);
			endif
		endforeach

		foreach topBorder in (this.borders.top)
			if (topBorder)
				this.AddControl(topBorder);
			endif
		endforeach

		foreach bottomBorder in (this.borders.bottom)
			if (bottomBorder)
				this.AddControl(bottomBorder);
			endif
		endforeach
	endfunction
endclass

enum class LayoutType
	DefaultBorder := "border_default",
	Golden := "border_golden",
	Blue := "border_blue",
	Marble := "border_marble"
endenum
