use uo;
use os;
use util;

include ":quedadebraco:quedadebraco";

// 1. Preparação (mantido)
var PREPARACAO_EMOTES := array{
    "ajusta postura",
    "firma cotovelo",
    "Se concentra",          // 9 chars
    "respira fundo",      // 12 chars
    "estica braço",       // 11 chars
    "endireita",          // 9 chars
    "move cotovelo",      // 12 chars
    "Se posiciona",          // 9 chars
    "prepara o braço",      // 12 chars
    "ajeita na mesa",     // 13 chars
    "firma no lugar",     // 13 chars
    "encara o rival",  // 6 chars
    "alinha cotovelo",    // 14 chars
    "em posição",         // 10 chars
    "ajusta na mesa"      // 13 chars
};

var CONTAGEM_EMOTES := array{
    "3...",
    "2...",
    "1...",
    "Iniciando"
};
// 3. Força
// 3. Força
var FORCA_ALTA_EMOTES := array{
    "flexiona braço",
    "aperta forte",
    "puxa com tudo",
    "gira a mão",
    "pressiona firme",
    "resiste bem",
    "mantém foco",
    "estica braço",
    "sustenta peso",
    "empurra firme",
    "engancha no rival",
    "tensiona músculos",
    "ergue mão",
    "veia saltando",
    "domina jogo",  // Removido o ponto extra aqui
    "músculos tensos"
};

var FORCA_MEDIA_EMOTES := array{
    "mantem firme",
    "equilibrado",
    "poder balanceado",
    "forca constante",
    "trava o punho",
"sustenta posição", 
"mantém controle", 
"pressiona levemente",
"foca no equilíbrio",
"ajusta a força",
"resiste", 
"acompanha movimento",
"puxa devagar",
"mantém postura",
"força igualada",
"tensiona levemente",
"aperta com calma",
"estabiliza braço",
"se adapta bem",
"acomoda peso",
"sustenta leve",
"ajusta punho",
"controla energia"

};

var FORCA_BAIXA_EMOTES := array{
    "braço tremendo",
    "perde força",
    "fraqueja no aperto",
    "mal segura",
    "força sumindo",
    "energia falhando",
    "vigor baixo",
    "força cedendo",
    "mal aguenta",
    "energia acabando",
    "vigor esgotando",
    "braço fraco",
    "cansa rápido",
    "tremor no braço",
    "força escassa",
    "perde controle",
    "não aguenta",
    "energia se esvai",
    "poder caindo",
    "tensão fraca",
    "braço vacilante",
    "força desvanecendo",
    "força em queda",
    "energia se esgota",
    "quase desiste",
    "braço cansado",
    "sem vigor",
    "poder fraquejando",
    "força desfalecendo",
    "cansaço evidente"
};

// 4. Fortitude
var FORTITUDE_ALTA_EMOTES := array{

    "aguenta firme",
    "nao cansa",
    "folego de ferro",
    "aguenta muito",
    "energia plena",
    "resistencia suprema",
    "folego imenso",
    "resiste firme",
"poder constante",
"vigor inabalável",
"resistência sólida",
"aguenta forte",
"força inquebrável",
"determinação total",
"energia persistente",
"folego robusto",
"resiliência total",
"vigor indomável",
"potência constante",
"sustenta pressão",
"força persistente",
"vigor imbatível",
"resistência firme",
"força incansável",
"determinação firme"
};

var FORTITUDE_MEDIA_EMOTES := array{
    "resiste bem",
    "aguenta firme",
    "folego normal",
    "vigor constante",
    "energia estável",
"mantém firmeza",
"poder estável",
"energia balanceada",
"força regular",
"aguenta o tranco",
"resistência média",
"folego satisfatório",
"aguenta o ritmo",
"aguenta bem",
"folego estável",
"vigor consistente"
};

