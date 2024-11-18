use uo;
use os;

include ":loot:lootParser";
include ":traps:traps";
include "include/client";
include "include/say";
include ":timedscripts:timedScripts";

/*
 * CreateTreasureChests(lvl, amt, rng, x, y, z, realm, lootname)
 * Função para criar baús do tesouro com armadilhas e loot
 */
function CreateTreasureChests(lvl, amt, rng, x, y, z, realm, lootname)
    var newx, newy, newz, checker, type, gfx, lootgroup, chest;
    var arraychest := array{};
    
    print("DEBUG::CreateTreasureChests - Iniciando criacao:");
    print("  Level: " + lvl);
    print("  Quantidade: " + amt);
    print("  Local: " + x + "," + y + "," + z);
    
    if (x == error)
        print("DEBUG::CreateTreasureChests - Coordenada X invalida");
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
                case(type)
                    1: gfx := 3650;
                    2: gfx := 3651;
                    3: gfx := 3703;
                    4: gfx := 3648;
                    5: gfx := 3708;
                endcase
                
                chest := CreateItemAtLocation(newx, newy, newz, gfx, 1, realm);
            if(chest)
        checker := 1;
        chest.movable := 0;
        
        print("DEBUG::CreateTreasureChests - Bau criado:");
        print("  Serial: " + chest.serial);
        print("  Grafico: " + chest.graphic);
        print("  Local: " + chest.x + "," + chest.y + "," + chest.z);
        
        // Configurações do baú
        SetObjProperty(chest, "IsTreasureChest", 1);
        SetObjProperty(chest, "CreatedOn", ReadGameClock());
        SetObjProperty(chest, "level", lvl);
        
        if (GetObjProperty(chest, "link"))
            SetObjProperty(chest, "IsSpawned", 1);
        else
            SetObjProperty(chest, "IsPlayerCreated", 1);
        endif
        
        chest.locked := 1;
        chest.movable := 0;
                    // Adiciona armadilhas com tratamento de erro
                    var trap := RandomInt(3)+1;
                    print("DEBUG::CreateTreasureChests - Adicionando armadilha tipo " + trap);
                    var trap_result := 0;
                    case(trap)
                        1: trap_result := AddTrap(chest, "Needle", lvl);
                        2: trap_result := AddTrap(chest, "Explosion", lvl);
                        3: trap_result := AddTrap(chest, "PoisonGas", lvl);
                    endcase
                    
                    if (!trap_result)
                        print("DEBUG::CreateTreasureChests - ALERTA: Falha ao adicionar armadilha");
                    endif
                    
                    // Gera o loot com tratamento de erro
                    print("DEBUG::CreateTreasureChests - Gerando loot: " + lootname);
                    var loot_result := Loot_Generate(chest, lootname);
                    if (!loot_result)
                        print("DEBUG::CreateTreasureChests - ALERTA: Falha ao gerar loot");
                    endif
                else
                    checker := 0;
                    tries := tries + 1;
                endif
            endif
            
            if (tries > 30)
                print("DEBUG::CreateTreasureChests - Muitas tentativas de spawn em " + x + "," + y + "," + z);
                var players := EnumerateOnlineCharacters();
                foreach player in players
                    if (player.cmdlevel > 2)
                        SendSysMessageEx(player, "Chestspawner em x " + x + " y " + y + " z " + z + " esta consumindo muito a CPU, delete-o ou recrie-o", SSM_FAIL);
                    endif
                endforeach
            endif
            sleep(10);
        endwhile
        
        if(chest)
            arraychest.append(chest);
            total := total + 1;
        endif
        sleep(10);
    endwhile
    
    print("DEBUG::CreateTreasureChests - Criacao finalizada:");
    print("  Total de baus: " + arraychest.size());
    return arraychest;
endfunction