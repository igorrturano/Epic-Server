use uo;
use os;

include ":faccao:include/faccao";

// Returns the faccao index (1..5) the Jailor is configured to serve, or 0.
function GetJailorFaccao(npc)
	var f := Cint(GetObjProperty(npc, "nomefaccao"));
	if (f < 1 || f > FACCAO_NOMES.size())
		return 0;
	endif
	return f;
endfunction

function SetJailorFaccao(npc, idx)
	if (idx < 1 || idx > FACCAO_NOMES.size())
		return 0;
	endif
	SetObjProperty(npc, "nomefaccao", idx);
	return 1;
endfunction

// True when player belongs to the same faccao as the Jailor.
// The Jailor serves a single faccao (set via GM gump); guards of that faccao
// can issue punishments via this Jailor.
function IsGuardOf(player, jailor)
	if (!player || !jailor)
		return 0;
	endif
	var jailorFaccao := GetJailorFaccao(jailor);
	if (!jailorFaccao)
		return 0;
	endif
	var playerFaccao := Cint(GetObjProperty(player, "nomefaccao"));
	return (playerFaccao == jailorFaccao);
endfunction

// True when this player is currently jailed (has an open quota).
function IsPrisoner(player)
	if (!player)
		return 0;
	endif
	var quota := Cint(GetObjProperty(player, "#jail_quota"));
	return (quota > 0);
endfunction
