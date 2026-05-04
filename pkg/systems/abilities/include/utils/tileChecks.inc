use uo;

function _IsWaterTile(thistile, locinfo)
    if (thistile.objtype)
        if ((thistile.objtype >= 0x1797) && (thistile.objtype <= 0x179c))
            return 1;
        elseif ((thistile.objtype >= 0x346e) && (thistile.objtype <= 0x3485))
            return 1;
        elseif ((thistile.objtype >= 0x3490) && (thistile.objtype <= 0x34ab))
            return 1;
        elseif ((thistile.objtype >= 0x34b5) && (thistile.objtype <= 0x34d5))
            return 1;
        elseif (thistile.objtype IN {20739, 2881, 2882, 2883, 2884, 5453})
            return 1;
        endif
    else
        if ((locinfo.landtile >= 0x00a8) && (locinfo.landtile <= 0x00ab))
            return 1;
        elseif ((locinfo.landtile >= 0x0136) && (locinfo.landtile <= 0x0137))
            return 1;
        elseif ((locinfo.landtile >= 0x3ff0) && (locinfo.landtile <= 0x3ff3))
            return 1;
        endif
    endif

    return 0;
endfunction
