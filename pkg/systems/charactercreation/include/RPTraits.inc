use uo;
use os;

include "include/client";
include "include/say";
include ":charactercreation:createchar";

function GetRacialTraits(raca)
    case (raca)
        ANAO: return GetBaduranCaracteristicas();
        DROW: return GetHarolothCaracteristicas();
        ELFO: return GetAlurielCaracteristicas();
        ORC: return GetGulthrakCaracteristicas();
        HUMANO: return GetBjorskeCaracteristicas();
        POLSKI: return GetPolkineaCaracteristicas();
        default: return error; // Retorna error em vez de struct vazio
    endcase
endfunction

function TranslateRacialTrait(category, value, chardata := 0)
    var racial_traits := GetRacialTraits(chardata.raca);

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
                "Jovem (30 anos)",
                "Adulto (50 anos)",
                "Meia-idade (70 anos)",
                "Maduro (90 anos)",
                "Idoso (120+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "De notas médias",
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
                "De notas médias",
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
                "musculoso (66kg-75kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Vermelhos",
                "Violeta",
                "Dourado",
                "Verde",
                "Azul"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (40 anos)",
                "Adulto (80 anos)",
                "Meia-idade (120 anos)",
                "Maduro (170 anos)",
                "Ancião (200+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "De notas médias",
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
                "Leve (45kg-50kg)",
                "Médio-leve (51kg-55kg)",
                "Médio (56kg-60kg)",
                "Médio-pesado (61kg-65kg)",
                "Pesado (66kg-75kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Azul",
                "Verde",
                "Castanho",
                "Preto",
                "Prateado"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem (40 anos)",
                "Adulto (80 anos)",
                "Meia-idade (120 anos)",
                "Maduro (170 anos)",
                "Ancião (200+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "De notas médias",
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
                "Baixo (1,80m-1,85m)",
                "Médio-baixo (1,86m-1,90m)",
                "Médio (1,91m-2m)",
                "Médio-alto (2,01m-2,15m)",
                "Alto (2,16m-2,30m)"
            }
        },
        "Peso" := struct{
            "nome" := "Peso",
            "opcoes" := array{
                "Magro (80kg-90kg)",
                "Médio (91kg-100kg)",
                "Musculoso (101kg-110kg)",
                "Robusto (111kg-120kg)",
                "Corpulento (121kg-130kg)"
            }
        },
        "Olhos" := struct{
            "nome" := "Características dos Olhos",
            "opcoes" := array{
                "Vermelho",
                "Amarelo",
                "Verde",
                "Preto",
                "Castanho"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem adulto (18 anos)",
                "Adulto (25 anos)",
                "Meia-idade (35 anos)",
                "Veterano (45 anos)",
                "Ancião (50+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
               "Aguda",
                "De notas médias",
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
                "Castanho",
                "Verde",
                "Azul",
                "Preto",
                "Mel"
            }
        },
        "Idade Aparente" := struct{
            "nome" := "Longevidade",
            "opcoes" := array{
                "Jovem adulto (45 anos)",
                "Adulto (65 anos)",
                "Meia-idade (85 anos)",
                "Maduro (105 anos)",
                "Ancião (125+ anos)"
            }
        },
        "Voz" := struct{
            "nome" := "Timbre de Voz",
            "opcoes" := array{
                "Aguda",
                "De notas médias",
                "Grave"
            }
        }
    };
    return caracteristicas;
endfunction


function ExtractAgeValue(raca, chosen_age_index)
    var racial_traits := GetRacialTraits(raca);
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