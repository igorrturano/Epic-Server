use uo;
use os;
use guilds;

include ":faccao:include/faccao";
include ":jailor:include/jailorStore";

// Returns the canonical faccao index (1..5) for a mobile, or 0 if none.
// Mirrors :merchants:citizen/include/citizenAuthority::GetMobileFactionIndex —
// native guild membership wins (matching FACCAO_NOMES by name), with
// nomefaccao CProp as a secondary path.
function GetMobileFactionIndex(mob)
	if (!mob)
		return 0;
	endif

	if (mob.guild)
		var idx := 1;
		foreach name in (FACCAO_NOMES)
			if (mob.guild.name == name)
				return idx;
			endif
			idx := idx + 1;
			sleepms(1);
		endforeach
	endif

	var n := GetObjProperty(mob, "nomefaccao");
	if (n != error)
		var ni := CInt(n);
		if (ni >= 1 && ni <= FACCAO_NOMES.size())
			return ni;
		endif
	endif

	return 0;
endfunction

// True when player belongs to one of the faccoes this Jailor serves.
function IsGuardOf(player, jailor)
	if (!player || !jailor)
		return 0;
	endif
	var jailorFactions := GetJailorFactions(jailor);
	if (!jailorFactions || !jailorFactions.size())
		return 0;
	endif
	var playerFaccao := GetMobileFactionIndex(player);
	if (!playerFaccao)
		return 0;
	endif
	return (playerFaccao in jailorFactions);
endfunction

function IsPrisoner(player)
	if (!player)
		return 0;
	endif
	var quota := Cint(GetObjProperty(player, "#jail_quota"));
	return (quota > 0);
endfunction
