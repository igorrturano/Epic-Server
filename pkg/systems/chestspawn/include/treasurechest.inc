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
                type := RandomInt(4)+1;
                var chestStruct := struct;
                case(type) // 0x1bd7 = madeira| 0x6995 = aço
                    1: chestStruct.+gfx := 3650; chestStruct.+maxHp := 30; chestStruct.+material := 0x1bd7;
                    2: chestStruct.+gfx := 3651; chestStruct.+maxHp := 30; chestStruct.+material := 0x1bd7;
                    3: chestStruct.+gfx := 3703; chestStruct.+maxHp := 22; chestStruct.+material := 0x1bd7;
                    4: chestStruct.+gfx := 3648; chestStruct.+maxHp := 45; chestStruct.+material := 0x6995;
                    5: chestStruct.+gfx := 3708; chestStruct.+maxHp := 45; chestStruct.+material := 0x6995;
                endcase
                
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