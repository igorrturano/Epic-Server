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
            config.+boss_template := "dreadspider_territorial";
            config.+cooldown := 7200;
            config.+player_radius := 60;
            
            // Configuração de 16 ondas
            config.+waves := array(
                // Ondas 1-4
                struct{"mobs" := array{"giantspider", "dreadspider"}, "amount" := 8},
                struct{"mobs" := array{"giantspider", "dreadspider"}, "amount" := 8},
                struct{"mobs" := array{"giantspider", "dreadspider"}, "amount" := 8},
                struct{"mobs" := array{"giantspider", "dreadspider"}, "amount" := 8},
                // Ondas 5-8
                struct{"mobs" := array{"terathanwarrior", "terathandrone"}, "amount" := 6},
                struct{"mobs" := array{"terathanwarrior", "terathandrone"}, "amount" := 6},
                struct{"mobs" := array{"terathanwarrior", "terathandrone"}, "amount" := 6},
                struct{"mobs" := array{"terathanwarrior", "terathandrone"}, "amount" := 6},
                // Ondas 9-12
                struct{"mobs" := array{"terathanavenger", "terathanmatriarch"}, "amount" := 5},
                struct{"mobs" := array{"terathanavenger", "terathanmatriarch"}, "amount" := 5},
                struct{"mobs" := array{"terathanavenger", "terathanmatriarch"}, "amount" := 5},
                struct{"mobs" := array{"terathanavenger", "terathanmatriarch"}, "amount" := 5},
                // Ondas 13-16
                struct{"mobs" := array{"ophidianknight", "ophidianmage"}, "amount" := 4},
                struct{"mobs" := array{"ophidianknight", "ophidianmage"}, "amount" := 4},
                struct{"mobs" := array{"ophidianknight", "ophidianmage"}, "amount" := 4},
                struct{"mobs" := array{"ophidianknight", "ophidianmage"}, "amount" := 4}
            );
            
        default:
            return error{"errortext" := "Tipo de spawn desconhecido: " + spawn_type};
    endcase
    
    return config;
endfunction