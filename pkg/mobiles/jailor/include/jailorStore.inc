// =============================================================================
// jailorStore.inc — Per-Jailor persistent state via DataStore.
// Mirrors :merchants:citizen/include/citizenStore exactly. One DataFile,
// one element per Jailor serial. Holds:
//   factions  array of faction indexes (1..5 into FACCAO_NOMES)
// =============================================================================

use os;
use uo;
use polsys;
use datafile;

const JAILOR_DATAFILE := ":jailor:jailorState";

function _JailorDataFile()
	return CreateDataFile(JAILOR_DATAFILE, DF_KEYTYPE_INTEGER);
endfunction

function GetJailorElem(npc)
	var df := _JailorDataFile();
	var elem := df.FindElement(npc.serial);
	if (!elem)
		elem := df.CreateElement(npc.serial);
		elem.SetProp("factions", array{});
	endif
	return elem;
endfunction

function GetJailorFactions(npc)
	var elem := GetJailorElem(npc);
	var f := elem.GetProp("factions");
	if (!f)
		return array{};
	endif
	return f;
endfunction

function SetJailorFactions(npc, factions)
	var elem := GetJailorElem(npc);
	elem.SetProp("factions", factions);
endfunction
