// CONSTANTS.INC - Constantes centralizadas do sistema Champion Spawn
// Criado durante refatoração para melhor manutenibilidade

// === ITEM OBJTYPES ===
const CHAMPION_ALTAR_OBJTYPE := 0x1822;
const RED_SKULL_OBJTYPE := 0x1854;
const WHITE_CANDLE_OBJTYPE := 0x142C;
const BOSS_AREA_PENTAGRAM_OBJTYPE := 0xfe6;
const CHAMPION_DROP_BAG_OBJTYPE := 0x0E76;

// === CORES/COLORS ===
const ALTAR_BOSS_TIMER_COLOR := 2709;
const WHITE_CANDLE_COLOR := 0x481;
const BOSS_AREA_COLOR := 0x497;

// === SONS/SOUNDS ===
const ALTAR_ACTIVATE_SOUND := 0x207;
const ALTAR_DEACTIVATE_SOUND := 0x1F8;
const BOSS_TIMER_COUNTDOWN_SOUND := 0x244;
const BOSS_SPAWN_SOUND := 0x2F3;

// === EFEITOS/EFFECTS ===
const BOSS_SPAWN_EFFECT := 0x3728;

// === TEMPOS/TIMING ===
const BOSS_TIMER_DURATION := 100;
const DEFAULT_COOLDOWN := 86400;
const PLAYER_TOLERANCE_TIME := 300;

// === DISTÂNCIAS/DISTANCES ===
const SPAWN_SEARCH_RADIUS := 20;
const MAX_SPAWN_ATTEMPTS := 100;
const GRID_SEARCH_STEP := 2;

// === COORDENADAS RELATIVAS DO ALTAR ===
// Usar funções em vez de const para arrays/structs

// === CONFIGURAÇÕES DE VALIDAÇÃO ===
const MIN_ALTAR_Z := -50;
const MAX_ALTAR_Z := 100;

// === FUNÇÕES PARA POSIÇÕES ===
function GetRedSkullPositions()
    return array{
        struct{ x := -1, y := -1 },  // Posição 0
        struct{ x :=  1, y := -1 },  // Posição 1
        struct{ x :=  1, y :=  1 },  // Posição 2
        struct{ x := -1, y :=  1 }   // Posição 3
    };
endfunction

function GetWhiteCandlePositions()
    return array{
        struct{ x := -2, y := -2 },  // Posição 1
        struct{ x :=  2, y := -2 },  // Posição 2  
        struct{ x :=  2, y :=  2 },  // Posição 3
        struct{ x := -2, y :=  2 }   // Posição 4
    };
endfunction


