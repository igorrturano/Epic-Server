use uo;
use os;
use guilds;

include ":faccao:include/faccao";

// Returns the canonical faccao index (1..5) for a mobile, or 0 if none.
// Mirrors :merchants:citizen/include/citizenAuthority::GetMobileFactionIndex —
// native guild membership wins (matching FACCAO_NOMES by name), with
// nomefaccao CProp as a secondary path. The legacy "faccao" CProp is
// intentionally ignored (it holds vendor codes, not faction names).
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

function GetJailorFaccao(npc)
	return GetMobileFactionIndex(npc);
endfunction

function SetJailorFaccao(npc, idx)
	if (idx < 1 || idx > FACCAO_NOMES.size())
		return 0;
	endif
	SetObjProperty(npc, "nomefaccao", idx);
	return 1;
endfunction

// True when player belongs to the same faccao as the Jailor.
// Both sides resolved via GetMobileFactionIndex (guild OR nomefaccao CProp).
function IsGuardOf(player, jailor)
	if (!player || !jailor)
		return 0;
	endif
	var jailorFaccao := GetJailorFaccao(jailor);
	if (!jailorFaccao)
		return 0;
	endif
	var playerFaccao := GetMobileFactionIndex(player);
	return (playerFaccao == jailorFaccao);
endfunction

function IsPrisoner(player)
	if (!player)
		return 0;
	endif
	var quota := Cint(GetObjProperty(player, "#jail_quota"));
	return (quota > 0);
endfunction
