use uo;

function isAssassinWeapon(who)
    var assassin_weapons := {
        0x27EF, 0x27ab, 0x27af, 11570, 11565, 11566, 0x1401, 0x1440, 0x0f51, 0x0f63, 0x143c, 0x13B2, 0xF9FC, 0xF99F, 
        0xF998, 0xF99B, 0xF99A, 0xF999, 0xF99D, 0xF9A1, 0xF9A2, 0xF9D2, 0xF9CA, 0xF9DC, 0xF9F4,  0xF9A0, 0xF9D4, 0xF9DE,
        0xF9E5, 0xF9A9, 0xF9B5, 0xF9C1, 0xF99E
        };
    foreach weapon in assassin_weapons
        if (weapon == who.weapon.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction

function isWoodWeapon(who)
    var wood_weapons := { 
        0xF996, 0x0df0, 0xECE6, 0x13f9, 0x0e8a, 0x0e81, 0x27AD, 0x27AE, 0x27A7, 0x13b3, 0xF9C0, 0x13B2, 0xe137, 0x26CC, 0xe139, 0x2d2b, 
        0x2d2a, 0x0f4f, 0x13fd, 0x26CD, 0x27A5, 0xF983, 0xF9BF, 0xFA14, 0xFA15
    };
    foreach weapon in wood_weapons
        if (weapon == who.weapon.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction

function isFireWeapon(item)
    var fire_weapons := { 
        0xC5FD, 0xC5FE, 0xC5FF, 0xC600, 0x89A7
    };

    foreach weapon in fire_weapons
        if (weapon == item.objtype)
            return 1;
        endif
    endforeach

    return 0;
endfunction