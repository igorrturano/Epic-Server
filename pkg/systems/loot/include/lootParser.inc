use uo;
use util;
use cfgfile;
use os;
use basic;
use datafile;

include ":loot:magicLoot";
include ":destroyable:include/destroyItem";
include ":sysbook:include/sealedBooks";
include ":crafting:/include/fabricacaoComplexos";
include "include/epicEconomyAudit";
include ":currency:include/seignorage";
include "include/epicCombatUtils";
include ":containers:personalLoot";

var itemdesc_cache := dictionary{};

const LORE_DROPPED_DATAFILE := ":unicos:lore_dropped";

// Phase 2 — coin tier filter helpers.
// cobre: any tier; prata: NPC tier >= 3; ouro: NPC tier >= 5.
// source=0 (no NPC, e.g. chests/mining/treasure) => no filter applied.

function Loot_GetNpcTier(npc)
	if (!npc || npc == error)
		return 0;
	endif
	var tpl_name := CStr(npc.npctemplate);
	if (!tpl_name || tpl_name == "")
		return 0;
	endif
	// Strip namespace prefix: ":epicai:RedDragon" -> "RedDragon"
	var i := len(tpl_name);
	while (i >= 1 && tpl_name[i, 1] != ":")
		i := i - 1;
	endwhile
	if (i > 0 && i < len(tpl_name))
		tpl_name := tpl_name[i + 1, len(tpl_name) - i];
	endif

	var npccfg := ReadConfigFile(":*:npcdesc");
	if (!npccfg || npccfg.errortext)
		return 0;
	endif
	var elem := FindConfigElem(npccfg, tpl_name);
	if (!elem || elem.errortext)
		return 0;
	endif
	return CInt(elem.Tier);
endfunction

function Loot_ParsedCoinObjtype(parsed)
	var ot := parsed.name;
	var t := TypeOf(ot);
	if (t != "Long" && t != "Integer")
		ot := GetObjtypeByName(CStr(ot));
	endif
	if (EA_IsCoin(ot))
		return ot;
	endif
	return 0;
endfunction

function Loot_CoinAllowedForTier(coin_objtype, npc)
	if (!npc || npc == error)
		return 1;
	endif
	if (coin_objtype == EA_COIN_COBRE)
		return 1;
	endif
	var tier := Loot_GetNpcTier(npc);
	if (coin_objtype == EA_COIN_PRATA)
		return (tier >= 3);
	endif
	if (coin_objtype == EA_COIN_OURO)
		return (tier >= 5);
	endif
	return 1;
endfunction

// Phase 4.5 — Drena Cofre virtual em drops bonus de NPC.
// Chamado no fim de Loot_Generate quando source é NPC. Por tier de moeda:
// - skip se tier do NPC < MinTier
// - skip se pool vazio
// - rola chance (5x se TrueBoss)
// - cap pelo daily budget restante
// - drena pool, mintra coin no container, audita POOL_NPC_DRAW

function Loot_TryDrainCoinTier(container, source, coin_objtype, npc_tier, is_trueboss)
	var cfg := GetPoolDropConfig(coin_objtype);
	if (!cfg)
		return 0;
	endif

	if (npc_tier < cfg.min_tier)
		return 0;
	endif

	var pool := GetPoolAmount(coin_objtype);
	if (pool <= 0)
		return 0;
	endif

	var chance := cfg.chance;
	if (is_trueboss)
		chance := chance * cfg.trueboss_mult;
	endif

	// Roll com 2 casas decimais (0.01% de granularidade).
	var roll := RandomInt(10000) + 1;
	if (CDbl(roll) > chance * 100.0)
		return 0;
	endif

	var dice_roll := CInt(RandomDiceRoll(cfg.dice));
	if (dice_roll <= 0)
		return 0;
	endif

	var actual := DrawFromPool(coin_objtype, dice_roll);
	if (actual <= 0)
		return 0;
	endif

	var coin := CreateItemInContainer(container, coin_objtype, actual);
	if (!coin || coin == error || coin.errortext)
		// Se falhou criar, devolve pro pool (best effort).
		InjectToPool(coin_objtype, actual);
		return 0;
	endif

	EA_Record(EA_POOL_NPC_DRAW, coin, actual, source, container,
		(is_trueboss ? "trueboss=1;" : "") + "tier=" + npc_tier);
	return actual;
endfunction

function Loot_TryDrainPool(container, source)
	if (!source || source == error)
		return 0;
	endif
	if (!container || container == error)
		return 0;
	endif

	var npc_tier := Loot_GetNpcTier(source);
	var is_trueboss := IsTrueBoss(source);

	Loot_TryDrainCoinTier(container, source, EA_COIN_COBRE, npc_tier, is_trueboss);
	Loot_TryDrainCoinTier(container, source, EA_COIN_PRATA, npc_tier, is_trueboss);
	Loot_TryDrainCoinTier(container, source, EA_COIN_OURO, npc_tier, is_trueboss);
	return 1;
endfunction

function LoreItems_GetDataFile()
    var df := OpenDataFile(LORE_DROPPED_DATAFILE);
    if (!df)
        CreateDataFile(LORE_DROPPED_DATAFILE, 0);
        df := OpenDataFile(LORE_DROPPED_DATAFILE);
    endif
    return df;
