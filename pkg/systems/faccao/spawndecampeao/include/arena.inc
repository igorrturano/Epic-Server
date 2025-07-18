// ARENA.INC - Funções para construção de arenas de Champion Spawn
// Criado durante refatoração para separar responsabilidades

use uo;
use os;
use util;

include ":faccao:include/shared_functions";

// === FUNÇÃO PRINCIPAL DE CONSTRUÇÃO DA ARENA ===

function BuildChampionEnvironment(center_x, center_y, center_z, realm)
    
    // Lista da Arena Externa
    var outer_arena := array {
        "0xC5 9 0 0 1", "0xC5 0 9 0 1", "0xC5 -9 0 0 1", "0xC5 0 -9 0 1",
        "0xC5 -6 -6 0 1", "0xC5 -6 6 0 1", "0xC5 6 6 0 1", "0xC5 6 -6 0 1",
        "0xDB -4 1 6 1", "0xDB -2 4 6 1", "0xDB 1 4 6 1", "0xDB 4 1 7 1",
        "0xDB 4 -2 6 1", "0xDB 1 -4 7 1", "0xDB -2 -4 6 1", "0xDB -4 -2 6 1",
        "0x1ECB -9 0 20 1", "0x1ECB -6 -6 20 1", "0x1ECB -6 6 20 1", "0x1ECB 0 -9 20 1",
        "0x1ECB 6 -6 20 1", "0x1ECB 9 0 20 1", "0x1ECB 6 6 20 1", "0x1ECB 0 9 20 1",
        "0x1ECB 4 -2 0 1", "0x1ECB 1 4 0 1", "0x1ECB -2 4 0 1", "0x1ECB -4 1 0 1",
        "0x1ECB -4 -2 0 1", "0x1ECB -2 -4 0 1", "0x1ECB 4 1 1 1", "0x1ECB 1 -4 1 1"
    };
    
    // Lista da Plataforma Central Elevada
    var center_platform := array {
        "0x1781 0 0 0 0", "0x49C 0 0 0 1", "0x51B 1 1 5 1", "0x51B 1 0 5 1",
        "0x51B 1 -1 5 1", "0x51B 0 1 5 1", "0x51B 0 0 5 1", "0x51B 0 -1 5 1",
        "0x51B -1 1 5 1", "0x51B -1 0 5 1", "0x51B -1 -1 5 1", "0x78D -2 -2 0 1",
        "0x78C -2 1 0 1", "0x78C -2 0 0 1", "0x78C -2 -1 0 1", "0x78B 1 -2 0 1",
        "0x78B 0 -2 0 1", "0x78B -1 -2 0 1", "0x78A 2 1 0 1", "0x78A 2 0 0 1",
        "0x78A 2 -1 0 1", "0x78F 2 -2 0 1", "0x78E 2 2 0 1", "0x789 1 2 0 1",
        "0x789 0 2 0 1", "0x789 -1 2 0 1", "0x790 -2 2 0 1", "0x788 1 1 0 1",
        "0x788 1 0 0 1", "0x788 1 -1 0 1", "0x788 0 1 0 1", "0x788 0 0 0 1",
        "0x788 0 -1 0 1", "0x788 -1 1 0 1", "0x788 -1 0 0 1", "0x788 -1 -1 0 1"
    };

    // COMBINA AS DUAS LISTAS EM UMA SÓ
    var design_lines := outer_arena + center_platform;

    var items_criados := 0;
    var erros := 0;

    foreach line in design_lines
        line := Trim(line);
        if (!line) continue; endif

        var parts := SplitWords(line);
        if (parts.size() < 4) 
            LogError("ChampSpawn", "Linha inválida no design da arena: '" + line + "'");
            continue; 
        endif

        var objtype := CInt(parts[1]);
        var x_offset := CInt(parts[2]);
        var y_offset := CInt(parts[3]);
        var z_offset := CInt(parts[4]);
        
        var final_x := center_x + x_offset;
        var final_y := center_y + y_offset;
        var final_z := center_z + z_offset;

        var item := CreateItemAtLocation(final_x, final_y, final_z, objtype, 1, realm);

        if (item)
            item.movable := 0; 
            items_criados += 1;
        else
            LogError("ChampSpawn", "Falha ao criar item da arena: " + Hex(objtype) + " em " + final_x + "," + final_y + "," + final_z);
            erros += 1;
        endif

        sleepms(2);
    endforeach

    if (erros > 0)
        LogWarning("ChampSpawn", "Construção da arena concluída com " + erros + " erros.");
    else
        LogInfo("ChampSpawn", "Arena construída com sucesso: " + items_criados + " itens criados");
    endif

    return items_criados;
endfunction
// === FUNÇÕES DE LIMPEZA DA ARENA ===

