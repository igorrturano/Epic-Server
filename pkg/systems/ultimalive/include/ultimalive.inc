// ultimalive.inc — API pública UltimaLive: mutações de static e land tile.
// Include path: ":ultimalive:include/ultimalive"

use uo;
use os;
use basic;
use datafile;
use polsys;

include ":datafile:datafile";

const UL_MUTATIONS_DF := ":ultimalive:mutations";

// Abre (ou cria) o DataFile de mutações. Retorna referência ou error.
function _UL_GetMutationsDF()
    var df := DFOpenDataFile(UL_MUTATIONS_DF, DF_CREATE);
    if (!df)
        return error{"errortext" := "UL: failed to open mutations datafile"};
    endif
    return df;
endfunction

// Monta a chave string de um entry: "realm x y z graphic"
// Land usa z=0, graphic=0 por convenção. Statics usam graphic nativo ou adicionado.
function _UL_MutationKey(realm, x, y, z, graphic)
    return CStr(realm) + " " + CStr(x) + " " + CStr(y) + " " + CStr(z) + " " + CStr(graphic);
endfunction

const UL_MUTATED_BLOCKS_DF := ":ultimalive:mutated_blocks";

function _UL_GetMutatedBlocksDF()
    var df := DFOpenDataFile(UL_MUTATED_BLOCKS_DF, DF_CREATE);
    if (!df)
        return error{"errortext" := "UL: failed to open mutated_blocks datafile"};
    endif
    return df;
endfunction

function _UL_MutatedBlockKey(realm, bx, by)
    return CStr(realm) + " " + CStr(bx) + " " + CStr(by);
endfunction

// Incrementa o contador do bloco. Chamado apenas na criação de nova chave de mutação.
function UL_MarkBlockMutated(realm, x, y)
    var df  := _UL_GetMutatedBlocksDF();
    if (df == error) return df; endif
    var bx  := CInt(x) / 8;
    var by  := CInt(y) / 8;
    var key := _UL_MutatedBlockKey(realm, bx, by);
    var elem := df.FindElement(key);
    if (!elem)
        df.CreateElement(key);
        elem := df.FindElement(key);
        if (!elem) return error{"errortext" := "UL_MarkBlockMutated: could not create element"}; endif
        elem.SetProp("Count", 1);
    else
        elem.SetProp("Count", CInt(elem.GetProp("Count")) + 1);
    endif
    return 1;
endfunction

// Decrementa o contador do bloco. Remove elemento quando Count chega a 0.
function UL_UnmarkBlockMutated(realm, x, y)
    var df  := _UL_GetMutatedBlocksDF();
    if (df == error) return df; endif
    var bx  := CInt(x) / 8;
    var by  := CInt(y) / 8;
    var key := _UL_MutatedBlockKey(realm, bx, by);
    var elem := df.FindElement(key);
    if (!elem) return 1; endif
    var count := CInt(elem.GetProp("Count")) - 1;
    if (count <= 0)
        df.DeleteElement(key);
    else
        elem.SetProp("Count", count);
    endif
    return 1;
endfunction

function UL_IsBlockMutated(realm, bx, by)
    var df := _UL_GetMutatedBlocksDF();
    if (df == error) return 0; endif
    var elem := df.FindElement(_UL_MutatedBlockKey(realm, bx, by));
    if (elem) return 1; endif
    return 0;
endfunction

