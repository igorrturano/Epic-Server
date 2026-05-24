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

// Monta a chave string de um entry: "realm x y z" ex: "britannia_alt 6045 670 0"
function _UL_MutationKey(realm, x, y, z)
    return CStr(realm) + " " + CStr(x) + " " + CStr(y) + " " + CStr(z);
endfunction

// Insere ou substitui um static em (realm, x, y, z) com graphic e hue dados.
// "static_modify" se há static nativo em statics.mul neste z; "static_add" se não.
function UL_SetStatic(realm, x, y, z, graphic, hue, source := "unknown")
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z);

    var mutation_type := "static_add";
    var natives := ListStaticsAtLocation(CInt(x), CInt(y), LIST_IGNORE_Z, 0, realm);
    foreach st in natives
        if (CInt(st.z) == CInt(z))
            mutation_type := "static_modify";
            break;
        endif
    endforeach

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
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

// Registra remoção de static em (realm, x, y, z).
// Mantém entry no DataFile com "static_delete" — sub-fase 1.D usa esse registro
// para enviar packet 0x40 de remoção ao cliente.
function UL_DeleteStatic(realm, x, y, z, source := "unknown")
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z);

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
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

// Retorna a mutação registrada em (realm, x, y, z), ou error se não há mutação.
function UL_GetMutation(realm, x, y, z)
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z);
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

// Reverte (apaga) a mutação em (realm, x, y, z) do DataFile.
// Diferente de UL_DeleteStatic: RevertMutation "esquece" a operação sem deixar rastro —
// útil para admin desfazer erros sem que o cliente receba packet 0x40.
function UL_RevertMutation(realm, x, y, z)
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, z);
    var existing := df.FindElement(key);
    if (!existing) return error{"errortext" := "no mutation to revert"}; endif

    df.DeleteElement(key);
    return 1;
endfunction

// Substitui o tile de land em (realm, x, y) pelo graphic dado.
// z=0 é a convenção UltimaLive para land — sempre armazenado com z fixo.
// Land tiles não têm hue (diferente de statics).
function UL_SetLandTile(realm, x, y, graphic, source := "unknown")
    var df := _UL_GetMutationsDF();
    if (df == error) return df; endif

    var key := _UL_MutationKey(realm, x, y, 0);

    var existing := df.FindElement(key);
    if (!existing)
        df.CreateElement(key);
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
