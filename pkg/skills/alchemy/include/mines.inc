use uo;
use os;
use datafile;

/**
 * Shared helpers for Granadier land mines (Campo Minado, L5).
 *
 * Mines are alchemy explosives planted via Minaterrestre() with gatilho
 * GATILHO_MINA. They register in the shared :tn:traps DataFile (keyed by
 * item.serial) and carry CProps: pottrap=1, CreatedBy=<planter serial>,
 * ExplosivoConfig, level. The DataFile is shared with floor traps, so always
 * filter by pottrap before treating an entry as a mine.
 */

// All currently-valid mines owned by ownerSerial. Skips dead serials and
// non-mine entries (floor traps).
function EnumOwnedMines(ownerSerial)
    var result := array{};
    var df := OpenDataFile(":tn:traps");
    if (!df)
        return result;
    endif
    var keys := df.keys();
    foreach key in (keys)
        var item := SystemFindObjectBySerial(CInt(key));
        if (!item)
            continue;
        endif
        if (CInt(GetObjProperty(item, "pottrap")) != 1)
            continue;
        endif
        if (CInt(GetObjProperty(item, "CreatedBy")) != CInt(ownerSerial))
            continue;
        endif
        result.append(item);
    endforeach
    return result;
endfunction

function CountOwnedMines(ownerSerial)
    return EnumOwnedMines(ownerSerial).size();
endfunction

// Detonate one mine: clean its DataFile element, then run the standard exploder
// (which applies the blast and destroys the item itself).
function DetonateMine(mine)
    if (!mine)
        return 0;
    endif
    var df := OpenDataFile(":tn:traps");
    if (df)
        df.DeleteElement(mine.serial);
    endif
    start_script(":alchemy:potions/exploder", { mine, mine });
    return 1;
endfunction
