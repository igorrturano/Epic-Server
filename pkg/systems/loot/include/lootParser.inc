use uo;
use util;
use cfgfile;
use os;
use basic;

include ":loot:magicLoot";
include ":destroyable:include/destroyItem";

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

	return 1;
endfunction

function CacheItemDesc()
    print("=== INICIANDO CACHE DE ITEMDESC ===");
    print("Tentando carregar :*:itemdesc");
    
    var cfg := ReadConfigFile(":*:itemdesc");
    if(cfg.errortext)
        print("ERRO ao carregar config: " + cfg.errortext);
        return;
    endif
    
    print("Config carregada com sucesso");
    print("Iniciando cache de " + len(cfg.keys()) + " elementos");
    
    foreach elem_name in ( cfg.keys() )
        var lower_name := Lower(CStr(elem_name));
        var elem := cfg[elem_name];
        if(elem)
            print("Processando: " + elem_name);
            if(elem.ObjType)
                print("-> Cacheando: " + lower_name + " (ObjType: " + elem.ObjType + ")");
                itemdesc_cache[lower_name] := elem.ObjType;
            else
                print("-> AVISO: Elemento sem ObjType: " + elem_name);
            endif
        else
            print("-> ERRO: Elemento invalido: " + elem_name);
        endif
        sleepms(2);
    endforeach
    
    print("=== CACHE FINALIZADO ===");
    print("Total items em cache: " + len(itemdesc_cache));
    print("==================");
endfunction

function Loot_CreateItem(container, line)
    var parsed := Loot_ParseString(line);
    print("Tentando criar item: " + parsed.name);
    
    if(Loot_ChanceCheck(parsed.chance))
        var i;
        for(i := 1; i <= parsed.amount; i := i + 1)
            var item := CreateItemInContainer(container, parsed.name);
            
            SetObjProperty(item,PROPNAME_DISPENSAVEL_FLAG,1);
            if(item.errortext)
                print("ERRO ao criar item ["+parsed.name+"]: " + item.errortext);
            else
                print("Item criado com sucesso: " + item.desc);
            endif

            sleepms(2);
        endfor
    endif
    
    return 1;
endfunction



function Loot_CreateStack(container, line)
	var parsed := Loot_ParseString(line);
	print("Tentando criar stack: " + parsed.name + " quantidade: " + parsed.amount);
	
	if ( Loot_ChanceCheck(parsed.chance) )
		var item := CreateItemInContainer(container, parsed.name, parsed.amount);
        SetObjProperty(item,PROPNAME_DISPENSAVEL_FLAG,1);
		if ( item.errortext )
			print("ERRO ao criar stack: " + item.errortext);
			return error{"errortext" := "Error - Could not create ["+parsed.name+"] ->"+item.errortext};
		endif
		print("Stack criado com sucesso: " + parsed.name + " x" + parsed.amount);
	else
		print("Chance check falhou para stack " + parsed.name);
	endif
	
	return 1;
endfunction