var FORTITUDE_BAIXA_EMOTES := array{
    "começa cansar",
    "folego curto",
    "resistencia fraca",
    "pouca energia",
    "mal resiste",
    "quase cede",
    "folego acabando",
    "vigor sumindo",
    "energia escassa",
    "resistencia cedendo",
    "mal aguenta",
    "folego final",
    "cansa rápido",
"energia fraca",
"resistência baixa",
"sem fôlego",
"quase desistindo",
"vigor em queda",
"mal se segura",
"força se esvai",
"resiste pouco",
"cansa facilmente",
"energia se esgota",
"vigor debilitado",
"quase sem energia",
"resistência instável",
"fôlego quase no fim"
};


// 6. Sorte
var SORTE_ALTA_EMOTES := array{
"toque da sorte", 
"golpe certeiro", 
"força inesperada",
"vibrações de vitória",
"chance dourada",
"sorte em alta",
"fôlego abençoado",
"empurrão da sorte",
"onda de sorte",
"força do acaso",
"sorte reluzente",
"brilho da vitória",
"vento a favor"
};

var SORTE_MEDIA_EMOTES := array{



"sorte em dia",
"sorriso discreto",
"energia constante",
"tudo na técnica",
"força no foco",
"habilidade em jogo",
"esforço constante",
"decisão firme",
"tensão controlada",
"estratégia em ação",
"momento de calma",
"equilíbrio perfeito",
"determinação sólida",
"controle absoluto",
"trabalho duro",
"treino recompensado",
"confiabilidade total",
"resistência calculada",
"foco inabalável",
"execução precisa"

    
};

// 6. Sorte
var SORTE_BAIXA_EMOTES := array{
    "ma sorte",
    "momento ruim",
    "energia negativa",
    "destino adverso",
    "sorte minima",
    "momento pessimo",
    "sorte em queda",
    "destino desfavorável",
    "momento desfavorável",
    "sorte nublada",
    "desastre a vista",
    "sorte desaparecendo",
    "momento de desespero",
    "destino traiçoeiro",
    "desânimo no ar",
    "sorte apagada",
    "energia em colapso",
    "sorte escorregadia",
    "caminho pedregoso"
};

var SORTE_CRITICO_SUCESSO_EMOTES := array{
"golpe inesperado",
"oponente escorrega",
"momento de distração",
"pequeno deslize",
"apoio da torcida",
"vibrações positivas",
"sinal divino",
"acerto perfeito",
"força do acaso",
"decisão certeira",
"instinto afiado",
"timing impecável",
"ajuste mágico",
"estratégia brilhante",
"energia contagiante",
"oportunidade rara",
"conexão súbita",
"ponto fraco revelado",
"acidente favorável",
"empurrão da sorte"
};

var SORTE_CRITICO_FALHA_EMOTES := array{
"escorregão fatal",
"decisão desastrosa",
"timing desastroso",
"concentração perdida",
"erro de cálculo",
"ponto fraco exposto",
"momento de fraqueza",
"fôlego se esvai",
"força esgotada",
"distração repentina",
"apoio desmoronando",
"desvio involuntário",
"sinal de desistência",
"pressão excessiva",
"desvio de atenção",
"desastre no punho",
"falta de sincronia",
"resistência quebrada",
"mau posicionamento",
"coração desanimado"
};

// 7. Resultado Final
var VITORIA_EMOTES := array{
"Força triunfante",
"Campeão absoluto",
"Glória alcançada",
"Sucesso exemplar",
"Valor inigualável",
"Vitória radiante",
"Força inigualável",
"Vitória retumbante",
"Poder sem igual",
"Desafio superado",
"Se levanta em triunfo",
"Sorri Largamento em vitória",
"Faz gesto de vitória",
"Celebra com danças",
"Gritos de alegria",
"Bate no peito orgulhoso",
"Mostra o punho cerrado"
};

// Substitui o array de emotes de derrota por sons
var DERROTA_SOUNDS := array{
    0x540,  // Som de derrota 1
    0x53e,  // Som de derrota 2
    0x444,  // Som de derrota 3
    0x440,  // Som de derrota 4
    0x42b,  // Som de derrota 5
    0x422   // Som de derrota 6
};

