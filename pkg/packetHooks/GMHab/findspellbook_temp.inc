// Helper function to find player's spellbook
function FindSpellbook(player)
    // Search in backpack
    foreach item in EnumerateItemsInContainer(player.backpack)
        var booktype := GetObjProperty(item, "Booktype");
        if (booktype)  // Has Booktype property = custom grimorio
            return item;
        endif
    endforeach
    
    // Search equipped (layer 1 = spellbook)
    var book := GetEquipmentByLayer(player, 0x01);
    if (book)
        var booktype := GetObjProperty(book, "Booktype");
        if (booktype)
            return book;
        endif
    endif
    
    return 0;
endfunction
