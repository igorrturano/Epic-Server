use uo;
use os;

include "include/client";
include "include/say";
include ":chestspawn:treasurechest";
include ":treasurehunting:treasuremaps";
include ":treasurehunting:diggingEffects";
include ":loot:chestPostProcess";

function GetChestGraphicForLevel(level)
    var graphics := array{};
    case (level)
        1: // Barris
            graphics := array{
                0x722c, 0x3c73, 0x3c78, 0xe77, 0xe9cc, 0xe9cd
            };
        2: // Caixas
            graphics := array{
                0xff67, 0xff66, 0xff3e, 0xff3d, 0xeb7a, 0xeb79, 0xead0, 0xeacf,
                0xd71f, 0xc248, 0xc247, 0xc246, 0xc245, 0xc244,
                0xc243, 0xc242, 0xc241, 0xc240, 0xc23f, 0xc23e, 0xc23d, 0xc23c,
                0xc23b, 0xc23a, 0xc239, 0xc238, 0xc237, 0xafcd, 0xafcc, 0x2df2,
                0x2df1, 0x1fff, 0xfc45, 0xec40, 0xec3f, 0xec3e, 0xec3d, 0xc45b,
                0xc45a, 0xc2a3, 0xc2a2, 0xafca, 0xafcb, 0xafce, 0xafcf,
                0xc26f, 0xb60f, 0xb60e, 0xc27e, 0xc280, 0xc27f, 0xe7d
            };
        3: // Baús de madeira
            graphics := array{
                0xe42,  0xe43,  0xff4f, 0xff4e, 0xff4d, 0xff4c, 0xff20, 0xff1f,
                0xff1e, 0xc476, 0xc475, 0xc446, 0xc298, 0xc297,
                0xc295, 0xc294, 0xc293, 0xc292, 0xc291, 0xc290, 0xc28f, 0xc28e,
                0xc28d, 0xc28c, 0xc28b, 0xc28a, 0xc289, 0xc286,
                0xc284, 0xc283, 0xc282, 0xc281, 0xc27d, 0xc27b, 0xc27c, 0xc27a,
                0xc279, 0xc277, 0xc276, 0xc274, 0xc275, 0xc273, 0xc272, 0xc271,
                0xc270, 0xc26e, 0xc26d, 0xc26c, 0xc26b, 0xc268, 0xc267, 0xc266,
                0xc265, 0xc264, 0xc263, 0xc235, 0xc236, 0xc234, 0xc233, 0xae2e,
                0xae2d, 0x74bd, 0x74bc, 0x74f1, 0x74f2, 0x2859, 0x285a, 0xc45c,
                0xc45d, 0xc460, 0xc461, 0x3cd0, 0x3cd1, 0x3cd2, 0x3cd3,
                0x3cc8
            };
        4: // Baús de metal
            graphics := array{
                0xff26, 0xff25, 0xff24, 0xff23, 0xff22, 0xff21,
                0xc285, 0xb934, 0xb933, 0xae24, 0xae23, 0xae22, 0xae21,
                0xae20, 0xae1f, 0x9c19, 0xfe92, 0xfe93, 0xfe94, 0xfe95, 0xfe96,
                0xfe97, 0xfe9c, 0xfe9d, 0xe40,  0xe7c,  0x9fe7, 0x9fe8
            };
        5: // Baús de metal + Sarcófagos
            graphics := array{
                0xff26, 0xff25, 0xff24, 0xff23, 0xff22, 0xff21,
                0xc285, 0xb934, 0xb933, 0xae24, 0xae23, 0xae22, 0xae21,
                0xae20, 0xae1f, 0x9c19, 0xfe92, 0xfe93, 0xfe94, 0xfe95, 0xfe96,
                0xfe97, 0xfe9c, 0xfe9d, 0xe40,  0xe7c,  0x9fe7, 0x9fe8, 
                0xd457, 0x730d, 0x730e, 0xbe03, 0xbe01, 0xbdfe,
                0x1c2f, 0x1c43, 0x1c46, 0x1c51, 0x1c54, 0x1c90, 0x1c22, 0x1c77
            };
        default:
            return 0x0E41;
    endcase

    if (!graphics.size())
        return 0x0E41;
    endif
    return graphics[RandomInt(graphics.size()) + 1];
endfunction

/*
 * Verifica se o jogador se moveu da posição inicial
 */
function HasPlayerMoved(who, initial_x, initial_y)
    return (who.x != initial_x || who.y != initial_y);
endfunction

exported function DigForMap(who, targ, map)
  
    if (!reserveItem(map))
        return 0;
    endif
    if (IsMapSolved(map))
        return 0;
    endif

    if (!ValidateMapDistance(who, targ, map))
        return 0;
    endif

    var result := StartTreasureDigging(who, targ, map);

    if (result)
        MarkMapComplete(map, who);
    endif

    return result;
