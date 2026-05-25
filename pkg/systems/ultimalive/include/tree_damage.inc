// tree_damage.inc — Lida com fases de destruição de árvores via UltimaLive.
// Include path: ":ultimalive:include/tree_damage"

use basic;

include ":ultimalive:include/packets";
include ":ultimalive:include/trees_meta";

const UL_TREE_PHASES_DF    := ":ultimalive:tree_phases";
const UL_PHASE_1_THRESHOLD := 20;  // wood_amount > 20 → fase 1
const UL_TREE_STUMP_GRAPHIC := 0x0E59;

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

function _UL_StorePhase(realm, x, y, trunk_graphic, phase)
    var df := _UL_GetTreePhasesDF();
    if (df.errortext)
        return;
    endif
    var key := _UL_TreePhaseKey(realm, x, y, trunk_graphic);
    var el  := df.FindElement(key);
    if (!el)
        el := df.CreateElement(key);
    endif
    el.SetProp("Phase", phase);
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
    var z_foliage := 0;
    var z_trunk   := 0;

    if (new_phase == 2 &&
        old_phase < 2)
        z_foliage := _UL_FindStaticZ(realm, x, y, meta.foliage);
        if (!z_foliage.errortext)
            UL_DeleteStatic(realm, x, y, z_foliage, meta.foliage);
        endif
    endif

    if (new_phase == 3 &&
        old_phase < 3)
        if (old_phase < 2)
            z_foliage := _UL_FindStaticZ(realm, x, y, meta.foliage);
            if (!z_foliage.errortext)
                UL_DeleteStatic(realm, x, y, z_foliage, meta.foliage);
            endif
        endif
        z_trunk := _UL_FindStaticZ(realm, x, y, meta.trunk);
        if (z_trunk.errortext)
            return 0;
        endif
        UL_SetStatic(realm, x, y, z_trunk, UL_TREE_STUMP_GRAPHIC, 0, "ul_tree_damage", meta.trunk);
    endif

    UL_SendStaticsUpdate(who, realm, bx, by);
    return 1;
endfunction

function UL_DamageTree(realm, x, y, tree_graphic, wood_amount, who)
    var meta := UL_GetTreeMeta(CInt(tree_graphic));
    if (meta.errortext)
        return 0;
    endif
    var new_phase := _UL_DetermineFase(CInt(wood_amount));
    var old_phase := _UL_GetStoredPhase(realm, x, y, meta.trunk);
    if (old_phase == 0)
        old_phase := 1;
    endif
    if (new_phase <= old_phase)
        return 0;
    endif
    var ok := _UL_ApplyFaseTransition(realm, x, y, meta, new_phase, old_phase, who);
    if (!ok)
        return 0;
    endif
    _UL_StorePhase(realm, x, y, meta.trunk, new_phase);
    return new_phase;
endfunction