// Insere ou substitui um static em (realm, x, y, z).
// target_graphic := -1  → pega primeiro native no z (comportamento padrão, un-targeted).
// target_graphic >= 0   → procura native com esse graphic específico no z.
//   Encontrou: static_modify, chave usa target_graphic.
//   Não encontrou: ERRO — chamador declarou um native que não existe.
// Se nenhum native em z (qualquer caminho): static_add, chave usa graphic novo.
function UL_SetStatic(realm, x, y, z, graphic, hue, source := "unknown", target_graphic := -1)
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var mutation_type := "static_add";
    var key_graphic   := CInt(graphic);
    var natives := ListStaticsAtLocation(CInt(x), CInt(y), LIST_IGNORE_Z, 0, realm);

    if (CInt(target_graphic) >= 0)
        var found := 0;
        foreach st in natives
            if (CInt(st.z) == CInt(z) && CInt(st.objtype) == CInt(target_graphic))
                mutation_type := "static_modify";
                key_graphic   := CInt(target_graphic);
                found         := 1;
                break;
            endif
        endforeach
        if (!found)
            return error{"errortext" := "UL_SetStatic: native " + Hex(CInt(target_graphic)) + " nao encontrado em z=" + CInt(z)};
        endif
    else
        foreach st in natives
            if (CInt(st.z) == CInt(z))
                mutation_type := "static_modify";
                key_graphic   := CInt(st.objtype);
                break;
            endif
        endforeach
    endif

    var key := _UL_MutationKey(realm, x, y, z, key_graphic);

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
        UL_MarkBlockMutated(realm, x, y);
    endif
    var elem := df.FindElement(key);
    if (!elem) return error{"errortext" := "UL_SetStatic: could not create element"}; endif

    elem.SetProp("MutationType", mutation_type);
    elem.SetProp("Graphic", CInt(graphic));
    elem.SetProp("Hue", CInt(hue));
    elem.SetProp("CreatedAt", CInt(polcore().systime));
    elem.SetProp("Source", CStr(source));
    return 1;
endfunction

// Registra remoção de static. target_graphic = graphic nativo a remover (identifica a chave).
function UL_DeleteStatic(realm, x, y, z, target_graphic, source := "unknown")
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z, CInt(target_graphic));

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
        UL_MarkBlockMutated(realm, x, y);
    endif
    var elem := df.FindElement(key);
    if (!elem) return error{"errortext" := "UL_DeleteStatic: could not create element"}; endif

    elem.SetProp("MutationType", "static_delete");
    elem.SetProp("Graphic", 0);
    elem.SetProp("Hue", 0);
    elem.SetProp("CreatedAt", CInt(polcore().systime));
    elem.SetProp("Source", CStr(source));
    return 1;
endfunction

// Retorna a mutação em (realm, x, y, z, target_graphic), ou error se não há.
// Para land: z=0, target_graphic=0. Para statics: target_graphic = graphic nativo ou adicionado.
function UL_GetMutation(realm, x, y, z, target_graphic)
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z, CInt(target_graphic));
    var elem := df.FindElement(key);
    if (!elem) return error{"errortext" := "no mutation"}; endif

    var result := struct{};
    result.+MutationType := CStr(elem.GetProp("MutationType"));
    result.+Graphic      := CInt(elem.GetProp("Graphic"));
    result.+Hue          := CInt(elem.GetProp("Hue"));
    result.+CreatedAt    := CInt(elem.GetProp("CreatedAt"));
    result.+Source       := CStr(elem.GetProp("Source"));
    return result;
endfunction

// Reverte (apaga) a mutação sem rastro. target_graphic = graphic nativo (ou 0 para land).
function UL_RevertMutation(realm, x, y, z, target_graphic)
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z, CInt(target_graphic));
    var existing := df.FindElement(key);
    if (!existing) return error{"errortext" := "no mutation to revert"}; endif

    df.DeleteElement(key);
    UL_UnmarkBlockMutated(realm, x, y);
    return 1;
endfunction

// Substitui o tile de land em (realm, x, y) pelo graphic dado.
// Chave fixa: z=0, graphic=0 (convenção land — graphic 0 nunca é static válido).
function UL_SetLandTile(realm, x, y, graphic, source := "unknown")
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, 0, 0);

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
        UL_MarkBlockMutated(realm, x, y);
    endif
    var elem := df.FindElement(key);
    if (!elem) return error{"errortext" := "UL_SetLandTile: could not create element"}; endif

    elem.SetProp("MutationType", "land_modify");
    elem.SetProp("Graphic", CInt(graphic));
    elem.SetProp("CreatedAt", CInt(polcore().systime));
    elem.SetProp("Source", CStr(source));
    return 1;
endfunction

// Altera a altitude do tile de land em (realm, x, y) para z.
function UL_SetLandAltitude(realm, x, y, z)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction
