// =====================================================================
// ARQUIVO: config.inc (VERSÃO HARD-CODED COM 16 ONDAS)
// Esta função retorna a configuração de spawn diretamente, sem
// ler arquivos externos, para garantir estabilidade.
// =====================================================================
use uo;
use os;
use util;

function LoadSpawnConfig(spawn_type)
    var config := struct;
    
    // Usamos Lower() para garantir que a comparação não falhe por causa de maiúsculas/minúsculas.
    case (Lower(spawn_type))
        "cavernadosgoblins":
            config.+name := "Area Inicial Superficie";
            config.+boss_template := "urgach";
            config.+cooldown := 3600;
            config.+player_radius := 60;
            
            // Configuração de 16 ondas
            config.+waves := array(
                // Ondas 1-4
                struct{"mobs" := array{"rat", "giantrat"}, "amount" := 30},
                struct{"mobs" := array{"goblin"}, "amount" := 30},
                struct{"mobs" := array{"goblin", "goblinarcher"}, "amount" := 30},
                struct{"mobs" := array{"goblin", "goblin2", "goblinarcher"}, "amount" := 30},
                // Ondas 5-8
                struct{"mobs" := array{"hobgoblin"}, "amount" := 15},
                struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 15},
                struct{"mobs" := array{"hobgoblin", "hobgoblin2", "hobgoblin3"}, "amount" := 15},
               struct{"mobs" := array{"hobgoblin2", "hobgoblin3"}, "amount" := 15},
                // Ondas 9-12
               struct{"mobs" := array{"varag"}, "amount" := 10},
               struct{"mobs" := array{"varag", "varag2"}, "amount" := 10},
               struct{"mobs" := array{"varag2", "varag3"}, "amount" := 10},
               struct{"mobs" := array{"varag3", "varag4"}, "amount" := 10},
                // Ondas 13-16
               struct{"mobs" := array{"gnoll"}, "amount" := 5},
               struct{"mobs" := array{"gnoll", "gnoll2"}, "amount" := 5},
               struct{"mobs" := array{"bugbear", "gnoll2"}, "amount" := 5},
               struct{"mobs" := array{"gnoll3", "orog2"}, "amount" := 5}
            );
        
        "covildasaranhas":
            config.+name := "Area Underdark";
            config.+boss_template := "megaspider";
            config.+cooldown := 7200;
            config.+player_radius := 60;
            
            // Configuração de 16 ondas
            config.+waves := array(
                // Ondas 1-42intere
                struct{"mobs" := array{"giantrat", "spiderling", "rat", "minibesouro"}, "amount" := 30},
                struct{"mobs" := array{"giantrat", "spiderling", "giantbat", "drider"}, "amount" := 30},
                struct{"mobs" := array{"giantrat", "spiderling", "drider", "giantbat"}, "amount" := 30},
                struct{"mobs" := array{"giantrat", "spiderling", "giantbat"}, "amount" := 30},
                struct{"mobs" := array{"spiderling"}, "amount" := 30},
                //Ondas 5-8
               struct{ "mobs" := array{"uivante", "drider", "spiderling"} , "amount" := 15 },
                struct{ "mobs" := array{"uivante", "drider", "spiderling"}, "amount" := 15 }, 
                struct{ "mobs" := array{"uivante", "drider", "spiderling"}, "amount" := 15 }, 
                struct{ "mobs" := array{"uivante", "drider", "spiderling"}, "amount" := 15 }, 
                //Ondas 9-12
                struct{"mobs" := array{"drider", "aranhabrutal", "blackpudim"}, "amount" := 10 },
                struct{"mobs" := array{"drider", "aranhabrutal", "frog2"}, "amount" := 10 },
                struct{"mobs" := array{"drider", "aranhabrutal", "quelino"}, "amount" := 10 },
                struct{"mobs" := array{"drider", "aranhabrutal", "blackpudim2"}, "amount" := 10 },
                //Ondas 13-16
                struct{"mobs" := array{"gholam", "vexhai"}, "amount" := 5},
                struct{"mobs" := array{"vorghul", "drider2"}, "amount" := 5},
                struct{"mobs" := array{"drider2", "drider2"}, "amount" := 5}
            );
            
        default:
            return error{"errortext" := "Tipo de spawn desconhecido: " + spawn_type};
    endcase
    
    return config;
endfunction