endfunction

exported function StartTreasureDigging(who, targ, map)
    if (!who || !targ || !map)
        print("[Digging] StartTreasureDigging: parametro invalido");
        return 0;
    endif

    print("[Digging] Iniciando escavacao para " + who.name + " em x=" + targ.x + " y=" + targ.y + " z=" + targ.z);

    // Registra posição inicial
    var initial_x := who.x;
    var initial_y := who.y;

    PrintTextAbove(who, "*Começa a cavar o tesouro*");
    SendSysMessageEx(who, "Permaneça parado enquanto cava. Se você se mover, precisará recomeçar.", SSM_INFO);

    var parts := array{};
    var maplevel := CInt(GetObjProperty(map, "level"));
    var loot := "treasuremap" + maplevel;
    print("[Digging] Nivel=" + maplevel + " loot=" + loot);

    // O baú começa bem enterrado
    var initial_z := who.z - 9;
    var current_z := initial_z;

    print("[Digging] Criando bau em x=" + targ.x + " y=" + targ.y + " z=" + initial_z + " realm=" + who.realm);
    var chest_graphic := GetChestGraphicForLevel(maplevel);
    print("[Digging] Graphic do bau: 0x" + hex(chest_graphic) + " (nivel " + maplevel + ")");
    var chest := CreateItemAtLocation(targ.x, targ.y, initial_z, chest_graphic, 1, who.realm);
    if (!chest)
        print("[Digging] FALHA: CreateItemAtLocation retornou 0 em x=" + targ.x + " y=" + targ.y + " z=" + initial_z + " realm=" + who.realm);
        return 0;
    endif
    print("[Digging] Bau criado: serial=" + chest.serial + " graphic=" + chest.graphic);

    // Configuração do baú
    SetObjProperty(chest, "CreatedOn", ReadGameClock());
    SetObjProperty(chest, "DestroyAt", ReadGameClock() + 600);
    SetObjProperty(chest, "level",    maplevel);
    SetObjProperty(chest, "MapLevel", maplevel);
    SetObjProperty(chest, "FoundBy", who.serial);

    chest.movable := 0;
    AddRandomTraps(chest, maplevel);
    var loot_result := Loot_Generate(chest, loot);
    ChestLoot_PostProcess(chest);
    print("[Digging] Loot_Generate resultado: " + loot_result + " | itens no bau: " + EnumerateItemsInContainer(chest).size());
    chest.locked := 1;

    // Sorteio de minigame de lockpick por nível
    var minigame_chance := 0;
    case (maplevel)
        1: minigame_chance := 10;
        2: minigame_chance := 15;
        3: minigame_chance := 20;
        4: minigame_chance := 25;
        5: minigame_chance := 30;
    endcase
    if (RandomInt(100) < minigame_chance)
        SetObjProperty(chest, "minigame_lock", 1);
        print("[Digging] Bau nivel=" + maplevel + " sorteou minigame de lockpick (" + minigame_chance + "% chance)");
    else
        print("[Digging] Bau nivel=" + maplevel + " sem minigame (" + minigame_chance + "% chance, nao sorteou)");
    endif

    // Inicia o script de destruição
    var script := start_script(":treasurehunting:chest/destroy", chest);
    if (!script)
        DestroyItem(chest);
        return 0;
    endif

    var i := 1;
    while (i <= 9)
        print("[Digging] Estagio " + i + " jogador em x=" + who.x + " y=" + who.y);
        if (HasPlayerMoved(who, initial_x, initial_y))
            print("[Digging] Jogador se moveu! Abortando.");
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

        PerformAction(who, 0x0e);
        IntegrateDiggingEffects(who, targ, i, chest.x, chest.y);
        
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

    print("[Digging] Escavacao concluida! Spawnando guardioes nivel=" + maplevel);
    start_script(":treasurehunting:guardians/spawn", array{who, maplevel, targ.x, targ.y, who.z, who.realm});
    print("[Digging] Bau pronto: x=" + chest.x + " y=" + chest.y + " z=" + chest.z);
    return chest;
endfunction

/*
 * Limpa recursos se a escavação for interrompida
 */
function CleanupDigging(parts, chest, who)
    foreach thing in parts
        if (thing)
            thing.movable := 1;
            DestroyItem(thing);
        endif
        sleepms(2);
    endforeach

    if (chest)
        // Sinaliza ao destroy_chest script que o baú foi abortado antes de destruir
        EraseObjProperty(chest, "DestroyAt");
        SetObjProperty(chest, "ChestAborted", 1);
        DestroyItem(chest);
    endif

    SendSysMessageEx(who, "A escavação foi interrompida porque você se moveu.", SSM_FAIL);
    return 0;
endfunction