// Helpers compartilhados do sistema Baú de Tranca Especial.
// Centraliza constantes, flag de debug e checagens repetidas.

use uo;
use os;

include "include/say";

// ====================================================================
// FLAG GLOBAL DE DEBUG
// ====================================================================
// Setar para 0 desliga TODOS os prints [TRANCA] e SysMessages [DEBUG]
// do fluxo do baú de tranca especial, em todos os arquivos que usam
// TrancaDebug.
const DEBUG_TRANCA := 0;

// ====================================================================
// CONSTANTES DE CONFIGURAÇÃO
// ====================================================================
const BAU_ESPECIAL_PUZZLE_OBJTYPE := 0xf022; // objtype interno que dispara PuzzleChestOpen
const DEFAULT_TRANCA_CHANCE              := 10;
const DEFAULT_TRANCA_MASTERMIND_CHANCE   := 25;
const DEFAULT_MIMIC_CHANCE               := 5;

// Piso obrigatório de chance de spawn do baú especial.
// Pra desligar o feature em um spawner específico, use o checkbox tranca_enabled=0
// (chance abaixo desse piso é silenciosamente clamped pra cima).
const MIN_TRANCA_CHANCE := 10;

// Pool visual default quando o GM não selecionou graphics.
// Função em vez de const porque escript não aceita const array.
function GetDefaultTrancaGraphics()
    return array{ 0xC284, 0xC2BE, 0xC298 };
endfunction

// ====================================================================
// HELPERS
// ====================================================================

// Checa se um chest é baú de tranca especial.
function IsTrancaEspecial(chest)
    if (!chest)
        return 0;
    endif
    return GetObjProperty(chest, "IsTrancaEspecial");
endfunction

// Log de debug do baú de tranca especial.
// - msg: texto que vai no print do script.log com prefixo [TRANCA]
// - who: opcional, se passado também envia SysMessage [DEBUG] ao player
function TrancaDebug(msg, who := 0)
    if (!DEBUG_TRANCA)
        return;
    endif
    print("[TRANCA] " + msg);
    if (who)
        SendSysMessageEx(who, "[DEBUG] " + msg, SSM_INFO);
    endif
endfunction

// Clamp de porcentagem para o range [lo..hi] (default 0..100).
function ClampPct(value, lo := 0, hi := 100)
    value := CInt(value);
    if (value < lo)
        return lo;
    elseif (value > hi)
        return hi;
    endif
    return value;
endfunction

// Lê ObjProperty com fallback default se não existir (error).
function GetPropOrDefault(obj, name, default_value)
    var v := GetObjProperty(obj, name);
    if (v == error)
        return default_value;
    endif
    return v;
endfunction
