use uo;
use os;

include "include/client";
include "include/say";

function GetRacialTraits(raca)
    case (raca)
        ANAO: return GetBaduranCaracteristicas();
        DROW: return GetHarolothCaracteristicas();
        ELFO: return GetAlurielCaracteristicas();
        ORC: return GetGulthrakCaracteristicas();
        HUMANO: return GetBjorskeCaracteristicas();
        POLSKI: return GetPolkineaCaracteristicas();
        default: return error;
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
                "Castanhos",
                "Azuis",
                "Verdes",
                "Preto",
                "Âmbar"
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
                "Veterano (50 anos)",
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
                "Musculoso (66kg-75kg)"
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

function ConvertAge(race, age)
    age := CInt(age);
    var age_category := "";

    case (race)
        ANAO:
            if (age < 18)
                return array{"Jovem", 18};
            elseif (age <= 30)
                age_category := "Jovem";
            elseif (age <= 70)
                age_category := "Adulto";
            elseif (age <= 90)
                age_category := "Meia-idade";
            elseif (age < 120)
                age_category := "Idoso";
            else
                return array{"Idoso", 120};
            endif
        HUMANO:
            if (age < 18)
                return array{"Jovem", 18};
            elseif (age <= 30)
                age_category := "Jovem";
            elseif (age <= 40)
                age_category := "Adulto";
            elseif (age <= 50)
                age_category := "Meia-idade";
            elseif (age < 60)
                age_category := "Veterano";
            else
                return array{"Ancião", 60};
            endif
        DROW:
          if (age < 18)
                return array{"Jovem", 18};
            elseif (age <= 80)
                age_category := "Jovem";
            elseif (age <= 120)
                age_category := "Adulto";
            elseif (age <= 170)
                age_category := "Meia-idade";
            elseif (age < 200)
                age_category := "Maduro";
            else
                return array{"Ancião", 200};
            endif
        ELFO:
            if (age < 18)
                return array{"Jovem", 18};
            elseif (age <= 80)
                age_category := "Jovem";
            elseif (age <= 120)
                age_category := "Adulto";
            elseif (age <= 170)
                age_category := "Meia-idade";
            elseif (age < 200)
                age_category := "Maduro";
            else
                return array{"Ancião", 200};
            endif
        ORC:
            if (age < 18)
                return array{"Jovem adulto", 18};
            elseif (age <= 25)
                age_category := "Jovem adulto";
            elseif (age <= 35)
                age_category := "Adulto";
            elseif (age <= 45)
                age_category := "Meia-idade";
            elseif (age <= 50)
                age_category := "Veterano";
            elseif (age <= 75)
                age_category := "Ancião";
            else
                return array{"Ancião", 75};
            endif
        POLSKI:
            if (age < 18)
                return array{"Jovem", 18};
            elseif (age <= 65)
                age_category := "Jovem";
            elseif (age <= 85)
                age_category := "Adulto";
            elseif (age <= 105)
                age_category := "Meia-idade";
            elseif (age <= 125)
                age_category := "Maduro";
            elseif (age >= 125)
                age_category := "Ancião";
            else
                return array{"Ancião", 125};
            endif
    default:
            return array{"Erro", age};
    endcase

    return array{age_category, age};
endfunction