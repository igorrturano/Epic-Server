// =============================================================================
// citizenStore.inc — Per-citizen persistent state via DataStore.
//
// One DataFile, one element per citizen serial. Holds:
//   factions          array of faction indexes (1..5 into FACCAO_NOMES)
//   jurisdiction      string ("Valtheris" — Phase 1 only value)
//   witnessLog        ring buffer of observation structs, max N=20, 24h decay
//   awarenessFreeUsed dict {playerSerial: lastFreeTs} for AWARENESS-3 perk
//
// The same datafile also holds an element keyed by *perp* serial to store
// "denouncedBy_<citizenSerial>" records (Layer 3 of the denunciation pipeline).
// =============================================================================

use os;
use uo;
use polsys;
use datafile;

const CITIZEN_DATAFILE      := ":merchants:citizenState";
const CITIZEN_WITNESS_MAX   := 20;
const CITIZEN_WITNESS_TTL   := 86400;  // 24 hours in game-clock seconds

function _CitizenDataFile()
    return CreateDataFile(CITIZEN_DATAFILE, DF_KEYTYPE_INTEGER);
endfunction

function GetCitizenElem(npc)
    var df := _CitizenDataFile();
    var elem := df.FindElement(npc.serial);
    if (!elem)
        elem := df.CreateElement(npc.serial);
        elem.SetProp("factions", array{});
        elem.SetProp("jurisdiction", "Valtheris");
        elem.SetProp("witnessLog", array{});
        elem.SetProp("awarenessFreeUsed", dictionary);
    endif
    return elem;
endfunction

function GetCitizenFactions(npc)
    var elem := GetCitizenElem(npc);
    var f := elem.GetProp("factions");
    if (!f)
        return array{};
    endif
    return f;
endfunction

function SetCitizenFactions(npc, factions)
    var elem := GetCitizenElem(npc);
    elem.SetProp("factions", factions);
endfunction

function GetCitizenJurisdiction(npc)
    var elem := GetCitizenElem(npc);
    var j := elem.GetProp("jurisdiction");
    if (!j)
        return "Valtheris";
    endif
    return j;
endfunction

function _PruneArray(log, now)
    var pruned := array{};
    foreach e in log
        if (now - CInt(e.ts) < CITIZEN_WITNESS_TTL)
            pruned.append(e);
        endif
        sleepms(1);
    endforeach
    return pruned;
endfunction

function _DropOldest(log, max_size)
    if (log.size() <= max_size)
        return log;
    endif
    var trimmed := array{};
    var start_idx := log.size() - max_size + 1;
    var i := start_idx;
    while (i <= log.size())
        trimmed.append(log[i]);
        i := i + 1;
    endwhile
    return trimmed;
endfunction

function AppendWitness(npc, entry)
    var elem := GetCitizenElem(npc);
    var log := elem.GetProp("witnessLog");
    if (!log)
        log := array{};
    endif

    log := _PruneArray(log, ReadGameClock());
    log.append(entry);
    log := _DropOldest(log, CITIZEN_WITNESS_MAX);

    elem.SetProp("witnessLog", log);
endfunction

function PruneWitnessLog(npc)
    var elem := GetCitizenElem(npc);
    var log := elem.GetProp("witnessLog");
    if (!log)
        return;
    endif
    elem.SetProp("witnessLog", _PruneArray(log, ReadGameClock()));
endfunction

function GetWitnessLog(npc)
    var elem := GetCitizenElem(npc);
    var log := elem.GetProp("witnessLog");
    if (!log)
        return array{};
    endif
    return log;
endfunction

function MarkAwarenessFreeUsed(npc, player)
    var elem := GetCitizenElem(npc);
    var d := elem.GetProp("awarenessFreeUsed");
    if (!d)
        d := dictionary;
    endif
    d[player.serial] := ReadGameClock();
    elem.SetProp("awarenessFreeUsed", d);
endfunction

function IsAwarenessFreeAvailable(npc, player)
    var elem := GetCitizenElem(npc);
    var d := elem.GetProp("awarenessFreeUsed");
    if (!d || !d.Exists(player.serial))
        return 1;
    endif
    var last := CInt(d[player.serial]);
    return (ReadGameClock() - last >= CITIZEN_WITNESS_TTL);
endfunction

function RecordDenounceOnPerp(citizen, perp, entry)
    var df := _CitizenDataFile();
    var elem := df.FindElement(perp.serial);
    if (!elem)
        elem := df.CreateElement(perp.serial);
    endif
    elem.SetProp("denouncedBy_" + citizen.serial, struct{
        "ts" := ReadGameClock(),
        "type" := entry.type,
        "citizenName" := citizen.name,
        "citizenSerial" := citizen.serial
    });
endfunction
