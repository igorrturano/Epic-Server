use uo;
use os;
use cfgfile;

include ":loot:lootParser";
include ":traps:traps";
include "include/client";
include "include/say";
include ":timedscripts:timedScripts";
include ":destroyable:include/destroyItem";
include ":IO:CFGFile";

// ========================================
// CONFIG FUNCTIONS - Using CFGFile Class
// ========================================

// Cache global do dicionário de baús (carregado uma vez)
var _chest_cache := 0;

// Carrega e retorna o dicionário de baús do arquivo chests.cfg
function LoadChestsConfig()
    // Se já tiver em cache, retorna direto (performance)
    if (_chest_cache)
        return _chest_cache;
    endif
    
    // Método tradicional: ReadConfigFile + GetConfigStringKeys
    var cfg_file := ReadConfigFile(":chestspawn:config/chests");
    if (!cfg_file)
        SysLog("ERROR: Failed to load chests.cfg file");
        return error;
    endif
    
    var chest_names := GetConfigStringKeys(cfg_file);
    if (!chest_names || chest_names.size() == 0)
        SysLog("ERROR: No chest entries found in chests.cfg");
        return error;
    endif
    
    // Cria dictionary manualmente
    var chests := dictionary;
    foreach chest_name in chest_names
        var elem := FindConfigElem(cfg_file, chest_name);
        if (elem)
            var chest_struct := struct{
                "Graphic" := GetConfigInt(elem, "Graphic"),
                "Material" := GetConfigInt(elem, "Material"),
                "MaxHp" := GetConfigInt(elem, "MaxHp"),
                "Name" := GetConfigString(elem, "Name")
            };
            chests[chest_name] := chest_struct;
        endif
    endforeach
    
    if (chests.size() == 0)
        SysLog("ERROR: Empty chest dictionary after processing config");
        return error;
    endif
    
    // Armazena em cache para próximas chamadas
    _chest_cache := chests;
    return chests;
endfunction

// Retorna informações de um baú aleatório do arquivo de configuração
function GetRandomChestFromConfig()
    var chests := LoadChestsConfig();
    if (chests == error)
        // Fallback para baú padrão se config falhar
        return struct{ 
            "graphic" := 3650, 
            "material" := 0x1bd7, 
            "maxhp" := 30 
        };
    endif
    
    var chest_keys := chests.keys();
    if (!chest_keys || chest_keys.size() == 0)
        // Fallback se não houver tipos
        return struct{ 
            "graphic" := 3650, 
            "material" := 0x1bd7, 
            "maxhp" := 30 
        };
    endif
    
    var random_key := chest_keys[RandomInt(chest_keys.size()) + 1];
    var chest_data := chests[random_key];
    
    return struct{
        "graphic" := CInt(chest_data.Graphic),
        "material" := CInt(chest_data.Material),
        "maxhp" := CInt(chest_data.MaxHp),
        "name" := CStr(chest_data.Name)
    };
endfunction

// Retorna informações de um baú específico pelo gráfico
function GetChestInfoByGraphic(graphic)
    var chests := LoadChestsConfig();
    if (chests == error)
        // Fallback baseado no gráfico
        var material := 0x1bd7; // madeira por padrão
        if (graphic == 3648 || graphic == 3708)
            material := 0x6995; // metal
        endif
        return struct{ 
            "graphic" := graphic, 
            "material" := material, 
            "maxhp" := 30,
            "name" := "Baú"
        };
    endif
    
    // Procura no dicionário pelo gráfico (acesso direto, sem loop!)
    foreach key in (chests.keys())
        var chest_data := chests[key];
        if (CInt(chest_data.Graphic) == graphic)
            return struct{
                "graphic" := graphic,
                "material" := CInt(chest_data.Material),
                "maxhp" := CInt(chest_data.MaxHp),
                "name" := CStr(chest_data.Name)
            };
        endif
    endforeach
    
    // Se não encontrou, retorna fallback
    var material := 0x1bd7; // madeira por padrão
    if (graphic == 3648 || graphic == 3708)
        material := 0x6995; // metal
    endif
    return struct{ 
        "graphic" := graphic, 
        "material" := material, 
        "maxhp" := 30,
        "name" := "Baú"
    };
endfunction

