use uo;
use util;
use cfgfile;
use os;
use basic;

include ":loot:magicLoot";
include ":destroyable:include/destroyItem";
include ":sysbook:include/sealedBooks";
include ":crafting:/include/fabricaComplexos";

var itemdesc_cache := dictionary{};

function Loot_Generate(container, loot_index)
	if ( !container.IsA(POLCLASS_CONTAINER) )
		return error{"errortext" := "Object passed was not a container."};
	endif

	var index_cfg := ReadConfigFile(":loot:config/loot-index");
	if ( index_cfg.errortext )
		return error{"errortext" := "Error - Could not open loot-index.cfg -> "+index_cfg.errortext};
	endif

	var index_elem := index_cfg[loot_index];
	if ( !index_elem )
		return error{"errortext" := "Error - loot-index.cfg elem ["+index_elem+"] does not exist."};
    //print( "Error - loot-index.cfg elem ["+index_elem+"] does not exist.");
	endif

	foreach line in GetConfigStringArray(index_elem, "Item")
		Loot_CreateItem(container, line);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Stack")
		Loot_CreateStack(container, line);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Random")
		Loot_CreateRandom(container, line);
		sleepms(2);
	endforeach
	
	foreach line in GetConfigStringArray(index_elem, "Group")
		if ( line != loot_index )
			//To do:
			// Check to make sure were not going back into the same group
			// to avoid a potential endless loop!
			Loot_Generate(container, line);
		endif
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Magic");
		Loot_CreateMagic(container, line);
		sleepms(2);
	endforeach
    
     Loot_TryAddSealedBook(container);
	return 1;
endfunction

function CacheItemDesc()
    var cfg := ReadConfigFile(":*:itemdesc");
    if(cfg.errortext)
        return;
    endif
    
    foreach elem_name in ( cfg.keys() )
        var lower_name := Lower(CStr(elem_name));
        var elem := cfg[elem_name];
        if(elem)
            if(elem.ObjType)
                itemdesc_cache[lower_name] := elem.ObjType;
            endif
        endif
        sleepms(2);
    endforeach
endfunction

function Loot_CreateItem(container, line)
    var parsed := Loot_ParseString(line);

    if(Loot_ChanceCheck(parsed.chance))
        var i;
        for(i := 1; i <= parsed.amount; i := i + 1)
            var combatDesc := ReadConfigFile(":combat:itemdesc");
            CreateItemProcess(container, parsed, 1);
            sleepms(2);
        endfor
    endif

    return 1;
endfunction

function CreateItemProcess(container, parsed, qtd)
    var objtype := GetObjtypeByName(parsed.name);
    var item;

    Print("parsed.level: " + parsed.level);
    if( parsed.level && parsed.level != 1)
        /*item :=  GeraItemComplexo(
                                    container,
                                    objtype,
                                    qtd,
                                    parsed.level,
                                    1,
                                    array {0x630e},
                                    FABRICACAO_MODO_MATERIAL::MESMO_TIER_OU_MENOR,
                                    false,
                                    1,//TODO alterar quando as classes estiverem implementadas nos loots
                                    ITEM_MONTADO
                                );*/
    else
        item :=  GeraItemComplexo(
                                    container,
                                    objtype,
                                    qtd,
                                    1,
                                    1,
                                    array {0x630e},
                                    FABRICACAO_MODO_MATERIAL::MESMO_TIER_OU_MENOR
                                );
        //Gera como produto montado
    endif
    
    if(!item)
        item := CreateItemInContainer(container, parsed.name);
        Loot_InitBottle(item);
    endif

    SetObjProperty(item,PROPNAME_DISPENSAVEL_FLAG,true);
    return item;
endfunction

function Loot_CreateStack(container, line)
	var parsed := Loot_ParseString(line);
	
    if ( Loot_ChanceCheck(parsed.chance) )
		var item := CreateItemProcess(container, parsed, parsed.amount);
		if ( item.errortext )
			return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
		endif
	endif

	return 1;
endfunction

function Loot_InitBottle(item)
	if (item.objtype == 0x0F0E)
		SetObjProperty(item, PROPNAME_ENCAIXE_COMPONENTE, "Frasco");
		SetObjProperty(item, PROPNAME_MATERIAL, 7139);
		SetObjProperty(item, PROPNAME_TIER_COMPONENTE, 1);
	endif
endfunction

