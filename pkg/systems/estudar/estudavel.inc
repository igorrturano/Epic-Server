// ============================================================================
// estudavel.inc - Configurações do Sistema de Estudo
// ============================================================================

const SKILL_LIMIT := 52.0;
const GAIN_AMOUNT := 0.1;
const SOUND_STUDY := 0x24a;

const DELAY_EMOTE_TO_PHRASE := 5000;  // 5 segundos
const DELAY_BETWEEN_ACTIONS := 8000;   // 8 segundos
const DELAY_FINAL := 1000;             // 1 segundo antes do skill check

const NUM_ACTIONS_MIN := 2;
const NUM_ACTIONS_MAX := 3;

// ============================================================================
// Função para obter skills válidas
// ============================================================================

function GetValidSkills()
    return array{"Theology", "Science", "Arcana"};
endfunction

// ============================================================================
// Frases temáticas por skill
// ============================================================================

function GetFrasesPorSkill(skill_name)
    var frases := struct{};

    if (skill_name == "Theology")
        frases.estudando := array{"*meditando sobre os princípios fundamentais da crença*"};
        frases.analisando := array{"*buscando a interpretação mais profunda do texto principal*"};
        frases.consultando := array{"*comparando ensinamentos e doutrinas com diligência*"};
        frases.registrando := array{"*anotando reflexões sobre a natureza do divino*"};
        frases.praticando := array{"*realizando uma prática contemplativa para obter clareza*"};

    elseif (skill_name == "Arcana")
        frases.estudando := array{"*tentando decifrar símbolos e escrituras antigas*"};
        frases.analisando := array{"*traçando e analisando padrões de energia invisível*"};
        frases.consultando := array{"*folheando volumes que tratam de fenômenos desconhecidos*"};
        frases.registrando := array{"*registrando as variáveis de um complexo arranjo místico*"};
        frases.praticando := array{"*manipulando uma pequena manifestação de poder*"};

    elseif (skill_name == "Science")
        frases.estudando := array{"*focando no detalhe de um novo diagrama ou esquema*"};
        frases.analisando := array{"*verificando a validade de uma hipótese com rigor*"};
        frases.consultando := array{"*revisando dados e resultados de observações anteriores*"};
        frases.registrando := array{"*escrevendo cálculos e medições de forma organizada*"};
        frases.praticando := array{"*preparando cuidadosamente os instrumentos para a próxima fase*"};
    endif

    return frases;
endfunction