function Loot_CreateRandom(container, line)
    var parsed, groups_cfg, groups_elem, lines, components;
    var all_items, item_count, stackable_count, item_entry, stack_entry;
    var amount, created, i, idx, item, result, qty;
    
    parsed := Loot_ParseString(line);
    print("=== INICIO PROCESSAMENTO RANDOM ===");
    print("Processando grupo random: " + parsed.name + " quantidade: " + parsed.amount + " chance: " + parsed.chance);
    
    groups_cfg := ReadConfigFile(":loot:config/loot-groups");
    if ( !groups_cfg || groups_cfg.errortext )
        print("ERRO ao ler loot-groups.cfg: " + groups_cfg.errortext);
        return error{"errortext" := "Could not read loot-groups.cfg"};
    endif
    
    groups_elem := groups_cfg[parsed.name];
    if ( !groups_elem )
        print("ERRO: grupo " + parsed.name + " não encontrado em loot-groups.cfg");
        return error{"errortext" := "Group not found"};
    endif

    if ( !Loot_ChanceCheck(parsed.chance) )
        print("Chance check falhou para " + parsed.name);
        return 1;
    endif
    
    print("Chance check passou para " + parsed.name);
    
    // Lista todos os items do grupo
    all_items := array{};
    item_count := 0;
    stackable_count := 0;

    print("=== LENDO CONFIGURAÇÃO DO GRUPO ===");
    
    // Processa o grupo. A estrutura é:
    // Item <name> <amount>
    // ou
    // Stack <name> <amount>
    lines := GetConfigStringArray(groups_elem, "Item");
    if(lines)
        foreach line in lines
            print("Processando line Item: " + line);
            components := SplitWords(line);
            if(components.size() >= 1)
                item_entry := struct{
                    "type" := "Item",
                    "name" := components[1],
                    "qty" := components.size() >= 2 ? components[2] : "1"
                };
                all_items.append(item_entry);
                item_count += 1;
                print("Item adicionado: " + components[1]);
            endif
        endforeach
    endif

    lines := GetConfigStringArray(groups_elem, "Stack");
    if(lines)
        foreach line in lines
            print("Processando line Stack: " + line);
            components := SplitWords(line);
            if(components.size() >= 1)
                stack_entry := struct{
                    "type" := "Stack",
                    "name" := components[1],
                    "qty" := components.size() >= 2 ? components[2] : "1"
                };
                all_items.append(stack_entry);
                stackable_count += 1;
                print("Stack adicionado: " + components[1]);
            endif
        endforeach
    endif

    print("Items no grupo: " + all_items.size());
    print("Items: " + item_count + " Stacks: " + stackable_count);
    
    if(all_items.size() == 0)
        print("AVISO: Nenhum item processado em " + parsed.name);
        return 0;
    endif
    
    // Cria os itens
    amount := CInt(parsed.amount);
    if(!amount)
        amount := 1;
    endif
    
    created := 0;
    print("Tentando criar " + amount + " itens do grupo " + parsed.name);
    
    for(i := 1; i <= amount; i := i + 1)
        if(all_items.size() > 0)
            idx := RandomInt(all_items.size()) + 1;
            item := all_items[idx];
            
            print("Tentativa " + i + " de " + amount);
            print("Item selecionado: " + item.type + " -> " + item.name + " x" + item.qty);
            
if(item.type == "Stack")
    qty := RandomDiceRoll(CStr(item.qty));
    print("Criando stack: " + item.name + " x" + qty);
    result := Loot_CreateStack(container, item.name + " " + qty + " 100");
    if(!result.errortext)
        created += 1;
    endif
else
    print("Criando item: " + item.name);
    result := Loot_CreateItem(container, item.name + " 1 100");
    if(!result.errortext)
        created += 1;
    endif
endif
        endif
        sleepms(2);
    endfor
    
    print("=== FIM PROCESSAMENTO ===");
    print("Criados " + created + " de " + amount + " items do grupo " + parsed.name);
    return 1;
endfunction

function Loot_ChanceCheck(chance)
	var rolado := RandomInt(100) + 1;

	if ( rolado <= CInt(chance) )
		return 1;
	else
		return 0;
	endif
endfunction

function Loot_ParseString(line)
    line := SplitWords(line);
    
    // Pega o nome/objtype do item
    var name := CStr(line[1]);
    print("Processando linha raw: " + name); // Debug
    
    // Se começar com 0x, converte para número
    if(name[1, 2] == "0x")
        // Converte hex para número
        name := CInt("0x" + name[3, len(name)]);
        print("Convertido hex para número: " + name + " (0x" + Hex(name) + ")"); // Debug
    endif
    
    // Pega a quantidade
    var amount := CStr(line[2]);
    amount := RandomDiceRoll(amount);
    
    // Pega a chance se existir
    var chance := CDbl(line[3]);
    if(!chance)
        chance := 100.0;
    endif
    
    var level := CInt(line[4]);
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

