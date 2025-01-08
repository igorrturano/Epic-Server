use uo;
use util;
use Math;

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

function DistribuicaoGaussianaNormalizada(valoresEixoX,pico,declinio,multiplicadorFinal := 100)

	var gaussianas := array; 
	var total := 0;
	foreach x in valoresEixoX
		var gaussiana := CalcularGaussiana(x,pico,declinio);
		total += gaussiana;
		gaussianas.append(gaussiana);
		Sleepms(2);
	endforeach

	var normalizados := array;
	//normaliza os valores para que a somatória dos valores de y(chance) ao longo do eixo x, não ultrapasse 100.
	foreach gaussiana in gaussianas
		normalizados.append(Floor((gaussiana/total) * multiplicadorFinal)); 
		Sleepms(2);
	endforeach;
	
	return normalizados;

endfunction

function CalcularGaussiana(x,pico,declinio)
	return 100 * Pow(ConstE(),(-declinio * Pow(x - pico, 2)));
endfunction