// Retorna array de gráficos filtrados por prefixo de categoria
function GetChestGraphicsByCategory(category_prefix)
    var chests := LoadChestsConfig();
    if (chests == error)
        return array{}; // Return empty array on error
    endif
    
    var graphics := array{};
    var prefix_len := len(category_prefix);
    
    foreach key in (chests.keys())
        // Filtra por prefixo do nome (ex: "Metal_", "Wooden_", "Barrel_")
        // Verifica se a chave tem tamanho suficiente E começa com o prefixo
        if (len(key) >= prefix_len)
            var key_prefix := key[1, prefix_len];
            if (key_prefix == category_prefix)
                var chest_data := chests[key];
                graphics.append(CInt(chest_data.Graphic));
            endif
        endif
    endforeach
    
    return graphics;
endfunction

// Retorna todos os gráficos do cfg (para compatibilidade)
function GetAllChestGraphicsFromConfig()
    var chests := LoadChestsConfig();
    if (chests == error)
        return array{};
    endif
    
    var graphics := array{};
    foreach key in (chests.keys())
        var chest_data := chests[key];
        graphics.append(CInt(chest_data.Graphic));
    endforeach
    
    return graphics;
endfunction

// ========================================
// LEGACY FUNCTION: Wrapper para GetAllChestGraphicsFromConfig()
// DEPRECATED: Use GetAllChestGraphicsFromConfig() diretamente
// Mantido apenas para compatibilidade com código legado
// ========================================
function GetAllChestGraphics()
    // Agora usa o cfg dinamicamente (291 tipos) ao invés de 271 hardcoded
    return GetAllChestGraphicsFromConfig();
endfunction

// ========================================
// Funções LEGACY (compatibilidade) - Agora usam CFGFile internamente
// ========================================

/*
 * GetChestConfig()
 * DEPRECATED: Use LoadChestsConfig() para acesso direto ao dictionary
 * Mantido para compatibilidade com código legado
 */
function GetChestConfig()
    return LoadChestsConfig();
endfunction

/*
 * GetRandomChestType()
 * Retorna um tipo de baú aleatório das 5 opções básicas
 * DEPRECATED: Use GetRandomChestFromConfig() que retorna o struct completo
 */
function GetRandomChestType()
    var chest_types := array{
        "Wooden_0E42",  // WoodenChestEast
        "Wooden_0E43",  // WoodenChestSouth
        "Barrel_0E77",  // WoodenBarrel
        "Metal_0E40",   // MetalChestEast
        "Metal_0E7C"    // MetalChestSouth
    };
    return chest_types[RandomInt(chest_types.size()) + 1];
endfunction

/*
 * GetChestInfo(chest_type)
 * Retorna informações de um tipo de baú pelo nome da chave no cfg
 * Returns struct{ graphic, material, maxhp, name } ou error
 * Agora usa CFGFile internamente
 */
function GetChestInfo(chest_type)
    var chests := LoadChestsConfig();
    if (chests == error)
        return error{"errortext" := "Could not load chests.cfg"};
    endif
    
    // Busca diretamente no dictionary
    if (!chests.exists(chest_type))
        return error{"errortext" := "Chest type '" + chest_type + "' not found in config"};
    endif
    
    var chest_data := chests[chest_type];
    
    var info := struct;
    info.+graphic := CInt(chest_data.Graphic);
    info.+material := CInt(chest_data.Material);
    info.+maxhp := CInt(chest_data.MaxHp);
    info.+name := CStr(chest_data.Name);
    
    return info;
endfunction

/*
 * CreateTreasureChests(lvl, amt, rng, x, y, z, realm, lootname)
 * Função para criar baús do tesouro com armadilhas e loot
 */
