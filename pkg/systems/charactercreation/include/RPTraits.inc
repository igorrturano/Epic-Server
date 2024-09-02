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
    print("GetRacialTraits chamado com raça: " + race);
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
            print("Erro: Raça desconhecida '" + race + "'");
            return error; // Retorna error em vez de struct vazio
    endcase
endfunction

function RacaToString(raca)
    case (raca)
        ANAO: return "Anão";
        DROW: return "Drow";
        ELFO: return "Elfo";
        ORC: return "Orc";
        HUMANO: return "Humano";
        POLSKI: return "Polski";
        default: return "Desconhecido";
    endcase
endfunction

function TranslateRacialTrait(race, category, value, chardata := 0)
   print("Raça do personagem: " + chardata.raca);
print("Características RP armazenadas: " + chardata.caracteristicas_rp);
var racial_traits := GetRacialTraits(chardata.raca, chardata);
print("Traços raciais obtidos: " + racial_traits);

if (racial_traits == error)
    print("Erro ao obter traços raciais para a raça: " + chardata.raca);
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
                "jovem (20-30 anos)",
                "adulto (31-40 anos)",
                "meia-idade (41-50 anos)",
                "maduro (51-60 anos)",
                "idoso (61+ anos)"
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
                "jovem (50-100 anos)",
                "adulto (101-150 anos)",
                "meia-idade (151-200 anos)",
                "maduro (201-250 anos)",
                "idoso (251+ anos)"
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
                "jovem (20-30 anos)",
                "adulto (31-40 anos)",
                "meia-idade (41-50 anos)",
                "veterano (51-60 anos)",
                "ancião (61+ anos)"
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
                "jovem (50-100 anos)",
                "adulto (101-200 anos)",
                "meia-idade (201-300 anos)",
                "maduro (301-400 anos)",
                "ancião (401+ anos)"
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
                "jovem (50-100 anos)",
                "adulto (101-200 anos)",
                "meia-idade (201-500 anos)",
                "maduro (501-700 anos)",
                "ancião (701+ anos)"
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
                "jovem adulto (15-20 anos)",
                "adulto (21-30 anos)",
                "meia-idade (31-40 anos)",
                "veterano (41-50 anos)",
                "ancião (51+ anos)"
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
                "jovem adulto (20-30 anos)",
                "adulto (31-50 anos)",
                "meia-idade (51-70 anos)",
                "maduro (71-90 anos)",
                "ancião (91+ anos)"
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
                "jovem adulto (20-30 anos)",
                "adulto (31-45 anos)",
                "meia-idade (46-60 anos)",
                "maduro (61-75 anos)",
                "ancião (76+ anos)"
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