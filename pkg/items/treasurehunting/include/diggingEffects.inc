use uo;
use os;
use basic;
use math;

// Constante única para efeito de partícula
const PARTICLE_EFFECT_SMOKE_PUFF := 0x9DAC;


// Constantes para patches de terra
enum DIRT_PATCHES 
    DIRT_BASIC_1 := 0x0911,
    DIRT_BASIC_2 := 0x0912,
    DIRT_BASIC_4 := 0x0914,
    DIRT_DECOR_1 := 0x1DFD,
    DIRT_DECOR_2 := 0x1DFE,
    DIRT_DECOR_3 := 0x1DFF,
    DIRT_DECOR_4 := 0x1E00
endenum

// Sons do sistema
const SOUND_DIG := 0x126;     // Som de pá cavando
const SOUND_COMPLETE := 0x127; // Som de conclusão

/*
 * Retorna array com tipos de terra que devem ser limpos
 */
function GetDirtTypes()
    var types := array{
        0x0911,  // DIRT_BASIC_1
        0x0912,  // DIRT_BASIC_2
        0x0914,  // DIRT_BASIC_4
        0x1DFD,  // DIRT_DECOR_1
        0x1DFE,  // DIRT_DECOR_2
        0x1DFF,  // DIRT_DECOR_3
        0x1E00   // DIRT_DECOR_4
    };
    return types;
endfunction


/*
 * Função principal para integrar efeitos de escavação
 */
function IntegrateDiggingEffects(who, targ, stage)
    // Som de escavação em cada estágio
    PlaySoundEffect(who, SOUND_DIG);
    
    // Ajustamos o z para ficar um nível abaixo do baú
    var dirt_z := targ.z - 1;
    
    case (stage)
        1: // Centro apenas
            CreateDirtAndSmoke(targ.x, targ.y, dirt_z, who.realm);
            
        2: // Centro + Norte
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm);
            
        3: // Centro + Norte + Sul
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm);
            CreateDirtAndSmoke(targ.x, targ.y+1, dirt_z, who.realm);
            
        4: // Cruz completa
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm);
            CreateDirtAndSmoke(targ.x, targ.y+1, dirt_z, who.realm);
            CreateDirtAndSmoke(targ.x-1, targ.y, dirt_z, who.realm);
            CreateDirtAndSmoke(targ.x+1, targ.y, dirt_z, who.realm);
            
        5: // Quadrado 3x3 sem centro
            for x := -1 to 1
                for y := -1 to 1
                    if (x != 0 || y != 0) // Pula o centro
                        CreateDirtAndSmoke(targ.x+x, targ.y+y, dirt_z, who.realm);
                        sleepms(50);
                    endif
                endfor
            endfor
            
        6: // Quadrado 3x3 sem centro
            for x := -1 to 1
                for y := -1 to 1
                    if (x != 0 || y != 0) // Pula o centro
                        CreateDirtAndSmoke(targ.x+x, targ.y+y, dirt_z, who.realm);
                        sleepms(50);
                    endif
                endfor
            endfor
            
        7: // Quadrado 3x3 sem centro
            for x := -1 to 1
                for y := -1 to 1
                    if (x != 0 || y != 0) // Pula o centro
                        CreateDirtAndSmoke(targ.x+x, targ.y+y, dirt_z, who.realm);
                        sleepms(50);
                    endif
                endfor
            endfor
            
        8: // Quadrado 3x3 sem centro
            for x := -1 to 1
                for y := -1 to 1
                    if (x != 0 || y != 0) // Pula o centro
                        CreateDirtAndSmoke(targ.x+x, targ.y+y, dirt_z, who.realm);
                        sleepms(50);
                    endif
                endfor
            endfor
            
        9: // Som final
            PlaySoundEffect(who, SOUND_COMPLETE);
    endcase
    
    return 1;
endfunction

function CreateDirtAndSmoke(x, y, z, realm)
    var dirt;
    var last_x := GetGlobalProperty("last_chest_x");
    var last_y := GetGlobalProperty("last_chest_y");
    
    if (x == last_x && y == last_y)
        dirt := CreateItemAtLocation(x, y, z+1, 0x0914, 1, realm);  // -1 para o dirt patch central
    else
        dirt := CreateItemAtLocation(x, y, z, 0x0914, 1, realm);
    endif
    
    SetupDirtPatch(dirt);
    PlayStationaryEffect(x, y, z, PARTICLE_EFFECT_SMOKE_PUFF, 10, 10);
endfunction


/*
 * Cria uma explosão de efeitos de fumaça
 */
function CreateSmokePuffRing(x, y, z, realm, radius := 1)
    // Pontos pré-calculados para formar um círculo
    var points := array{
        array{1, 0},    // 0 graus
        array{1, 1},    // 45 graus
        array{0, 1},    // 90 graus
        array{-1, 1},   // 135 graus
        array{-1, 0},   // 180 graus
        array{-1, -1},  // 225 graus
        array{0, -1},   // 270 graus
        array{1, -1}    // 315 graus
    };
    
    foreach point in points
        var new_x := x + (point[1] * radius);
        var new_y := y + (point[2] * radius);
        PlayStationaryEffect(new_x, new_y, z, PARTICLE_EFFECT_SMOKE_PUFF, 8, 10);
        sleepms(50);
    endforeach
endfunction

/*
 * Retorna array de pontos em círculo
 */
function GetPointsInCircle(center_x, center_y, radius)
    var points := array{};
    
    var x := -radius;
    while (x <= radius)
        var y := -radius;
        while (y <= radius)
            if ((x*x + y*y) <= radius*radius)
                points.append(struct{
                    "x" := center_x + x,
                    "y" := center_y + y
                });
            endif
            y := y + 1;
        endwhile
        x := x + 1;
    endwhile
    
    return points;
endfunction

/*
 * Configura propriedades dos patches de terra
 */
function SetupDirtPatch(item)
    if (item)
        item.movable := 0;
        item.decayAt := ReadGameClock() + 20; // 20 segundos para decair
    endif
endfunction

function CreateSmokePuffBurst(x, y, z, realm, intensity := 1.0)
    var points := GetPointsInCircle(x, y, 2);
    
    foreach point in points
        if (RandomInt(100) < 40 * intensity)
            PlayStationaryEffect(point.x, point.y, z, 
                               PARTICLE_EFFECT_SMOKE_PUFF, 
                               CInt(7 * intensity), // Velocidade
                               CInt(10 * intensity) // Duração
            );
            sleepms(50);
        endif
    endforeach
endfunction


/*
 * Limpa todos os itens de terra em uma área ao redor do ponto especificado
 * Parâmetros:
 * x, y - Coordenadas centrais
 * realm - Reino onde procurar
 * range - Distância da busca (padrão 3 tiles)
 */
exported function CleanupDirtItems(x, y, realm, range := 3)

    // Procura por itens em uma área quadrada
    var items := ListItemsNearLocation(x, y, -1, range, realm);
    var count := 0;
    var dirt_types := GetDirtTypes();
    
    foreach item in items
        if (item && item.objtype in dirt_types)
            // Força o item a ser destruído, mesmo se imóvel
            item.movable := 1;
            DestroyItem(item);
            count += 1;
        endif
        sleepms(2); // Pequena pausa para evitar sobrecarga
    endforeach
    
    return count;
endfunction

/*
 * Limpa a terra associada a um baú específico
 * Parâmetros:
 * chest - Referência ao baú do tesouro
 */
exported function CleanupChestDirt(chest)
    if (!chest)
        return 0;
    endif
    
    return CleanupDirtItems(chest.x, chest.y, chest.realm);
endfunction

