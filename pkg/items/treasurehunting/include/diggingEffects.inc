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
 * Função principal para integrar efeitos de escavação.
 * chest_x, chest_y: posição do baú para elevar o patch central +1z
 */
function IntegrateDiggingEffects(who, targ, stage, chest_x, chest_y)
    PlaySoundEffect(who, SOUND_DIG);

    var dirt_z := targ.z - 1;

    case (stage)
        1:
            CreateDirtAndSmoke(targ.x, targ.y, dirt_z, who.realm, chest_x, chest_y);

        2:
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm, chest_x, chest_y);

        3:
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm, chest_x, chest_y);
            CreateDirtAndSmoke(targ.x, targ.y+1, dirt_z, who.realm, chest_x, chest_y);

        4:
            CreateDirtAndSmoke(targ.x, targ.y-1, dirt_z, who.realm, chest_x, chest_y);
            CreateDirtAndSmoke(targ.x, targ.y+1, dirt_z, who.realm, chest_x, chest_y);
            CreateDirtAndSmoke(targ.x-1, targ.y, dirt_z, who.realm, chest_x, chest_y);
            CreateDirtAndSmoke(targ.x+1, targ.y, dirt_z, who.realm, chest_x, chest_y);

        5:
            for dx := -1 to 1
                for dy := -1 to 1
                    if (dx != 0 || dy != 0)
                        CreateDirtAndSmoke(targ.x+dx, targ.y+dy, dirt_z, who.realm, chest_x, chest_y);
                        sleepms(50);
                    endif
                endfor
            endfor
        6:
            for dx := -1 to 1
                for dy := -1 to 1
                    if (dx != 0 || dy != 0)
                        CreateDirtAndSmoke(targ.x+dx, targ.y+dy, dirt_z, who.realm, chest_x, chest_y);
                        sleepms(50);
                    endif
                endfor
            endfor
        7:
            for dx := -1 to 1
                for dy := -1 to 1
                    if (dx != 0 || dy != 0)
                        CreateDirtAndSmoke(targ.x+dx, targ.y+dy, dirt_z, who.realm, chest_x, chest_y);
                        sleepms(50);
                    endif
                endfor
            endfor
        8:
            for dx := -1 to 1
                for dy := -1 to 1
                    if (dx != 0 || dy != 0)
                        CreateDirtAndSmoke(targ.x+dx, targ.y+dy, dirt_z, who.realm, chest_x, chest_y);
                        sleepms(50);
                    endif
                endfor
            endfor

        9:
            PlaySoundEffect(who, SOUND_COMPLETE);
    endcase

    return 1;
endfunction

function CreateDirtAndSmoke(x, y, z, realm, chest_x, chest_y)
    var item_z := (x == chest_x && y == chest_y) ? z+1 : z;
    var dirt := CreateItemAtLocation(x, y, item_z, 0x0914, 1, realm);
    SetupDirtPatch(dirt);
    PlayStationaryEffect(x, y, z, PARTICLE_EFFECT_SMOKE_PUFF, 10, 10);
endfunction


function SetupDirtPatch(item)
    if (item)
        item.movable := 0;
        item.decayAt := ReadGameClock() + 20;
    endif
endfunction

exported function CleanupDirtItems(x, y, realm, range := 3)
    var items := ListItemsNearLocation(x, y, -1, range, realm);
    var count := 0;
    var dirt_types := GetDirtTypes();

    foreach item in items
        if (item && item.objtype in dirt_types)
            item.movable := 1;
            DestroyItem(item);
            count += 1;
        endif
        sleepms(2);
    endforeach

    return count;
endfunction

exported function CleanupChestDirt(chest)
    if (!chest)
        return 0;
    endif

    return CleanupDirtItems(chest.x, chest.y, chest.realm);
endfunction

