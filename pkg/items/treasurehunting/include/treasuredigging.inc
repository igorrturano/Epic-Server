use uo;
use os;

include "include/client";
include "include/say";
include ":timedScripts:timedScripts";
include ":chestspawn:treasurechest";
include ":treasurehunting:treasureguardians"; 
include ":treasurehunting:treasuremaps";
include ":treasurehunting:diggingEffects";

const TREASURE_CHEST_DURATION := 600;

/*
 * Verifica se o jogador se moveu da posição inicial
 */
function HasPlayerMoved(who, initial_x, initial_y)
    return (who.x != initial_x || who.y != initial_y);
endfunction



exported function StartChestTimer(chest)
    // Remove qualquer timer existente
    TS_LowerDuration(chest, "treasure_chest_destroy", -1);
    
    // Inicia o novo timer
    var timer_result := TS_StartTimer(chest, "treasure_chest_destroy", TREASURE_CHEST_DURATION);
  
    return timer_result;
endfunction

exported function DigForMap(who, targ, map)
  
    if (!reserveItem(map))
        return 0;
    endif
    if (GetObjProperty(map, "Completed"))
        return 0;
    endif
    
    if (!ValidateMapDistance(who, targ, map))
        return 0;
    endif

    var result := StartTreasureDigging(who, targ, map);
    
    if (result)
        SetObjProperty(map, "Completed", ReadGameClock());
        SetObjProperty(map, "CompletedBy", who.serial);
        SetName(map, "Mapa do Tesouro Nível " + GetObjProperty(map, "level") + " [Desvendado por " + who.name + "]");
    endif
    
    return result;
endfunction

exported function StartTreasureDigging(who, targ, map)
    if (!who || !targ || !map)
        return 0;
    endif

    // Registra posição inicial
    var initial_x := who.x;
    var initial_y := who.y;
    
    PrintTextAbove(who, "*Começa a cavar o tesouro*");
    SendSysMessageEx(who, "Permaneça parado enquanto cava. Se você se mover, precisará recomeçar.", SSM_INFO);
    
    var parts := array{};
    var maplevel := CInt(GetObjProperty(map, "level"));
    var loot := "treasuremap" + maplevel;
    
    // O baú começa bem enterrado
    var initial_z := who.z - 9;
    var current_z := initial_z;
    
    var chest := CreateTreasureChests(maplevel, 1, 0, targ.x, targ.y, initial_z, who.realm, loot)[1];
    if (!chest)
        return 0;
    endif
    
    // Adicione estas linhas aqui
