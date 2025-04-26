use uo;
use util;

function RemoveAcentos(text)
    var acentos := "áàãâäéèêëíìîïóòõôöúùûüũçÁÀÃÂÄÉÈÊËÍÌÎÏÓÒÕÔÖÚÙÛÜŨÇ";
    var semAcentos := "aaaaaeeeeiiiiooooouuuuucAAAAAEEEEIIIIOOOOOUUUUUC";

    var novoTexto := "";
    var i := 1;

    for (i := 1; i <= text.length(); i := i + 1)
        var char := text[i];
        var pos := acentos.find(char);
        if (pos > 0)
            novoTexto += semAcentos[pos];
        else
            novoTexto += char;
        endif
        Sleepms(2);//fix de runwayscript;
    endfor

    return novoTexto;
endfunction

function NormalizeString(text)
    text := Lower(RemoveAcentos(text));
    text := StrReplace(text, "_", " ");
    text := StrReplace(text, "  ", " ");
    return Trim(text);
endfunction

function updateToolTip(obj_reference)
   IncRevision(obj_reference);
endfunction

function isMaleGraphic(who)
	var male_graphics := {400, 1830, 1832, 1828, 1835};
	if (who.graphic in male_graphics)
		return 1;
	endif

	return 0;
endfunction

function isFemaleGraphic(who)
    var female_graphics := {401, 1831, 1833, 1829, 1834};
    if (who.graphic in female_graphics)
        return 1;   
    endif

    return 0;
endfunction

function join( a_array, delimiter )
  var result := "";
  var size := a_array.size();

  for i := 0 to size - 1
    result += a_array[i];
    if ( i < size - 1 )
      result += delimiter;
    endif
  endfor

  return result;
endfunction

function ChooseWings(who)
    var gump := GFCreateGump();
    GFResizePic(gump, 0, 0, 9380, 400, 500);
    GFTextMid(gump, 10, 30, 380, 2100, "Escolha Suas Asas Divinas");
    GFTextMid(gump, 10, 60, 380, 2100, "Esta escolha é permanente");
    
    var wing_types := array{0xF050, 0xF051, 0xF052, 0xF053, 0xF054, 0xF055};
    var wing_gumps:= array{0xD0A1, 0xD0A2, 0xD0A3, 0xD0A4, 0xD0A5, 0xD0A6};
    var y_pos := 100;
    var x_pos := 50;
    
    foreach graphic in wing_gumps
        GFAddButton(gump, x_pos, y_pos, 2117, 2118, GF_CLOSE_BTN, _graphic_iter);
        GFResizePic(gump, x_pos + 40, y_pos, 9350, 100, 100);
        GFGumpPic(gump, x_pos, y_pos - 40, graphic);
        
        if (_graphic_iter % 2 == 0)
            y_pos += 120;
            x_pos := 50;
        else
            x_pos := 200;
        endif
        sleepms(2);
    endforeach
    
    var input := GFSendGump(who, gump);
    if (input[0])
        return wing_types[input[0]];
    endif
    
    return 0;
endfunction