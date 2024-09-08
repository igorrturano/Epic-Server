use uo;
use os;

include "include/client";
include "include/say";
include ":charactercreation:createchar";

const RACA_ANAO := "ANAO";
const RACA_DROW := "DROW";
const RACA_ELFO := "ELFO";
const RACA_ORC := "ORC";
const RACA_HUMANO := "HUMANO";
const RACA_POLSKI := "POLSKI";


function GetCharacteristicasRP(race)
    var caracteristicas := dictionary{
        "AIGLANOS" := GetAiglanosCaracteristicas(),
        "BADURAN" := GetBaduranCaracteristicas(),
        "BJORSKA" := GetBjorskeCaracteristicas(),
        "HAROLOTH" := GetHarolothCaracteristicas(),
        "ALURIEL" := GetAlurielCaracteristicas(),
        "GULTHRAK" := GetGulthrakCaracteristicas(),
        "POLKINEA" := GetPolkineaCaracteristicas(),
        "POVOS_DO_LESTE" := GetPovoDoLesteCaracteristicas()
    };
    return caracteristicas[race];
endfunction

function GetRacialTraits(race, chardata := 0)

    case (race)
        ANAO: return GetBaduranCaracteristicas();
        DROW: return GetHarolothCaracteristicas();
        ELFO: return GetAlurielCaracteristicas();
        ORC: return GetGulthrakCaracteristicas();
        HUMANO: 
            if (chardata && chardata.culture == AIGLANA)
                return GetAiglanosCaracteristicas();
            elseif (chardata && chardata.culture == LESTE)
                return GetPovoDoLesteCaracteristicas();
            elseif (chardata && chardata.culture == BJORSKA)
                return GetBjorskeCaracteristicas();
            else
                return GetAiglanosCaracteristicas(); // Default para humanos
            endif
        POLSKI: return GetPolkineaCaracteristicas();
        default: 

            return error; // Retorna error em vez de struct vazio
    endcase
endfunction

function RacaToString(raca)
    case (raca)
        ANAO: return "anão";
        DROW: return "drow";
        ELFO: return "elfo";
        ORC: return "orc";
        HUMANO: return "humano";
        POLSKI: return "polski";
        default: return "Desconhecido";
    endcase
endfunction

function TranslateRacialTrait(race, category, value, chardata := 0)


var racial_traits := GetRacialTraits(chardata.raca, chardata);


if (racial_traits == error)

    return 0;
endif

var CaracteristicasCategories := array{
    "Altura",
    "Peso",
    "Olhos",
    "Idade Aparente",
    "Voz"
};
endfunction

function GetAiglanosCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,60m-1,65m)",
                "médio-baixo (1,66m-1,70m)",
                "médio (1,71m-1,75m)",
                "médio-alto (1,76m-1,80m)",
                "alto (1,81m-1,85m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "esbelto (55kg-65kg)",
                "médio (66kg-75kg)",
                "atlético (76kg-85kg)",
                "robusto (86kg-95kg)",
                "corpulento (96kg-105kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Caracteristica dos Olhos",
            "opcoes" := array{
                "castanhos amendoados",
                "verdes arredondados",
                "azuis penetrantes",
                "cinzentos estreitos",
                "âmbar expressivos"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem (20 anos)",
                "adulto (30 anos)",
                "meia-idade (40 anos)",
                "maduro (50 anos)",
                "idoso (60 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
                "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction
function GetBaduranCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "muito baixo (1,20m-1,25m)",
                "baixo (1,26m-1,30m)",
                "médio (1,31m-1,35m)",
                "alto (1,36m-1,40m)",
                "muito alto (1,41m-1,48m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "leve (50kg-60kg)",
                "médio (61kg-70kg)",
                "pesado (71kg-80kg)",
                "muito pesado (81kg-90kg)",
                "pesado (91kg-100kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "castanhos profundos",
                "azuis brilhantes",
                "verdes intensos",
                "cinza metálico",
                "âmbar dourado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem (75 anos)",
                "adulto (120 anos)",
                "meia-idade (170 anos)",
                "maduro (230 anos)",
                "idoso (250 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
               "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function GetBjorskeCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,65m-1,70m)",
                "médio-baixo (1,71m-1,75m)",
                "médio (1,76m-1,80m)",
                "médio-alto (1,81m-1,85m)",
                "alto (1,86m-1,90m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "esbelto (70kg-80kg)",
                "médio (81kg-90kg)",
                "atlético (91kg-100kg)",
                "robusto (101kg-110kg)",
                "corpulento (111kg-120kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "azuis",
                "verdes",
                "cinzentos",
                "castanhos",
                "um olho de cada cor"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem (20 anos)",
                "adulto (30 anos)",
                "meia-idade (40 anos)",
                "veterano (50 anos)",
                "ancião (60 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
               "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction
function GetHarolothCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,50m-1,55m)",
                "médio-baixo (1,56m-1,60m)",
                "médio (1,61m-1,65m)",
                "médio-alto (1,66m-1,70m)",
                "alto (1,71m-1,75m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "esbelto (45kg-50kg)",
                "médio (51kg-55kg)",
                "atlético (56kg-60kg)",
                "robusto (61kg-65kg)",
                "musculoso (66kg-70kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "vermelhos brilhantes",
                "Violeta profundo",
                "âmbar dourado",
                "verde fluorescente",
                "azul elétrico"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem (75 anos)",
                "adulto (150 anos)",
                "meia-idade (250 anos)",
                "maduro (350 anos)",
                "ancião (400 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
               "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function GetAlurielCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,60m-1,65m)",
                "médio-baixo (1,66m-1,70m)",
                "médio (1,71m-1,75m)",
                "médio-alto (1,76m-1,80m)",
                "alto (1,81m-1,85m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "leve (45kg-50kg)",
                "médio-leve (51kg-55kg)",
                "médio (56kg-60kg)",
                "médio-pesado (61kg-65kg)",
                "pesado (66kg-70kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "azul celeste brilhante",
                "verde esmeralda profundo",
                "âmbar dourado cintilante",
                "violeta suave",
                "prateado luminoso"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem (75 anos)",
                "adulto (150 anos)",
                "meia-idade (350 anos)",
                "maduro (600 anos)",
                "ancião (700 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
               "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function GetGulthrakCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,70m-1,75m)",
                "médio-baixo (1,76m-1,80m)",
                "médio (1,81m-1,85m)",
                "médio-alto (1,86m-1,90m)",
                "alto (1,91m-1,95m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "magro (80kg-90kg)",
                "médio (91kg-100kg)",
                "musculoso (101kg-110kg)",
                "robusto (111kg-120kg)",
                "corpulento (121kg-130kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "vermelho brilhante",
                "âmbar profundo",
                "verde fluorescente",
                "preto como carvão",
                "branco leitoso"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem adulto (15 anos)",
                "adulto (25 anos)",
                "meia-idade (35 anos)",
                "veterano (45 anos)",
                "ancião (50 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
               "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function GetPolkineaCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "muito baixo (80cm-85cm)",
                "baixo (86cm-90cm)",
                "médio (91cm-95cm)",
                "alto (96cm-100cm)",
                "muito alto (101cm-110cm)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "leve (20kg-25kg)",
                "médio-leve (26kg-30kg)",
                "médio (31kg-35kg)",
                "médio-pesado (36kg-40kg)",
                "pesado (41kg-45kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "castanho amendoado",
                "verde brilhante",
                "azul profundo",
                "cinza prateado",
                "âmbar dourado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem adulto (25 anos)",
                "adulto (40 anos)",
                "meia-idade (55 anos)",
                "maduro (70 anos)",
                "ancião (90+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
                "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function GetPovoDoLesteCaracteristicas()
    var caracteristicas := struct{
        "Altura" := struct{
            "nome" := "Altura",
            "opcoes" := array{
                "baixo (1,55m-1,60m)",
                "médio-baixo (1,61m-1,65m)",
                "médio (1,66m-1,70m)",
                "médio-alto (1,71m-1,75m)",
                "alto (1,76m-1,80m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "esbelto (50kg-55kg)",
                "médio-leve (56kg-60kg)",
                "médio (61kg-65kg)",
                "médio-pesado (66kg-70kg)",
                "robusto (71kg-75kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "castanho escuro amendoado",
                "preto ônix brilhante",
                "castanho claro estreito",
                "cinza chumbo oblíquo",
                "âmbar dourado rasgado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "jovem adulto (25 anos)",
                "adulto (35 anos)",
                "meia-idade (50 anos)",
                "maduro (65 anos)",
                "ancião (75 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Tipo de Voz",
            "opcoes" := array{
                "aguda",
                "de notas médias",
                "grave"
            }
        }
    };
    return caracteristicas;
endfunction

function ExtractAgeValue(race, chosen_age_index)
    var racial_traits := GetRacialTraits(race);
    if (racial_traits == error)
        return error;
    endif

    var age_options := racial_traits["Idade Aparente"].opcoes;
    if (chosen_age_index < 1 || chosen_age_index > age_options.size())
        return error;
    endif

    var chosen_age_string := age_options[chosen_age_index];
    var age_value := 0;

    // Extrai o número entre parênteses
    var start := chosen_age_string.find("(");
    var end := chosen_age_string.find(" anos)");
    if (start != error && end != error)
        age_value := CInt(chosen_age_string[start+1, end-start-1]);
    endif

    return age_value;
endfunction