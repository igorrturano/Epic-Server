use uo;
use os;

include ":loot:lootParser";
include ":traps:traps";
include "include/client";
include "include/say";
include ":timedscripts:timedScripts";
include ":destroyable:include/destroyItem";

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
                // Array with all chest graphics organized by category (matching epicchest.src)
                var chest_graphics := array{
                    // MENU_BAUS - Baús (Metal + Madeira + Joias) - 127 items
                    // Baús de Metal
                    0xFF26, 0xFF25, 0xFF24, 0xFF23, 0xFF22, 0xFF21, 0xC2C4, 0xC2BB, 0xC2BA,
                    0xC285, 0xB934, 0xB933, 0xAE24, 0xAE23, 0xAE22, 0xAE21, 0xAE20, 0xAE1F,
                    0xA308, 0xA309, 0x9C18, 0x9C19,
                    0x1C60, 0x1C22, 0x1C77,
                    0xFE92, 0xFE93, 0xFE94, 0xFE95, 0xFE96, 0xFE97, 0xFE9C, 0xFE9D,
                    // Baús de Madeira (WoodenChest com Gump 0x49)
                    3650, 3651, 3648, 3708,
                    0xFF4F, 0xFF4E, 0xFF4D, 0xFF4C, 0xFF3E, 0xFF3D, 0xFF20, 0xFF1F, 0xFF1E,
                    0xC476, 0xC475, 0xC446, 0xC2C3, 0xC2C2, 0xC298, 0xC297, 0xC296, 0xC295,
                    0xC294, 0xC293, 0xC28F, 0xC28E, 0xC28D, 0xC28C, 0xC28B, 0xC28A, 0xC289,
                    0xC288, 0xC287, 0xC286, 0xC284, 0xC283, 0xC282, 0xC281, 0xC27F, 0xC280,
                    0xC27D, 0xC27E, 0xC27B, 0xC27C, 0xC27A, 0xC279, 0xC277, 0xC276, 0xC274,
                    0xC275, 0xC273, 0xC272, 0xC271, 0xC270, 0xC26E, 0xC26D, 0xC26C, 0xC26B,
                    0xC269, 0xC266, 0xC265, 0xC264, 0xC263, 0xC292, 0xC291, 0xC290, 0xC268,
                    0xC267, 0xC235, 0xC236, 0xC234, 0xC233, 0xAE2E, 0xAE2D, 0x74BD, 0x74BC,
                    0x74F1, 0x74F2, 0x3CC8, 0x2859, 0x285A, 0xC45C, 0xC45D, 0xC460, 0xC461,
                    0xC29F, 0x3CD0, 0x3CD1, 0x3CD2, 0x3CD3,
                    // Caixas de Joias
                    0xC54C, 0xC54E, 0xC2BD, 0xC2BC, 0xC26F, 0xB60F, 0xB60E, 0xB56F, 0xB570,
                    0x407C, 0x407D,
                    0xFF0B, 0xFF0C, 0x9FE7, 0x9FE8, 0xC2BE,
                    // MENU_BOLSAS - Bolsas (Mochilas + Bolsas) - 12 items
                    // Mochilas (Backpack)
                    0xFE99, 0xFE98, 0xFCA4, 0xF977, 0xF976,
                    // Bolsas
                    0xFF1D, 0xFF1C, 0xFD0C, 0xFD0B, 0xEB63, 0xEB62, 0xE9DD,
                    // MENU_CESTOS - Cestos (Caixas de Madeira e Crates) - 48 items
                    // Caixas de Madeira
                    0xFF67, 0xFF66, 0xEB7A, 0xEB79, 0xEAD0, 0xEACF, 0xD71F, 0xC2C0, 0xC2BF,
                    0xC248, 0xC247, 0xC246, 0xC245, 0xC244, 0xC243, 0xC242, 0xC241,
                    0xC240, 0xC23F, 0xC23E, 0xC23D, 0xC23C, 0xC23B, 0xC23A, 0xC239, 0xC238,
                    0xC237, 0xAFCD, 0xAFCC, 0x2DF2, 0x2DF1, 0x1FFF,
                    // Crates (MediumCrate/SmallCrate com Gump 0x44)
                    0xFC45, 0xEC40, 0xEC3F, 0xEC3E, 0xEC3D, 0xEC3C, 0xEC3B,
                    0xD53E, 0xC45B, 0xC45A, 0xC2A3, 0xC2A2, 0xC2A0,
                    0xAFCA, 0xAFCB, 0xAFCE, 0xAFCF,
                    // MENU_ESTANTES - Estantes de Livros - 43 items
                    0xFEEA, 0xEA08, 0xEA07, 0xE9CD, 0xE9CC, 0xB912, 0xB911, 0xB4DB, 0xB4DA,
                    0x79FA, 0x79F6, 0x79F5, 0x79F2, 0x79ED, 0x79EC, 0x79FB, 0x6F98, 0x6F97,
                    0x6F23, 0x6F22, 0x6F0D, 0x6F0C, 0x6F08, 0x6F07, 0x6F06, 0x6B40, 0x6B3F,
                    0x6B32, 0x6B31, 0x6A94, 0x6A95, 0x6A96, 0x6A97, 0x6A98, 0x6A99, 0x0C14,
                    0x0C15, 0x0C16, 0x0A97, 0x0A98, 0x0A99, 0x0A9A, 0x0A9B, 0x0A9C,
                    // MENU_BARRIS - Barris (barrel) - 12 items
                    0x722C, 0x3C73, 0x0E7F, 0xC29B, 0xC29A, 0x3C78,
                    3703, 0xC299, 0xC29C, 0xC29D,
                    // MENU_TUMBAS - Túmulos e Urnas - 29 items
                    0x18C9, 0x18CA, 0x18CB, 0x18CC, 0x18CD, 0x1C61, 0x241C,
                    0xBBB7, 0xD456, 0xD457, 0x730D, 0x730E, 0xBE03, 0xBE01, 0xBDFE, 0xB225,
                    0xB224, 0xB223, 0xB222, 0xB221, 0xB220, 0xB217, 0xAFBB, 0xAFBA, 0x7311,
                    0x7312, 0x7313, 0x0B45, 0x0B46, 0x0B47, 0x0B48,
                    0x1C2F, 0x1C43, 0x1C46, 0x1C51, 0x1C54, 0x1C90
                };
                
                // Randomly select a chest graphic from the full list
                var gfx := chest_graphics[RandomInt(chest_graphics.size()) + 1];
                
                // Determine material and HP based on graphic type
                var chestStruct := struct;
                chestStruct.+gfx := gfx;
                
                // Metal chests (higher HP)
                if (gfx == 3648 || gfx == 3708 || gfx == 0xEC40 || gfx == 0xEC3F || 
                    gfx == 0xEC3E || gfx == 0xEC3D || gfx == 0xEC3C || gfx == 0xEC3B)
                    chestStruct.+maxHp := 45;
                    chestStruct.+material := 0x6995; // Metal
                else
                    // Wooden/other chests
                    chestStruct.+maxHp := 30;
                    chestStruct.+material := 0x1bd7; // Wood
                endif
                
                chest := CreateItemAtLocation(newx, newy, newz, chestStruct.gfx, 1, realm);
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

                    // Add traps based on level
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