function CleanupArenaItems(center_x, center_y, center_z, realm)
    var items_removed := 0;
    var search_radius := 15;
    var z_range := 30;

    LogInfo("ChampSpawn_Cleanup", "Iniciando limpeza da arena em " + center_x + "," + center_y + "," + center_z + " (Raio: " + search_radius + ", Altura: +/-" + z_range + ")");

    for z_offset := -z_range to z_range
        var current_z := center_z + z_offset;
        var nearby_items := ListItemsNearLocation(center_x, center_y, current_z, search_radius, realm);
        
        foreach item in nearby_items
            var should_remove := 0;
            
            case (item.objtype)
                0xC5:
                0xDB:
                0x1ECB:
                0x1781:
                0x1822:
                0x1854: 
                0x142C: 
                0x49C:
                0x51B:
                0x78D:
                0x78C:
                0x78B:
                0x78A:
                0x78F:
                0x78E:
                0x789:
                0x790:
                0x788:
                4295:
                4296:
                4298:
                4299:
                    should_remove := 1;
            endcase
            
            if (should_remove)
                // ==========================================================
                // INÍCIO DA CORREÇÃO - REMOÇÃO DO TRY/CATCH
                // ==========================================================
                var result := DestroyItem(item);
                if (result.errortext)
                    LogWarning("ChampSpawn_Cleanup", "Falha ao destruir item da arena: " + item.desc + " (Serial: " + item.serial + ") - Erro: " + result.errortext);
                else
                    items_removed += 1;
                endif
                // ==========================================================
                // FIM DA CORREÇÃO
                // ==========================================================
            endif
        endforeach
        sleepms(2);
    endfor
    
    if (items_removed > 0)
        LogInfo("ChampSpawn_Cleanup", "Limpeza da arena concluída: " + items_removed + " itens removidos.");
    endif
    
    return items_removed;
endfunction

// Adicionar ao final de pkg/spawndecampeao/include/arena.inc

/*
 * RebuildChampionSpawn(territory_id)
 *
 * Purpose: Reconstrói a arena e o altar de um Champion Spawn em um território que voltou
 *          ao estado selvagem ou cujo totem expirou.
 * Parameters:
 *   territory_id - O ID único do território onde o spawn será reconstruído.
 * Return:
 *   1 em caso de sucesso, 0 em caso de falha.
 */
function RebuildChampionSpawn(territory_id)
    // 1. Encontrar as informações do território para obter a localização e o tipo de champion
    var territory_elem := TM_FindTerritory(territory_id);
    if (!territory_elem)
        LogError("RebuildChampion", "Território " + territory_id + " não encontrado. Reconstrução abortada.");
        return 0;
    endif

    // 2. Garantir que o território está marcado como Selvagem (segurança)
    SetTerritoryFaction(territory_elem, DEFAULT_FACTION);

    // 3. Obter as informações necessárias para a reconstrução
    var champion_type := GetElementProp(territory_elem, "ChampionType");
    var territory_name := GetElementProp(territory_elem, "Name");
    var center_info := TM_GetTerritoryCenter(territory_id);

    if (!champion_type || !territory_name || !center_info)
        LogError("RebuildChampion", "Dados incompletos para reconstrução do território " + territory_id + ". Faltando ChampionType, Name ou Center.");
        return 0;
    endif
    
    // 4. Carregar a configuração do spawn
    var altar_config := LoadSpawnConfig(champion_type);
    if (altar_config.errortext)
        LogError("RebuildChampion", "Configuração de spawn não encontrada para: " + champion_type + ". Reconstrução abortada.");
        return 0;
    endif

    // 5. Limpar qualquer resquício da arena antiga (segurança extra) e construir a nova
    LogInfo("RebuildChampion", "Limpando e construindo novo ambiente para " + territory_name);
    CleanupArenaItems(center_info.x, center_info.y, center_info.z, center_info.realm);
    var build_result := BuildChampionEnvironment(center_info.x, center_info.y, center_info.z, center_info.realm);
    if (!build_result)
        LogError("RebuildChampion", "Falha ao construir ambiente do champion para " + territory_name);
        return 0;
    endif
    
    // 6. Criar o novo altar na posição elevada
    LogInfo("RebuildChampion", "Criando novo altar para " + territory_name);
    var altar := CreateItemAtLocation(center_info.x, center_info.y, center_info.z + 5, 0x1822, 1, center_info.realm);
    if (!altar)
        LogError("RebuildChampion", "FALHA CRÍTICA: Falha ao criar o item do altar para " + territory_name);
        return 0;
    endif

    // 7. Configurar o novo altar
    SetObjProperty(altar, "ChampionConfig", altar_config);
    SetObjProperty(altar, "TerritoryName", territory_name);
    SetObjProperty(altar, "IsTerritorChampion", 1);

    LogInfo("RebuildChampion", "Champion Spawn para o território '" + territory_name + "' foi reconstruído e está pronto para ser ativado.");
    return 1;
endfunction