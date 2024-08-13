use uo;

// Função auxiliar para remover acentos
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
