// CONFIG.INC - Funções de configuração do sistema Champion Spawn
// Criado durante refatoração para separar responsabilidades

use uo;
use os;
use cfgfile;
use util;

include ":faccao:include/shared_functions";

// FUNÇÃO CENTRALIZADA: Carrega configurações de spawn do settings.cfg
function LoadSpawnConfig(spawn_type)
    var cfg := ReadConfigFile(":spawndecampeao:config/settings");
    if (!cfg)
        LogError("ChampSpawn", "Erro: Não foi possível carregar settings.cfg");
        return error;
    endif
    
    var spawn_name;
    case (Lower(spawn_type))
        "cavernadosgoblins":
            spawn_name := "CavernaDosGoblins";
        "covildasaranhas":
            spawn_name := "CovilDasAranhas";
        default:
            LogError("ChampSpawn", "Tipo de spawn desconhecido: " + spawn_type);
            return error;
    endcase
    
    var spawn_cfg := FindConfigElem(cfg, spawn_name);
    if (!spawn_cfg)
        LogError("ChampSpawn", "Configuração não encontrada para: " + spawn_name);
        return error;
    endif
    
    var config := struct;
    config.+name := GetConfigString(spawn_cfg, "Name");
    config.+boss_template := GetConfigString(spawn_cfg, "BossTemplate");
    config.+cooldown := GetConfigInt(spawn_cfg, "ActivationCooldown");
    config.+player_radius := GetConfigInt(spawn_cfg, "PlayerCheckRadius");
    
    // Carregar waves
    var waves := array{};
    var i;
    for (i := 1; i <= 4; i := i + 1)
        var wave_mobs := GetConfigString(spawn_cfg, "Wave" + i + " Mobs");
        var wave_amount := GetConfigInt(spawn_cfg, "Wave" + i + " Amount");
        
        if (wave_mobs && wave_amount)
            var mob_list := SplitWords(wave_mobs);
            waves.append(struct{"mobs" := mob_list, "amount" := wave_amount});
        endif
    endfor
    
    config.+waves := waves;
    
    LogInfo("ChampSpawn", "Configuração carregada do settings.cfg - Spawn: " + spawn_name + 
           ", Boss: " + config.boss_template + ", Waves: " + len(waves));
    
    return config;
endfunction