function Loot_CreateRandom(container, line)
    var parsed, groups_cfg, groups_elem, lines, components;
    var all_items, item_count, stackable_count, item_entry, stack_entry;
    var amount, created, i, idx, item, result, qty;
    
    parsed := Loot_ParseString(line);
    groups_cfg := ReadConfigFile(":loot:config/loot-groups");
    if ( !groups_cfg || groups_cfg.errortext )
        return error{"errortext" := "Could not read loot-groups.cfg"};
    endif
    
    groups_elem := groups_cfg[parsed.name];
    if ( !groups_elem )
        return error{"errortext" := "Group not found"};
    endif

    if ( !Loot_ChanceCheck(parsed.chance) )
        return 1;
    endif
    
    // Lista todos os items do grupo
    all_items := array{};
    item_count := 0;
    stackable_count := 0;
    
    // Processa o grupo. A estrutura é:
    // Item <name> <amount>
    // ou
    // Stack <name> <amount>
    lines := GetConfigStringArray(groups_elem, "Item");
    if(lines)
        foreach line in lines
            components := SplitWords(line);
            if(components.size() >= 1)
                item_entry := struct{
                    "type" := "Item",
                    "name" := components[1],
                    "qty" := components.size() >= 2 ? components[2] : "1"
                };
                all_items.append(item_entry);
                item_count += 1;
            endif
        endforeach
    endif

    lines := GetConfigStringArray(groups_elem, "Stack");
    if(lines)
        foreach line in lines
            components := SplitWords(line);
            if(components.size() >= 1)
                stack_entry := struct{
                    "type" := "Stack",
                    "name" := components[1],
                    "qty" := components.size() >= 2 ? components[2] : "1"
                };
                all_items.append(stack_entry);
                stackable_count += 1;
            endif
        endforeach
    endif

    if(all_items.size() == 0)
        return 0;
    endif
    
    // Cria os itens
    amount := CInt(parsed.amount);
    if(!amount)
        amount := 1;
    endif
    
    created := 0;
    
    for(i := 1; i <= amount; i := i + 1)
        if(all_items.size() > 0)
            idx := RandomInt(all_items.size()) + 1;
            item := all_items[idx];
            
            if(item.type == "Stack")
                qty := RandomDiceRoll(CStr(item.qty));
                result := Loot_CreateStack(container, item.name + " " + qty + " 100");
                if(!result.errortext)
                    created += 1;
                endif
            else
                result := Loot_CreateItem(container, item.name + " 1 100");
                if(!result.errortext)
                    created += 1;
                endif
            endif
        endif
        sleepms(2);
    endfor
    
    return 1;
endfunction

function Loot_ChanceCheck(chance)
	// Convert chance to a decimal for comparison
	var decimal_chance := CDbl(chance);
	
	// For percentages >= 1, use regular 1-100 roll
	if (decimal_chance >= 1)
		var rolado := RandomInt(100) + 1;
		return (rolado <= decimal_chance);
	else
		// For chances < 1% (like 0.3%), use 1-1000 roll for more precision
		var rolado := RandomInt(1000) + 1;
		// Convert chance to per-thousand (0.3 becomes 3 in 1000)
		var per_thousand := decimal_chance * 10;
		return (rolado <= per_thousand);
	endif
endfunction

function Loot_ParseString(line)
    line := SplitWords(line);
    
    // Pega o nome/objtype do item
    var name := CStr(line[1]);
    
    // Se começar com 0x, converte para número
    if(name[1, 2] == "0x")
        // Converte hex para número
        name := CInt("0x" + name[3, len(name)]);
    endif
    
    // Pega a quantidade
    var amount := CStr(line[2]);
    amount := RandomDiceRoll(amount);
    
    // Pega a chance se existir
    var chance := CDbl(line[3]);
    if(!chance)
        chance := 100.0;
    endif
    
    var level := 1;
    if(Find(line[4],">",1) >= 1)
        Print("line: " + line + " - " + line[4]);
        level := line[4];
    else
        level := cint(line[4]);
    endif

    if(!level)
        level := 1;
    endif
    
    var result := struct;
    result.+name := name;
    result.+amount := amount;
    result.+chance := chance;
    result.+level := level;
    
    return result;
endfunction

function Loot_TryAddSealedBook(container)
    // Decide a chance de aparecer um livro lacrado (exemplo: 10%)
    var chance := 30;
    if (RandomInt(100) + 1 > chance)
        return 0;
    endif

    var serial := SealedBooks_GetRandomSerial();
    if (!serial)
        return 0;
    endif

    var book := SealedBooks_CloneToContainer(serial, container);
    if (!book || book.errortext)
        return 0;
    endif

    return 1;
endfunction