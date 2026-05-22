// ultimalive.inc — API pública UltimaLive: mutações de static e land tile.
// Include path: ":ultimalive:include/ultimalive"

use uo;
use os;
use datafile;

// Insere ou substitui um static em (realm, x, y, z) com graphic e hue dados.
function UL_SetStatic(realm, x, y, z, graphic, hue)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Remove o static em (realm, x, y, z) da camada de mutações.
function UL_DeleteStatic(realm, x, y, z)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Substitui o tile de land em (realm, x, y) pelo graphic dado.
function UL_SetLandTile(realm, x, y, graphic)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Altera a altitude do tile de land em (realm, x, y) para z.
function UL_SetLandAltitude(realm, x, y, z)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Retorna a mutação registrada em (realm, x, y, z), ou error se não há mutação.
function UL_GetMutation(realm, x, y, z)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction

// Reverte a mutação em (realm, x, y, z) ao estado original do mapa.
function UL_RevertMutation(realm, x, y, z)
    return error{"errortext" := "STUB - sub-fase 1.A"};
endfunction
