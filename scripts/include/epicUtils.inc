use uo;

// Função auxiliar para remover acentos
function RemoveAcentos(texto)
    var acentos := "áàâãäéèêëíìîïóòôõöúùûüçÁÀÂÃÄÉÈÊËÍÌÎÏÓÒÔÕÖÚÙÛÜÇ";
    var semAcentos := "aaaaaeeeeiiiiooooouuuucAAAAAEEEEIIIIOOOOOUUUUC";
    var resultado := "";

    foreach caractere in texto
        var pos := Find(acentos, caractere, 1);
        if (pos > 0)
            resultado += semAcentos[pos];
        else
            resultado += caractere;
        endif
    endforeach

    return resultado;
endfunction