function PlayVictoryEmote(who)
    var display_name := GetDisplayName(who);
    if (!display_name)
        display_name := who.name;
    endif
    
    PrintTextAbove(who, "O competidor " + display_name + " Vence a Rodada!", 3, 68);
endfunction


var GANCHO_EMOTES := array{
    "puxa com gancho",    // 13 chars
    "gancho forte",       // 12 chars
    "gancho rapido",      // 12 chars
    "aplica gancho",      // 12 chars
    "gancho preciso",     // 13 chars
    "gancho certeiro",    // 14 chars
    "gira o gancho",      // 12 chars
    "gancho brutal",      // 12 chars
    "gancho firme",       // 11 chars
    "gancho veloz",       // 11 chars
    "gancho mortal",      // 12 chars
    "gancho fulminante",  // 15 chars
    "gancho preciso",     // 13 chars
    "gancho decisivo",    // 14 chars
    "Puxa para perto",
    "gancho potente"      // 13 chars
};

var COBERTURA_EMOTES := array{
    "firme e forte",      // 12 chars
    "bloqueia tudo",      // 12 chars
    "defende firme",      // 12 chars
    "nao cede nada",      // 12 chars
    "segura forte",       // 11 chars
    "aguenta firme",      // 12 chars
    "resiste bem",        // 10 chars
    "nãoo recua",          // 9 chars
    "Usa Alavanca",       // 11 chars
    "Faz o giro rápido",
    "Aperta o punho",
    "bloqueia bem",       // 11 chars
    "trava tudo",         // 10 chars
    "defesa perfeita",    // 14 chars
    "nao passa nada",     // 13 chars
    "defende tudo",       // 11 chars
    "bloqueia forte"      // 13 chars
};

var PRESSAO_EMOTES := array{
    "pressiona forte",    // 14 chars
    "força total",        // 10 chars
    "Sustenta pressão",     // 12 chars
    "aumenta forca",      // 12 chars
    "pressiona mais",     // 12 chars
    "empurra forte",      // 12 chars
    "pressiona rápido",       // 11 chars
    "força firme",        // 10 chars
    "pressiona bem",      // 11 chars
    "força plena",        // 10 chars
    "Faz força extra",      // 11 chars
    "Empurra com tudo",     // 12 chars
    "pressiona tudo",     // 12 chars
    "pressão forte",      // 11 chars
    "Aumenta a carga"
};

// Funções unificadas para emotes aleatórios
function PlayPreparacaoEmote(who)
    var emote := PREPARACAO_EMOTES[RandomInt(PREPARACAO_EMOTES.size()) + 1];
    PrintTextAbove(who, "*" + emote + "*", 3, 40);
endfunction

function PlayContagemEmote(who, index)
    PrintTextAbove(who, "*" + CONTAGEM_EMOTES[index] + "*", 3, 53);
endfunction




function PlayMoveEmote(who, move_type)
    var emotes;
    case (move_type)
        CONST_MOVE_HOOK: emotes := GANCHO_EMOTES;
        CONST_MOVE_COVER: emotes := COBERTURA_EMOTES;
        CONST_MOVE_PRESSURE: emotes := PRESSAO_EMOTES;
        default: return;
    endcase
    
    var emote := emotes[RandomInt(emotes.size()) + 1];
    PerformAction(who, 0x22);
    PrintTextAbove(who, "*" + emote + "*", 3, 40);
endfunction




// Função atualizada para reproduzir sons de força
function PlayForcaEmote(who, valor)
    var emotes;
    var sound;
    
    if (valor >= 100)
        emotes := FORCA_ALTA_EMOTES;
        sound := FORCA_SOUNDS.forte;
    elseif (valor >= 50)
        emotes := FORCA_MEDIA_EMOTES;
        sound := FORCA_SOUNDS.media;
    else
        emotes := FORCA_BAIXA_EMOTES;
        sound := FORCA_SOUNDS.fraca;
    endif
    
    var emote := emotes[RandomInt(emotes.size()) + 1];
    PrintTextAbove(who, "*" + emote + "*", 3, GetEmoteColor(valor));
    PlaySoundEffect(who, sound);
