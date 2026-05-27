// tree_damage.inc — Lida com fases de destruição de árvores via UltimaLive.
// Include path: ":ultimalive:include/tree_damage"

use basic;
use os;

include ":ultimalive:include/packets";
include ":ultimalive:include/trees_meta";
include ":ultimalive:include/tree_pieces";

const UL_TREE_PHASES_DF     := ":ultimalive:tree_phases";
const UL_PHASE_1_THRESHOLD  := 20;       // wood_amount > 20 → fase 1
const UL_TREE_STUMP_GRAPHIC := 0x0E59;
const UL_REGEN_SECONDS      := 1800;     // 30*60 em producao

function _UL_GetTreePhasesDF()
    var df := DFOpenDataFile(UL_TREE_PHASES_DF, DF_CREATE);
    if (!df)
        return error{"errortext" := "UL: failed to open tree_phases datafile"};
    endif
    return df;
endfunction

function _UL_TreePhaseKey(realm, x, y, trunk_graphic)
    return CStr(realm) + " " + CStr(CInt(x)) + " " + CStr(CInt(y)) + " " + CStr(CInt(trunk_graphic));
endfunction

function _UL_GetStoredPhase(realm, x, y, trunk_graphic)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext)
        return 0;
    endif
    var key := _UL_TreePhaseKey(realm, x, y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el)
        return 0;
    endif
    return CInt(el.GetProp("Phase"));
endfunction

function _UL_StorePhase(realm, x, y, trunk_graphic, phase, z_trunk := -1, z_foliage := -1)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext)
        return;
    endif
    var key := _UL_TreePhaseKey(realm, x, y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el)
        el := df.CreateElement(key);
    endif
    el.SetProp("Phase",        phase);
    el.SetProp("Realm",        CStr(realm));
    el.SetProp("X",            CInt(x));
    el.SetProp("Y",            CInt(y));
    el.SetProp("TrunkGraphic", CInt(trunk_graphic));
    if (CInt(z_trunk) != -1)
        el.SetProp("ZTrunk", CInt(z_trunk));
    endif
    if (CInt(z_foliage) != -1)
        el.SetProp("ZFoliage", CInt(z_foliage));
    endif
endfunction

function _UL_SetPiecesRemaining(realm, x, y, trunk_graphic, count)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext) return; endif
    var key := _UL_TreePhaseKey(realm, x, y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el)
        el := df.CreateElement(key);
    endif
    el.SetProp("PiecesRemaining", CInt(count));
endfunction

function _UL_DecrementPieces(realm, parent_x, parent_y, trunk_graphic)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext) return -1; endif
    var key := _UL_TreePhaseKey(realm, parent_x, parent_y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el) return -1; endif
    var pr := el.GetProp("PiecesRemaining");
    if (pr.errortext)
        return -1;
    endif
    var remaining := CInt(pr) - 1;
    if (remaining < 0) remaining := 0; endif
    el.SetProp("PiecesRemaining", remaining);
    return remaining;
endfunction

function _UL_SetWillRegenAt(realm, x, y, trunk_graphic)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext)
        return;
    endif
    var key := _UL_TreePhaseKey(realm, x, y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el)
        return;
    endif
    var existing := el.GetProp("WillRegenAt");
    if (!existing.errortext && CInt(existing) > 0)
        return;
    endif
    var regen_at := polcore().systime + UL_REGEN_SECONDS;
    el.SetProp("WillRegenAt", regen_at);
    print("[UL_DEBUG] WillRegenAt set: realm=" + CStr(realm) + " x=" + CInt(x) + " y=" + CInt(y) + " regen_at=" + regen_at);
endfunction

function _UL_DetermineFase(wood_amount)
    if (CInt(wood_amount) > UL_PHASE_1_THRESHOLD)
        return 1;
    elseif (CInt(wood_amount) > 0)
        return 2;
    else
        return 3;
    endif
endfunction

function _UL_FindStaticZ(realm, x, y, target_graphic)
    var statics := ListStaticsAtLocation(CInt(x), CInt(y), LIST_IGNORE_Z, 0, realm);
    foreach st in statics
        if (CInt(st.objtype) == CInt(target_graphic))
            return st.z;
        endif
    endforeach
    return error{"errortext" := "_UL_FindStaticZ: " + Hex(CInt(target_graphic)) + " nao encontrado em " + CInt(x) + "," + CInt(y)};
endfunction

function _UL_ApplyFaseTransition(realm, x, y, meta, new_phase, old_phase, who)
    var bx        := CInt(x) / 8;
    var by        := CInt(y) / 8;
    var z_foliage := -1;
    var z_trunk   := -1;
    var z_found   := 0;
    var res       := struct{};
    res.+ok        := 0;
    res.+z_trunk   := -1;
    res.+z_foliage := -1;

    if (new_phase == 2 &&
        old_phase < 2)
        z_found := _UL_FindStaticZ(realm, x, y, meta.foliage);
        if (!z_found.errortext)
            z_foliage := CInt(z_found);
            UL_DeleteStatic(realm, x, y, z_foliage, meta.foliage);
            PrintTextAbove(who, "*folhas comecam a cair*");
        endif
    endif

    if (new_phase == 3 &&
        old_phase < 3)
        if (old_phase < 2)
            z_found := _UL_FindStaticZ(realm, x, y, meta.foliage);
            if (!z_found.errortext)
                z_foliage := CInt(z_found);
                UL_DeleteStatic(realm, x, y, z_foliage, meta.foliage);
            endif
        endif
        z_found := _UL_FindStaticZ(realm, x, y, meta.trunk);
        if (z_found.errortext)
            res.z_foliage := z_foliage;
            return res;
        endif
        z_trunk := CInt(z_found);
        PrintTextAbove(who, "*o tronco do " + meta.type_name + " cede com um estalo*");
        UL_SetStatic(realm, x, y, z_trunk, UL_TREE_STUMP_GRAPHIC, 0, "ul_tree_damage", meta.trunk);
        UL_CreateFallenLogs(realm, x, y, z_trunk, meta, who);
    endif

    UL_NotifyNearbyPlayers(realm, bx, by);
    res.ok        := 1;
    res.z_trunk   := z_trunk;
    res.z_foliage := z_foliage;
    return res;
endfunction

function UL_DamageTree(realm, x, y, tree_graphic, wood_amount, who)
    print("[UL_DEBUG] DamageTree: realm=" + CStr(realm) + " x=" + CInt(x) + " y=" + CInt(y) + " graphic=" + Hex(CInt(tree_graphic)) + " wood=" + CInt(wood_amount) + " who=" + who.name);
    var meta := UL_GetTreeMeta(CInt(tree_graphic));
    if (meta.errortext)
        return 0;
    endif
    var new_phase := _UL_DetermineFase(CInt(wood_amount));
    var old_phase := _UL_GetStoredPhase(realm, x, y, meta.trunk);
    if (old_phase == 0)
        old_phase := 1;
    endif
    print("[UL_DEBUG] type=" + meta.type_name + " phase old->new=" + old_phase + "->" + new_phase);
    if (new_phase <= old_phase)
        return 0;
    endif
    var res := _UL_ApplyFaseTransition(realm, x, y, meta, new_phase, old_phase, who);
    if (res.ok == 0)
        return 0;
    endif
    _UL_StorePhase(realm, x, y, meta.trunk, new_phase, res.z_trunk, res.z_foliage);
    print("[UL_DEBUG] StorePhase: phase=" + new_phase + " z_trunk=" + res.z_trunk + " z_foliage=" + res.z_foliage);
    return new_phase;
endfunction
