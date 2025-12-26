// Helper function to find player's spellbook
function FindSpellbook(player)
    print("[GMHab DEBUG] Searching for spellbook in backpack and equipped...");
    
    // Search in backpack
    foreach item in EnumerateItemsInContainer(player.backpack)
        print("[GMHab DEBUG] Checking item: " + item.objtype + " / " + item.name);
        var booktype := GetObjProperty(item, "Booktype");
        print("[GMHab DEBUG] Item booktype: " + booktype);
        if (booktype)  // Has Booktype property = custom grimorio
            print("[GMHab DEBUG] Found grimorio in backpack!");
            return item;
        endif
    endforeach
    
    // Search equipped (layer 1 = spellbook)
    print("[GMHab DEBUG] Checking equipped layer 1...");
    var book := GetEquipmentByLayer(player, 0x01);
    if (book)
        print("[GMHab DEBUG] Found equipped item: " + book.objtype);
        var booktype := GetObjProperty(book, "Booktype");
        print("[GMHab DEBUG] Equipped booktype: " + booktype);
        if (booktype)
            print("[GMHab DEBUG] Found grimorio equipped!");
            return book;
        endif
    endif
    
    print("[GMHab DEBUG] No grimorio found!");
    return 0;
endfunction