endfunction

// Função para exibir emotes de fortitude baseado no valor
function PlayFortitudeEmote(who, valor)
    var emotes;
    if (valor >= 100)
        emotes := FORTITUDE_ALTA_EMOTES;
    elseif (valor >= 50)
        emotes := FORTITUDE_MEDIA_EMOTES;
    else
        emotes := FORTITUDE_BAIXA_EMOTES;
    endif
    
    var emote := emotes[RandomInt(emotes.size()) + 1];
    PrintTextAbove(who, "*" + emote + "*", 3, GetEmoteColor(valor));
endfunction

// Função para exibir emotes de sorte baseado no valor e críticos
function PlaySorteEmote(who, valor, is_critico := 0, tipo_critico := 0)
    var emotes;
    
    if (is_critico)
        if (tipo_critico == 1) // Crítico de sucesso
            emotes := SORTE_CRITICO_SUCESSO_EMOTES;
        else // Crítico de falha
            emotes := SORTE_CRITICO_FALHA_EMOTES;
        endif
    else
        if (valor >= 75)
            emotes := SORTE_ALTA_EMOTES;
        elseif (valor >= 35)
            emotes := SORTE_MEDIA_EMOTES;
        else
            emotes := SORTE_BAIXA_EMOTES;
        endif
    endif
    
    var emote := emotes[RandomInt(emotes.size()) + 1];
    PrintTextAbove(who, "*" + emote + "*", 3, GetEmoteColor(valor));
endfunction

// Função atualizada para reproduzir som de derrota em vez de emote
function PlayResultadoEmote(who, is_victory := 0)
    if (is_victory)
        var emote := VITORIA_EMOTES[RandomInt(VITORIA_EMOTES.size()) + 1];
        PrintTextAbove(who, "*" + emote + "*", 3, 68);
        PlaySoundEffect(who, VITORIA_SOUND);
    else
        // Para derrota, apenas toca um som aleatório do array
        var sound := DERROTA_SOUNDS[RandomInt(DERROTA_SOUNDS.size()) + 1];
        PlaySoundEffect(who, sound);
    endif
endfunction

// Função utilitária para determinar a cor do emote baseado no valor
function GetEmoteColor(valor)
    if (valor >= 100)
        return 68; // Verde para alto
    elseif (valor >= 50)
        return 53; // Amarelo para médio
    else
        return 38; // Vermelho para baixo
    endif
endfunction

// Nova função para movimentos que substitui a PlayMoveEmote existente
function PlayMovimentoEmote(who, move_type)
    var emotes;
    var cor;
    
    case (move_type)
        CONST_MOVE_HOOK: 
            emotes := GANCHO_EMOTES;
            cor := 53; // Amarelo
        CONST_MOVE_COVER: 
            emotes := COBERTURA_EMOTES;
            cor := 68; // Verde
        CONST_MOVE_PRESSURE: 
            emotes := PRESSAO_EMOTES;
            cor := 38; // Vermelho
        default: 
            return;
    endcase
    
    var emote := emotes[RandomInt(emotes.size()) + 1];
    PerformAction(who, 0x22);
    PrintTextAbove(who, "*" + emote + "*", 3, cor);
endfunction

// Função auxiliar para adicionar delay entre emotes
function WaitBetweenEmotes()
    sleepms(2000); // Espera 2 segundos entre emotes
endfunction



// Sons para falhas críticas
var CRITICO_FALHA_SOUNDS := array{
    0x43f,  // Som de falha crítica 1
    0x428   // Som de falha crítica 2
};

// Sons para vitórias críticas
var CRITICO_SUCESSO_SOUNDS := array{
    0x430,  // Som de vitória crítica 1
    0x431   // Som de vitória crítica 2
};

// Sons para diferentes níveis de força
var FORCA_SOUNDS := struct{
    forte := 0x439,  // Som de força forte
    media := 0x43a,  // Som de força média
    fraca := 0x434   // Som de força fraca
};

// Som de vitória normal
const VITORIA_SOUND := 0x41e;