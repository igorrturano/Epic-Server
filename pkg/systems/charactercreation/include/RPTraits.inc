use uo;
use os;

include "include/client";
include "include/say";
include ":charactercreation:createchar";

function GetRacialTraits(culture)

    case (culture)
        BADURAN: return GetBaduranCaracteristicas();
        HAROLOTH: return GetHarolothCaracteristicas();
        ALURIEL: return GetAlurielCaracteristicas();
        GULTHRAK: return GetGulthrakCaracteristicas();
        BJORSKA: return GetBjorskeCaracteristicas();
        POLSKI: return GetPolkineaCaracteristicas();
        AIGLANA: return GetAiglanosCaracteristicas();
        LESTE: return GetPovoDoLesteCaracteristicas();

        default: return error; // Retorna error em vez de struct vazio
    endcase
endfunction

function TranslateRacialTrait(category, value, chardata := 0)
    var racial_traits := GetRacialTraits(chardata.culture);

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
                "Baixo (1,60m-1,65m)",
                "Médio-baixo (1,66m-1,70m)",
                "Médio (1,71m-1,75m)",
                "Médio-alto (1,76m-1,80m)",
                "Alto (1,81m-1,85m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Esbelto (55kg-65kg)",
                "Médio (66kg-75kg)",
                "Atlético (76kg-85kg)",
                "Robusto (86kg-95kg)",
                "Corpulento (96kg-105kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Caracteristica dos Olhos",
            "opcoes" := array{
                "Castanhos amendoados",
                "Verdes arredondados",
                "Azuis penetrantes",
                "Cinzentos estreitos",
                "Âmbar expressivos"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (20 anos)",
                "Adulto (30 anos)",
                "Meia-idade (40 anos)",
                "Maduro (50 anos)",
                "Idoso (60 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
                "Aguda",
                "de notas médias",
                "Grave"
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
                "Muito baixo (1,20m-1,25m)",
                "Baixo (1,26m-1,30m)",
                "Médio (1,31m-1,35m)",
                "Alto (1,36m-1,40m)",
                "Muito alto (1,41m-1,48m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Leve (50kg-60kg)",
                "Médio (61kg-70kg)",
                "Pesado (71kg-80kg)",
                "Muito pesado (81kg-90kg)",
                "Gordo (91kg-100kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Castanhos profundos",
                "Azuis brilhantes",
                "Verdes intensos",
                "Cinza metálico",
                "Âmbar dourado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (75 anos)",
                "Adulto (120 anos)",
                "Meia-idade (170 anos)",
                "Maduro (230 anos)",
                "Idoso (250 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "de notas médias",
                "Grave"
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
                "Baixo (1,65m-1,70m)",
                "Médio-baixo (1,71m-1,75m)",
                "Médio (1,76m-1,80m)",
                "Médio-alto (1,81m-1,85m)",
                "Alto (1,86m-1,90m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Esbelto (70kg-80kg)",
                "Médio (81kg-90kg)",
                "Atlético (91kg-100kg)",
                "Robusto (101kg-110kg)",
                "Corpulento (111kg-120kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Azuis",
                "Verdes",
                "Cinzentos",
                "Castanhos",
                "Heterocromia"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (20 anos)",
                "Adulto (30 anos)",
                "Meia-idade (40 anos)",
                "veterano (50 anos)",
                "Ancião (60 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "de notas médias",
                "Grave"
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
                "Baixo (1,50m-1,55m)",
                "Médio-baixo (1,56m-1,60m)",
                "Médio (1,61m-1,65m)",
                "Médio-alto (1,66m-1,70m)",
                "Alto (1,71m-1,75m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Esbelto (45kg-50kg)",
                "Médio (51kg-55kg)",
                "Atlético (56kg-60kg)",
                "Robusto (61kg-65kg)",
                "musculoso (66kg-70kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Vermelhos brilhantes",
                "Violeta profundo",
                "Âmbar dourado",
                "Verde fluorescente",
                "Azul elétrico"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (75 anos)",
                "Adulto (150 anos)",
                "Meia-idade (250 anos)",
                "Maduro (350 anos)",
                "Ancião (400 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "de notas médias",
                "Grave"
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
                "Baixo (1,60m-1,65m)",
                "Médio-baixo (1,66m-1,70m)",
                "Médio (1,71m-1,75m)",
                "Médio-alto (1,76m-1,80m)",
                "Alto (1,81m-1,85m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Leve (45kg-50kg)",
                "Médio-leve (51kg-55kg)",
                "Médio (56kg-60kg)",
                "Médio-pesado (61kg-65kg)",
                "Pesado (66kg-70kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Azul celeste brilhante",
                "Verde esmeralda profundo",
                "Âmbar dourado cintilante",
                "Violeta suave",
                "Prateado luminoso"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (75 anos)",
                "Adulto (150 anos)",
                "Meia-idade (350 anos)",
                "Maduro (600 anos)",
                "Ancião (700 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "de notas médias",
                "Grave"
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
                "Baixo (1,70m-1,75m)",
                "Médio-baixo (1,76m-1,80m)",
                "Médio (1,81m-1,85m)",
                "Médio-alto (1,86m-1,90m)",
                "Alto (1,91m-1,95m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "magro (80kg-90kg)",
                "Médio (91kg-100kg)",
                "musculoso (101kg-110kg)",
                "Robusto (111kg-120kg)",
                "Corpulento (121kg-130kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Vermelho escuro",
                "Âmbar profundo",
                "Verde escuro",
                "Preto como carvão",
                "Castanho escuro"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem adulto (15 anos)",
                "Adulto (25 anos)",
                "Meia-idade (35 anos)",
                "veterano (45 anos)",
                "Ancião (50 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "de notas médias",
                "Grave"
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
                "Muito baixo (80cm-85cm)",
                "Baixo (86cm-90cm)",
                "Médio (91cm-95cm)",
                "Alto (96cm-100cm)",
                "Muito alto (101cm-110cm)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Leve (20kg-25kg)",
                "Médio-leve (26kg-30kg)",
                "Médio (31kg-35kg)",
                "Médio-pesado (36kg-40kg)",
                "Pesado (41kg-45kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Castanho amendoado",
                "Verde brilhante",
                "Azul profundo",
                "Cinza prateado",
                "Âmbar dourado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem adulto (25 anos)",
                "Adulto (40 anos)",
                "Meia-idade (55 anos)",
                "Maduro (70 anos)",
                "Ancião (90+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
                "Aguda",
                "de notas médias",
                "Grave"
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
                "Baixo (1,55m-1,60m)",
                "Médio-baixo (1,61m-1,65m)",
                "Médio (1,66m-1,70m)",
                "Médio-alto (1,71m-1,75m)",
                "Alto (1,76m-1,80m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Esbelto (50kg-55kg)",
                "Médio-leve (56kg-60kg)",
                "Médio (61kg-65kg)",
                "Médio-pesado (66kg-70kg)",
                "Robusto (71kg-75kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Castanho escuro amendoado",
                "Preto ônix brilhante",
                "Castanho claro estreito",
                "Cinza chumbo oblíquo",
                "Âmbar dourado rasgado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem adulto (25 anos)",
                "Adulto (35 anos)",
                "Meia-idade (50 anos)",
                "Maduro (65 anos)",
                "Ancião (75 anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
                "Aguda",
                "de notas médias",
                "Grave"
            }
        }
    };
    return caracteristicas;
endfunction

function ExtractAgeValue(culture, chosen_age_index)
    var racial_traits := GetRacialTraits(culture);
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