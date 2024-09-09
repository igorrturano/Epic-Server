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

function isMale(who)
	var male_graphics := {400, 1830, 1832, 1828};
	if (who.graphic in male_graphics)
		return 1;
	endif

	return 0;
endfunction

function isFemale(who)
    var female_graphics := {401, 1831, 1833, 1829};
    if (who.graphic in female_graphics)
        return 1;   
    endif

    return 0;
endfunction