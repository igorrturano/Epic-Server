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
            config.+name := "Caverna dos Goblins";
            config.+boss_template := "orcbrute_territorial";
            config.+cooldown := 3600;
            config.+player_radius := 60;
            
            // Configuração de 16 ondas
            config.+waves := array(
                // Ondas 1-4
                struct{"mobs" := array{"goblin", "goblin2"}, "amount" := 30},
                struct{"mobs" := array{"goblin", "goblin2"}, "amount" := 1}
                //struct{"mobs" := array{"goblin", "goblin2"}, "amount" := 1},
                //struct{"mobs" := array{"goblin", "goblin2"}, "amount" := 1},
                // Ondas 5-8
                //struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 1},
                //struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 1},
                //struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 1},
               // struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 1},
                // Ondas 9-12
              //  struct{"mobs" := array{"orcbrute"}, "amount" := 1},
              //  struct{"mobs" := array{"orcbrute"}, "amount" := 1},
               // struct{"mobs" := array{"orcbrute"}, "amount" := 1},
               // struct{"mobs" := array{"orcbrute"}, "amount" := 1},
                // Ondas 13-16
               // struct{"mobs" := array{"ettin", "troll"}, "amount" := 1},
               // struct{"mobs" := array{"ettin", "troll"}, "amount" := 1},
               // struct{"mobs" := array{"ettin", "troll"}, "amount" := 1},
               // struct{"mobs" := array{"ettin", "troll"}, "amount" := 1}
            );
        
        "covildasaranhas":
            config.+name := "Covil das Aranhas";
            config.+boss_template := "dridertitan_territorial";
            config.+cooldown := 7200;
            config.+player_radius := 60;
            
            // Configuração de 16 ondas
            config.+waves := array(
                // Ondas 1-4
                struct{"mobs" := array{"giantrat", "spiderling", "rat", "minibesouro"}, "amount" := 3},
                struct{"mobs" := array{"driderpoison", "dridersacerdotisa"}, "amount" := 2}
                //struct{"mobs" := array{"giantrat", "spiderling", "rat", "minibesouro"}, "amount" := 3},
                //struct{"mobs" := array{"giantrat", "spiderling", "rat", "minibesouro"}, "amount" := 3},
                //struct{"mobs" := array{"giantrat", "spiderling", "rat", "minibesouro"}, "amount" := 3},
                // Ondas 5-8
               //struct{ "mobs" := array{"drider", "drider2", "drider3"} , "amount" := 3 },
                //struct{ "mobs" := array{"drider", "drider2", "drider3"}, "amount" := 3 }, 
                //struct{ "mobs" := array{"drider", "drider2", "drider3"}, "amount" := 3 }, 
                //struct{ "mobs" := array{"drider", "drider2", "drider3"}, "amount" := 3 }, 
                // Ondas 9-12
                //struct{"mobs" := array{"drider3", "drider4"}, "amount" := 2 },
                //struct{"mobs" := array{"drider3", "drider4"}, "amount" := 2 },
                //struct{"mobs" := array{"drider3", "drider4"}, "amount" := 2 },
                //struct{"mobs" := array{"drider3", "drider4"}, "amount" := 2 },
                // Ondas 13-16
                //struct{"mobs" := array{"dridermatriarca", "dridermage"}, "amount" := 2},
                //struct{"mobs" := array{"dridermatriarca", "dridermage"}, "amount" := 2},
                //struct{"mobs" := array{"driderpoison", "dridersacerdotisa"}, "amount" := 2},
            );
            
        default:
            return error{"errortext" := "Tipo de spawn desconhecido: " + spawn_type};
    endcase
    
    return config;
endfunction