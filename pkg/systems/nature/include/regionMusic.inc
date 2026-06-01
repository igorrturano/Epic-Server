use uo;
use os;
use basic;

/*
 * regionMusic.inc
 *
 * Música ambiente por região, sensível a dia/noite.
 *
 * O config "midi N" em regions.cfg toca uma faixa fixa ao entrar e não sabe
 * a hora do dia. Para regiões que precisam de faixa diferente de dia e de
 * noite, usamos um EnterScript que chama PlayRegionMusic().
 *
 * Dia/noite vem de GetGlobalProperty("dia") ("dia" / "noite"), setado pelo
 * ciclo em :nature:daynightClock.
 *
 * Os IDs são os índices de Music/Digital/Config.txt (lado cliente).
 *
 * Dedup: guardamos a última faixa tocada em "#lastRegionMusic" para não
 * reiniciar a música ao cruzar a fronteira entre partes da mesma região
 * (ex.: nortegelado_parte1 -> nortegelado_parte2). Se o dia virar noite
 * enquanto o jogador anda, o próximo cruzamento já troca para a faixa certa.
 */
function PlayRegionMusic(who, regioname)
	if (!who)
		return 0;
	endif

	var isDay := (GetGlobalProperty("dia") == "dia");
	var trackId := 0;

	if (regioname == "Valtheris")
		// Streets of Whiterun (dia) / The City Gates (noite)
		trackId := isDay ? 124 : 123;
	elseif (Find(regioname, "nortegelado", 1) == 1)
		// Taiga: From Past to Present (dia) / Kyne's Peace (noite)
		trackId := isDay ? 109 : 120;
	endif

	if (trackId > 0)
		if (cint(GetObjProperty(who, "#lastRegionMusic")) != trackId)
			PlayMusic(who, trackId);
			SetObjProperty(who, "#lastRegionMusic", trackId);
		endif
	endif

	return trackId;
endfunction