SetGlobalProperty("last_chest_x", targ.x);
SetGlobalProperty("last_chest_y", targ.y);

    // Configuração do baú
    SetObjProperty(chest, "CreatedOn", ReadGameClock());
    SetObjProperty(chest, "DestroyAt", ReadGameClock() + 120);
    SetObjProperty(chest, "MapLevel", maplevel);
    SetObjProperty(chest, "FoundBy", who.serial);
    
    chest.movable := 0;
    
    // Inicia o script de destruição
    var script := start_script(":treasurehunting:chest/destroy", chest);
    if (!script)
        DestroyItem(chest);
        return 0;
    endif
    
    // Inicia o timer do baú
    if (!StartChestTimer(chest))
        DestroyItem(chest);
        return 0;
    endif
    
    var i := 1;
    while (i <= 9)
        if (HasPlayerMoved(who, initial_x, initial_y))
            return CleanupDigging(parts, chest, who);
        endif
        
        // Primeira parte do case - mensagens e efeitos
        case (i)
            1: 
                PrintTextAbove(who, "*Começa a cavar*");
            2: 
                PrintTextAbove(who, "*Continua cavando*");
                var patch1 := CreateItemAtLocation(targ.x, targ.y, current_z, 0x0914, 1, who.realm);
                SetupDirtPatch(patch1);
                parts.append(patch1);
            3: 
                PrintTextAbove(who, "*Cavando profundamente*");
                var patch2 := CreateItemAtLocation(targ.x-1, targ.y, current_z, 0x0914, 1, who.realm);
                var patch3 := CreateItemAtLocation(targ.x+1, targ.y, current_z, 0x0914, 1, who.realm);
                SetupDirtPatch(patch2);
                SetupDirtPatch(patch3);
                parts.append(patch2);
                parts.append(patch3);
            4: 
                PrintTextAbove(who, "*A terra cede*");
                var patch4 := CreateItemAtLocation(targ.x, targ.y-1, current_z, 0x0914, 1, who.realm);
                var patch5 := CreateItemAtLocation(targ.x, targ.y+1, current_z, 0x0914, 1, who.realm);
                SetupDirtPatch(patch4);
                SetupDirtPatch(patch5);
                parts.append(patch4);
                parts.append(patch5);
            5: 
                PrintTextAbove(who, "*A terra começa a ceder*");
                if (!HasPlayerMoved(who, initial_x, initial_y))
                    chest.movable := 1;
                    MoveObjectToLocation(chest, chest.x, chest.y, current_z, who.realm, MOVEOBJECT_FORCELOCATION);
                    chest.movable := 0;
                endif
            6:
                PrintTextAbove(who, "*O tesouro está surgindo*");
                var offsets := array{};
                offsets.append(array{-1,-1});
                offsets.append(array{1,-1});
                offsets.append(array{-1,1});
                offsets.append(array{1,1});
                foreach offset in offsets
                    var patch := CreateItemAtLocation(targ.x + offset[1], targ.y + offset[2], current_z, DIRT_BASIC_1 + RandomInt(2), 1, who.realm);
                    SetupDirtPatch(patch);
                    parts.append(patch);
                endforeach
            7:
                PrintTextAbove(who, "*Algo está surgindo*");
                var final_offsets := array{};
                final_offsets.append(array{-2,0});
                final_offsets.append(array{2,0});
                final_offsets.append(array{0,-2});
                final_offsets.append(array{0,2});
                foreach offset in final_offsets
                    var patch := CreateItemAtLocation(targ.x + offset[1], targ.y + offset[2], current_z, DIRT_DECOR_1 + RandomInt(4), 1, who.realm);
                    SetupDirtPatch(patch);
                    parts.append(patch);
                endforeach
8:
    PrintTextAbove(who, "*O tesouro está quase visível*");
    if (!HasPlayerMoved(who, initial_x, initial_y))
        chest.movable := 1;
        MoveObjectToLocation(chest, chest.x, chest.y, who.z, who.realm, MOVEOBJECT_FORCELOCATION);
        chest.movable := 0;
    endif
            9:
                PrintTextAbove(who, "*Termina de cavar*");
        endcase

        PerformAction(who, 0x0b);
        IntegrateDiggingEffects(who, targ, i);
        
        current_z := current_z + 1;
        
        // Segunda parte do case - delays
        case(i)
            1:
                Sleep(1);
            2:
                Sleep(2);
            3:
                Sleep(2);
            4:
                Sleep(3);
            5:
                Sleep(3);
            6:
                Sleep(3);
            7:
                Sleep(2);
            8:
                Sleep(2);
            9:
                Sleep(1);
        endcase
        
        i := i + 1;
    endwhile

    // Spawna os guardiões
    var guardians := SpawnTreasureGuardians(who, maplevel, targ.x, targ.y, who.z, who.realm);
    if (!guardians)
    endif
    return chest;
endfunction

/*
 * Limpa recursos se a escavação for interrompida
 */
function CleanupDigging(parts, chest, who)
    // Primeiro marca todos para não serem movíveis novamente e os destrói
    foreach thing in parts
        if (thing)
            thing.movable := 1;
            thing.sellprice := -1;
            thing.buyprice := -1;
            thing.decayAt := 0;
            DestroyItem(thing);
        endif
        sleepms(2); // Pequeno delay para evitar sobrecarga
    endforeach
    
    if (chest)
        DestroyItem(chest);
    endif
    
    SendSysMessageEx(who, "A escavação foi interrompida porque você se moveu.", SSM_FAIL);
    return 0;
endfunction

function ConfigureDirtPatch(patch)
    if (patch)
        patch.movable := 0;
        patch.sellprice := -1;
        patch.buyprice := -1;
        patch.decayAt := 0;
    endif
endfunction