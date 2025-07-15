// CONFIG_SIMPLE.INC - Versão simplificada para resolver problemas de configuração
// Retorna configuração hard-coded para testes

// Função simplificada que retorna configuração hard-coded
function LoadSpawnConfig(spawn_type)
    var config := struct;
    
    if (spawn_type == "cavernadosgoblins" || spawn_type == "CavernaDosGoblins")
        config.+name := "Caverna dos Goblins";
        config.+boss_template := "orcbrute_territorial";
        config.+cooldown := 3600;
        config.+player_radius := 60;
        
        // Criar waves manualmente
        var waves := array{};
        waves.append(struct{"mobs" := array{"goblin", "goblin2"}, "amount" := 30});
        waves.append(struct{"mobs" := array{"hobgoblin", "hobgoblin2"}, "amount" := 20});
        waves.append(struct{"mobs" := array{"orcbrute", "orcbrute"}, "amount" := 15});
        waves.append(struct{"mobs" := array{"ettin", "troll"}, "amount" := 10});
        config.+waves := waves;
        
    elseif (spawn_type == "covildasaranhas" || spawn_type == "CovilDasAranhas")
        config.+name := "Covil das Aranhas";
        config.+boss_template := "dreadspider_territorial";
        config.+cooldown := 7200;
        config.+player_radius := 60;
        
        // Criar waves manualmente
        var waves := array{};
        waves.append(struct{"mobs" := array{"giantspider", "dreadspider"}, "amount" := 8});
        waves.append(struct{"mobs" := array{"terathanwarrior", "terathandrone"}, "amount" := 6});
        waves.append(struct{"mobs" := array{"terathanavenger", "terathanmatriarch"}, "amount" := 5});
        waves.append(struct{"mobs" := array{"ophidianknight", "ophidianmage"}, "amount" := 4});
        config.+waves := waves;
        
    else
        return error;
    endif
    
    return config;
endfunction
