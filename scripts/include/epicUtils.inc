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