endfunction

function LoreItems_IsDropped(item_name)
    var objtype;
    if (item_name[1,2] == "0x")
        objtype := CInt("0x" + item_name[3, len(item_name)]);
    else
        objtype := CInt(item_name);
    endif
    var df := LoreItems_GetDataFile();
    var elem := df.FindElement(Hex(objtype));
    return (elem && !elem.errortext);
endfunction

function LoreItems_Register(item)
    // Fast lookup: prevent same objtype from dropping again
    var df := LoreItems_GetDataFile();
    if (!df.FindElement(Hex(item.objtype)))
        df.CreateElement(Hex(item.objtype));
    endif

    // Resolve lore item name from config
    var lore_cfg := ReadConfigFile(":custompizza:itemrp/lore_items");
    var lore_elem := FindConfigElem(lore_cfg, item.objtype);
    var lore_name := lore_elem ? lore_elem.name : item.desc;

    // Unicos lista: register physical item by serial so it shows in .unicos gump
    var lista := OpenDataFile(":unicos:lista");
    if (!lista)
        CreateDataFile(":unicos:lista", 0);
        lista := OpenDataFile(":unicos:lista");
    endif
    var elem_name := Hex(item.serial);
    if (!lista.FindElement(elem_name))
        lista.CreateElement(elem_name);
    endif
    var elem := lista.FindElement(elem_name);
    SetObjProperty(item, "itemunico", 1);
    item.decayAt := 0;
    elem.SetProp("Title", "Lore Item");
    elem.SetProp("LoreName", lore_name);
    elem.SetProp("ItemDesc", item.desc);
    elem.SetProp("ID", item.serial);
    elem.SetProp("Creator", "Sistema de Loot");
    elem.SetProp("Time", polcore().systime);
endfunction

function Loot_Generate(container, loot_index, source := 0)
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
		Loot_CreateItem(container, line, source);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Stack")
		Loot_CreateStack(container, line, source);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Random")
		Loot_CreateRandom(container, line, source);
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Group")
		if ( line != loot_index )
			//To do:
			// Check to make sure were not going back into the same group
			// to avoid a potential endless loop!
			Loot_Generate(container, line, source);
		endif
		sleepms(2);
	endforeach

	foreach line in GetConfigStringArray(index_elem, "Magic");
		Loot_CreateMagic(container, line);
		sleepms(2);
	endforeach

     Loot_TryAddSealedBook(container);

     if (source && source != error)
        Loot_TryDrainPool(container, source);
     endif

	// Defensive: if Loot_Generate is ever called directly on a true_boss
	// corpse (not its backpack), stamp the new items. In the normal NPC
	// flow Loot_Generate fills the backpack at spawn — corpse stamping
	// runs from npcdeath.src.
	if (GetObjProperty(container, "true_boss") == 1)
		PersonalLoot_Stamp(container);
	endif
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

function Loot_CreateItem(container, line, source := 0)
    var parsed := Loot_ParseString(line);
    var item;

    if(Loot_ChanceCheck(parsed.chance))
        var i;
        for(i := 1; i <= parsed.amount; i := i + 1)
            var combatDesc := ReadConfigFile(":combat:itemdesc");
                        CreateItemProcess(container, parsed, 1);

            sleepms(2);
        endfor
        return item;
    endif

    return 1;
endfunction

function CreateItemProcess(container, parsed, qtd)
    var objtype := GetObjtypeByName(parsed.name);
    var item;

    if( parsed.level && parsed.level != 1)
        item :=  GeraItemComplexo(
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
                                );
        if(item)
            item.name := "Item Danificado";
        endif
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

function Loot_CreateStack(container, line, source := 0)
	var parsed := Loot_ParseString(line);

	if ( Loot_ChanceCheck(parsed.chance) )
		var coin_ot := Loot_ParsedCoinObjtype(parsed);
		if (coin_ot && !Loot_CoinAllowedForTier(coin_ot, source))
			return 1;
		endif

		var item := CreateItemProcess(container, parsed, parsed.amount);
		if ( item.errortext )
			return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
		endif
		EA_Record(EA_MINT_LOOT, item, parsed.amount, source, container, parsed.name);
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

function Loot_CreateRandom(container, line, source := 0)
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

    // Filter out lore items that have already been dropped in the world
    if (parsed.name == "LoreItems")
        var filtered := array{};
        foreach entry in all_items
            if (!LoreItems_IsDropped(entry.name))
                filtered.append(entry);
            endif
        endforeach
        all_items := filtered;
        if (all_items.size() == 0)
            return 1;
        endif
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
            var tier := parsed.level; 
            
            if(item.type == "Stack")
                qty := RandomDiceRoll(CStr(item.qty));
                result := Loot_CreateStack(container, item.name + " " + qty + " 100 " + tier, source);
                if(!result.errortext)
                    created += 1;
                endif
            else
                result := Loot_CreateItem(container, item.name + " 1 100 " + tier, source);
                if(!result.errortext)
                    if (parsed.name == "LoreItems" && result.IsA(POLCLASS_ITEM))
                        LoreItems_Register(result);
                    endif
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
    // Chance de aparecer um livro lacrado aprovado (5%)
    var chance := 5;
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