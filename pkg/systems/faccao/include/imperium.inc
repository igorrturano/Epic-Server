use uo;

// Limites da regiao do Imperium (Valtheris).
// Ajuste estes valores caso as coordenadas mudem no mapa.
const VALTHERIS_X1 := 1135;
const VALTHERIS_Y1 := 1020;
const VALTHERIS_X2 := 1201;
const VALTHERIS_Y2 := 1098;
const VALTHERIS_REALM := "britannia_alt";

function InValtheris(who)
	if (!who)
		return 0;
	endif
	if (who.realm != VALTHERIS_REALM)
		return 0;
	endif
	return (who.x >= VALTHERIS_X1 && who.x <= VALTHERIS_X2
	        && who.y >= VALTHERIS_Y1 && who.y <= VALTHERIS_Y2);
endfunction

function HasPraetors()
	var praetors := GetGlobalProperty("Praetors");
	if (praetors == error || !praetors)
		return 0;
	endif
	return praetors.size() > 0;
endfunction
