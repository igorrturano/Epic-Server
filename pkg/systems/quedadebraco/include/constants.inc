use uo;
use os;

// Constantes do jogo
const TOTAL_ROUNDS := 5;
const ROUNDS_TO_WIN := 3;

// Estados do jogo
const STATE_WAITING := 0;
const STATE_STARTING := 1;
const STATE_CHOOSING_MOVES := 2;  // Mudado de STATE_CHOOSING para ser mais descritivo
const STATE_PROCESSING := 3;
const STATE_ROUND_END := 4;
const STATE_GAME_OVER := 5;

// Movimentos possíveis
const CONST_MOVE_NONE := 0;
const CONST_MOVE_HOOK := 1;    // +20% força, -20% fortitude
const CONST_MOVE_COVER := 2;    // +5% força, +5% fortitude
const CONST_MOVE_PRESSURE := 3;    // -20% força, +20% fortitude

// Sistema de sorte (antigo enum LUCK_CONSTANTS)
const LUCK_CRIT_SUCCESS := 95;    // 95+ = Crítico de Sucesso (5% de chance)
const LUCK_CRIT_FAIL := 5;     // 5- = Crítico de Falha (5% de chance)
const LUCK_BASE := 50;    // Valor base médio
const LUCK_VARIANCE := 30;    // Variação máxima (+/-)
const LUCK_MIN := 1;     // Valor mínimo absoluto
const LUCK_MAX := 100;   // Valor máximo absoluto

// Percentuais de contribuição (antigo enum PERCENTAGES)
const PERCENT_STRENGTH := 0.35;  // Era 0.42
const PERCENT_FORTITUDE := 0.35;  // Era 0.42
const PERCENT_LUCK := 0.25;  // Era 0.14
const PERCENT_RACIAL := 0.05;  // Era 0.02

// Pontos Raciais
const RACIAL_DEFAULT := 100;   // Base para cálculo
const RACIAL_ANAO := 120;      // 20% mais forte
const RACIAL_ORC := 120;       // 20% mais forte 
const RACIAL_HUMANO := 100;    // Base (padrão)
const RACIAL_ELFO := 90;       // 10% mais fraco
const RACIAL_DROW := 90;       // 10% mais fraco
const RACIAL_POLSKI := 80;     // 20% mais fraco