function CreateTreasureChests(lvl, amt, rng, x, y, z, realm, lootname)
    var newx, newy, newz, checker, type, lootgroup, chest;
    var arraychest := array{};
        
    if (x == error)
        return arraychest;
    endif
    
    if(rng > 10)
        rng := 10;
    endif
    
    var total := 0;
    while(total < amt)
        chest := 0;
        checker := 0;
        var tries := 0;
        
        while(checker == 0)
            newx := cint(x + RandomInt(rng) - RandomInt(rng));
            newy := cint(y + RandomInt(rng) - RandomInt(rng));
            newz := GetWorldHeight(x, y, realm);
            
            if (z > newz+5)
                newz := z;
            elseif(z < newz-5)
                newz := z;
            endif
            
            if(len(ListItemsNearLocation(newx, newy, newz, 0, realm)) == 0)
                // Get random chest type from config
                var chest_type := GetRandomChestType();
                var chestStruct := GetChestInfo(chest_type);
                
                // Fallback to hardcoded if config fails
                if (chestStruct.errortext)
                    chestStruct := struct;
                    var type := RandomInt(4) + 1;
                    case(type)
                        1: chestStruct.+gfx := 3650; chestStruct.+maxHp := 30; chestStruct.+material := 0x1bd7;
                        2: chestStruct.+gfx := 3651; chestStruct.+maxHp := 30; chestStruct.+material := 0x1bd7;
                        3: chestStruct.+gfx := 3703; chestStruct.+maxHp := 22; chestStruct.+material := 0x1bd7;
                        4: chestStruct.+gfx := 3648; chestStruct.+maxHp := 45; chestStruct.+material := 0x6995;
                        5: chestStruct.+gfx := 3708; chestStruct.+maxHp := 45; chestStruct.+material := 0x6995;
                    endcase
                endif
                
                chest := CreateItemAtLocation(newx, newy, newz, chestStruct.graphic, 1, realm);
            if(chest)
        checker := 1;
        chest.movable := 0;
                
        // Configurações do baú
        SetObjProperty(chest, "IsTreasureChest", 1);
        SetObjProperty(chest, "CreatedOn", ReadGameClock());
        SetObjProperty(chest, PROPNAME_DISPENSAVEL_FLAG, true);
        SetObjProperty(chest, PROPNAME_DURABILIDADE_MAX, chestStruct.maxHp * lvl);
        SetObjProperty(chest, PROPNAME_MATERIAL, chestStruct.material);
        SetObjProperty(chest, "level", lvl);
        
        if (GetObjProperty(chest, "link"))
            SetObjProperty(chest, "IsSpawned", 1);
        else
            SetObjProperty(chest, "IsPlayerCreated", 1);
        endif
        
        chest.locked := 1;
        chest.movable := 0;

        // Add traps using centralized function
        AddRandomTraps(chest, lvl);

                    Loot_Generate(chest, lootname);
                else
                    checker := 0;
                    tries := tries + 1;
                endif
            endif
            
            if (tries > 30)
                var players := EnumerateOnlineCharacters();
                foreach player in players
                    if (player.cmdlevel > 2)
                        SendSysMessageEx(player, "Chestspawner em x " + x + " y " + y + " z " + z + " esta consumindo muito a CPU, e sera destruido", SSM_FAIL);
                        DestroyItem(chest);
                        sleepms(20);
                    endif
                endforeach
            endif
            sleep(1);
        endwhile
        
        if(chest)
            arraychest.append(chest);
            total := total + 1;
        endif
        sleep(10);
    endwhile
    
    return arraychest;
endfunction

// Adds random traps to a chest based on level
// Centralized trap logic - used by both CreateTreasureChests and epicchestspawner
function AddRandomTraps(chest, lvl)
    if (lvl <= 3)
        // Levels 1-3: 1 trap
        var trap := RandomInt(3) + 1;
        case(trap)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
    elseif (lvl == 4)
        // Level 4: 1 trap, 50% chance for 2nd trap
        var trap := RandomInt(3) + 1;
        case(trap)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
        if (RandomInt(100) < 50)
            var trap2 := RandomInt(3) + 1;
            case(trap2)
                1: AddTrap(chest, "Needle", lvl);
                2: AddTrap(chest, "Explosion", lvl);
                3: AddTrap(chest, "PoisonGas", lvl);
            endcase
        endif
    elseif (lvl == 5)
        // Level 5: 2 traps, 50% chance for 3rd trap
        var trap1 := RandomInt(3) + 1;
        case(trap1)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
        var trap2 := RandomInt(3) + 1;
        case(trap2)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
        if (RandomInt(100) < 50)
            var trap3 := RandomInt(3) + 1;
            case(trap3)
                1: AddTrap(chest, "Needle", lvl);
                2: AddTrap(chest, "Explosion", lvl);
                3: AddTrap(chest, "PoisonGas", lvl);
            endcase
        endif
    else
        // Level 6+: 3 traps always
        var trap1 := RandomInt(3) + 1;
        case(trap1)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
        var trap2 := RandomInt(3) + 1;
        case(trap2)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
        var trap3 := RandomInt(3) + 1;
        case(trap3)
            1: AddTrap(chest, "Needle", lvl);
            2: AddTrap(chest, "Explosion", lvl);
            3: AddTrap(chest, "PoisonGas", lvl);
        endcase
    endif
endfunction