var event_types := array{
    "Criar Itens",
    "Abrir Porta",
    "Abrir Passagem",
    "Criar Npcs",
    "Ultimas Palavras",
    "Sem corpo",
    "Efeito Grafico",
    "Tocar Som"
};

function HandleAddDeathEvent(who, input, byref form_data)
    // Initialize the death events array if it doesn't exist
    if (!form_data.deathevents)
        form_data.+deathevents := array{};
    endif
    
    // Create a new event structure with proper initialization
    var death_event := struct{
        name := "Death Event " + (form_data.deathevents.size() + 1),
        type := "",  // Will be set during configuration
        items := array{},
        text := array{},
        npctemplate := "",
        amount := 0,
        range := 5,
        duracao := 120,  // Default lifetime in seconds
        x := who.x,  // Default to boss location (6,6,6)
        y := who.y,
        z := who.z,
        realm := who.realm
    };
    
    // Show the events management screen
    var result := ShowDeathEventsList(who, death_event);
    
    // If the user completed the event configuration successfully
    if (result)
        // Validate the event based on its type
        if (!death_event.type)
            SendSysMessage(who, "Death event must have a type. Event creation cancelled.");
            return 0;
        endif
        
        // Type-specific validation
        case (death_event.type)
            "Criar Itens":
                if (death_event.items.size() < 1)
                    SendSysMessage(who, "Create Items event must have at least one item. Event creation cancelled.");
                    return 0;
                endif
                break;
                
            "Criar Npcs":
                if (!death_event.npctemplate || !death_event.amount)
                    SendSysMessage(who, "Create NPCs event must have a template and amount. Event creation cancelled.");
                    return 0;
                endif
                break;
                
            "Ultimas Palavras":
                if (death_event.text.size() < 1)
                    SendSysMessage(who, "Last Words event must have at least one text entry. Event creation cancelled.");
                    return 0;
                endif
                break;

            "Efeito Grafico":
                if (!death_event.fxid)
                    SendSysMessage(who, "Graphic Effect event must have an effect ID. Event creation cancelled.");
                    return 0;
                endif
                break;

            "Tocar Som":
                if (!death_event.soundid)
                    SendSysMessage(who, "Sound event must have a sound ID. Event creation cancelled.");
                    return 0;
                endif
                break;
                
            "Sem corpo": 
                // No validation needed
                break;
        endcase
        
        // Add the event to form_data
        form_data.deathevents.append(death_event);
        SendSysMessage(who, "Added death event '" + death_event.name + "' of type " + death_event.type);
        return 1;
    endif
    
    SendSysMessage(who, "Death event creation cancelled.");
    return 0;
endfunction

// Function to display the death event management screen
function ShowDeathEventsList(who, byref death_event)
    var continue_editing := 1;
    
    while (continue_editing)
        // Create a gump with standardized style
        var gump := GFECreateGump("Configuração de Evento de Morte", 600, 520);
        
        // Basic event properties at the top (moved 20 pixels down)
        GFTextLine(gump, 30, 90, 1153, "Nome do Evento:");
        GFTextEntry(gump, 150, 90, 300, 20, 1153, death_event.name, 1);
        
        GFTextLine(gump, 30, 110, 1153, "Tipo do Evento:");
        
        // If type is already set, show it with option to change
        if (death_event.type)
            GFTextLine(gump, 150, 110, 1153, death_event.type);
            GFAddButton(gump, 350, 110, 2117, 2118, GF_CLOSE_BTN, 50); // Change type button
            GFTextLine(gump, 380, 110, 1153, "Mudar Tipo");
            
            // Show type-specific configuration options
            case (death_event.type)
                "Criar Itens":
                    GFAddButton(gump, 30, 140, 2117, 2118, GF_CLOSE_BTN, 101);
                    GFTextLine(gump, 60, 140, 1153, "Adicionar Item");
                    
                    // List items
                    var list_y := 170;
                    if (death_event.items.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Itens Configurados:");
                        list_y += 25;
                        
                        var i := 1;
                        foreach iteminfo in (death_event.items)
                            GFTextLine(gump, 60, list_y, 2100, i + ". " + iteminfo.item + " em " + 
                                      iteminfo.x + "," + iteminfo.y + "," + iteminfo.z);
                            
                            // Add remove button
                            GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i);
                            
                            list_y += 20;
                            i += 1;
                        endforeach
                    else
                        GFTextLine(gump, 30, list_y, 2100, "Nenhum item configurado ainda.");
                    endif
                    break;
                    
                "Abrir Porta":
                "Abrir Passagem":
                    GFAddButton(gump, 30, 140, 2117, 2118, GF_CLOSE_BTN, 102);
                    GFTextLine(gump, 60, 140, 1153, "Selecionar Porta/Passagem");
                    
                    // List targeted items
                    var list_y := 170;
                    if (death_event.items.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Objetos selecionados:");
                        list_y += 25;
                        
                        var i := 1;
                        foreach serial in (death_event.items)
                            var item := SystemFindObjectBySerial(serial);
                            var desc := item ? item.desc : "Objeto desconhecido";
                            GFTextLine(gump, 60, list_y, 2100, i + ". " + desc + " (" + serial + ")");
                            
                            // Add remove button
                            GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i);
                            
                            list_y += 20;
                            i += 1;
                        endforeach
                    else
                        GFTextLine(gump, 30, list_y, 2100, "Nenhum objeto selecionado ainda.");
                    endif
                    break;
                    
                "Criar Npcs":
                    // NPC Template
                    GFTextLine(gump, 30, 140, 1153, "Template do NPC:");
                    GFTextEntry(gump, 150, 140, 250, 20, 1153, death_event.npctemplate, 2);
                    
                    // Amount
                    GFTextLine(gump, 30, 170, 1153, "Quantidade:");
                    GFTextEntry(gump, 150, 170, 100, 20, 1153, death_event.amount, 3);
                    
                    // Range
                    GFTextLine(gump, 30, 200, 1153, "Alcance de Spawn:");
                    GFTextEntry(gump, 150, 200, 100, 20, 1153, death_event.range, 4);
                    
                    // Duration
                    GFTextLine(gump, 30, 230, 1153, "Duração (segundos, 0 = permanente):");
                    GFTextEntry(gump, 260, 230, 100, 20, 1153, death_event.duracao, 5);
                    
                    // Spawn location options
                    GFTextLine(gump, 30, 260, 1153, "Local de Spawn:");
                    
                    // Option 1: Around boss (default)
                    GFRadioButton(gump, 30, 290, 210, 211, (death_event.x == 6 && death_event.y == 6), 10);
                    GFTextLine(gump, 60, 290, 1153, "Ao redor do boss (quando morrer)");
                    
                    // Option 2: Custom target location
                    GFRadioButton(gump, 30, 320, 210, 211, (death_event.x != 6 && death_event.y != 6 && death_event.x != who.x && death_event.y != who.y), 11);
                    
                    if (death_event.x != 6 && death_event.y != 6 && death_event.x != who.x && death_event.y != who.y)
                        GFTextLine(gump, 60, 320, 1153, "Em local específico: " + death_event.x + ", " + death_event.y + ", " + death_event.z);
                    else
                        GFTextLine(gump, 60, 320, 1153, "Em local específico (clique no botão para selecionar)");
                    endif
                    
                    GFAddButton(gump, 350, 320, 2117, 2118, GF_CLOSE_BTN, 103);
                    GFTextLine(gump, 380, 320, 1153, "Selecionar Local");
                    
                    // Boss options here if needed
                    break;
                    
                "Ultimas Palavras":
                    GFAddButton(gump, 30, 140, 2117, 2118, GF_CLOSE_BTN, 104);
                    GFTextLine(gump, 60, 140, 1153, "Adicionar Linha de Texto");
                    
                    // List text lines
                    var list_y := 170;
                    if (death_event.text.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Linhas de Texto:");
                        list_y += 25;
                        
                        var i := 1;
                        foreach line in (death_event.text)
                            var display_text := line;
                            if (line.length() > 40)
                                display_text := line.substring(1, 40) + "...";
                            endif
                            
                            GFTextLine(gump, 60, list_y, 2100, i + ". " + display_text);
                            
                            // Add edit and remove buttons
                            GFAddButton(gump, 30, list_y, 2117, 2118, GF_CLOSE_BTN, 200 + i); // Edit
                            GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i); // Delete
                            
                            list_y += 20;
                            i += 1;
                        endforeach
                    else
                        GFTextLine(gump, 30, list_y, 2100, "Nenhuma linha de texto adicionada ainda.");
                    endif
                    break;

                "Sem corpo":
                    GFTextLine(gump, 30, 140, 1153, "Este evento fará com que o NPC não deixe corpo ao morrer.");
                    GFTextLine(gump, 30, 160, 2100, "Não há configurações adicionais necessárias.");
                    break;
        
                "Efeito Grafico":
                    // Effect ID
                    GFTextLine(gump, 30, 140, 1153, "ID do Efeito:");
                    GFTextEntry(gump, 150, 140, 100, 20, 1153, death_event.fxid ? death_event.fxid : "", 2);
                    
                    // Speed
                    GFTextLine(gump, 30, 170, 1153, "Velocidade:");
                    GFTextEntry(gump, 150, 170, 100, 20, 1153, death_event.speed ? death_event.speed : "7", 3);
                    
                    // Loops
                    GFTextLine(gump, 30, 200, 1153, "Repetições:");
                    GFTextEntry(gump, 150, 200, 100, 20, 1153, death_event.loops ? death_event.loops : "1", 4);
                    break;
                    
                "Tocar Som":
                    GFTextLine(gump, 30, 140, 1153, "ID do Som:");
                    GFTextEntry(gump, 150, 140, 100, 20, 1153, death_event.soundid ? death_event.soundid : "", 2);
                    break;


            endcase
        else           
            var y_pos := 140;
            var i := 1;
            
            foreach type in event_types
                GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, 10 + i);
                GFTextLine(gump, 60, y_pos, 1153, type);
                y_pos += 25;
                i += 1;
            endforeach
        endif
        
        // Add Save and Cancel buttons
        GFAddButton(gump, 30, 450, 2128, 2129, GF_CLOSE_BTN, 999); // Save button
        GFAddButton(gump, 200, 450, 2119, 2120, GF_CLOSE_BTN, 998); // Cancel button
        
        var input := GFSendGump(who, gump);
        
        // Update event properties
        if (input[0] != 998 && input[0] != 0) // Not canceled
            var name := GFExtractData(input, 1);
            if (name && name != "")
                death_event.name := name;
            endif
            
            // Update type-specific properties
            case (death_event.type)
                "Criar Npcs":
                    var template := GFExtractData(input, 2);
                    if (template && template != "")
                        death_event.npctemplate := template;
                    endif
                    
                    var amount := CInt(GFExtractData(input, 3));
                    if (amount > 0)
                        death_event.amount := amount;
                    endif
                    
                    var range := CInt(GFExtractData(input, 4));
                    if (range > 0)
                        death_event.range := range;
                    endif
                    
                    var duration := CInt(GFExtractData(input, 5));
                    if (duration >= 0)
                        death_event.duracao := duration;
                    endif
                    
                    // Handle spawn location radio buttons
                    if (input[10]) // Around boss
                        death_event.x := 6;
                        death_event.y := 6;
                        death_event.z := 6;
                    elseif (input[11]) // Custom location
                        SendSysMessage(who, "Selecione o local para o spawn de NPCs:");
                        var loc := TargetCoordinates(who);
                        if (loc)
                            death_event.x := loc.x;
                            death_event.y := loc.y;
                            death_event.z := loc.z;
                            death_event.realm := loc.realm;
                            SendSysMessage(who, "Local de spawn definido para " + loc.x + ", " + loc.y + ", " + loc.z);
                        endif
                    endif
                    break;
                    
                "Efeito Grafico":
                    var fxid := CInt(GFExtractData(input, 2));
                    if (fxid)
                        death_event.fxid := fxid;
                    endif
                    
                    var speed := CInt(GFExtractData(input, 3));
                    if (speed)
                        death_event.speed := speed;
                    endif
                    
                    var loops := CInt(GFExtractData(input, 4));
                    if (loops)
                        death_event.loops := loops;
                    endif
                    break;
                    
                "Tocar Som":
                    var soundid := CInt(GFExtractData(input, 2));
                    if (soundid)
                        death_event.soundid := soundid;
                    endif
                    break;
            endcase
        endif
        
        // Handle button clicks
        if (input[0] == 998 || input[0] == 0)
            // Cancel button
            return 0;
        elseif (input[0] == 999)
            // Save button
            return 1;
        elseif (input[0] >= 11 && input[0] <= 18)
            // Event type selection
            death_event.type := event_types[input[0] - 10];
        elseif (input[0] == 50)
            // Change type button
            death_event.type := "";
        elseif (input[0] == 101)
            // Add Item button
            SendSysMessage(who, "Selecione um template de item:");
            var template := Target(who);
            if (template)
                SendSysMessage(who, "Agora selecione o local para criar o item:");
                var loc := TargetCoordinates(who);
                if (loc)
                    var item_info := struct{
                        item := template.objtype,
                        x := loc.x,
                        y := loc.y,
                        z := loc.z
                    };
                    death_event.items.append(item_info);
                    SendSysMessage(who, "Item adicionado em " + loc.x + ", " + loc.y + ", " + loc.z);
                endif
            endif
        elseif (input[0] == 102)
            // Target Door/Passage button
            SendSysMessage(who, "Selecione a porta ou passagem para abrir quando morrer:");
            var targ := Target(who);
            if (targ)
                death_event.items.append(targ.serial);
                SendSysMessage(who, "Objeto " + targ.desc + " adicionado");
            endif
        elseif (input[0] == 103)
            // Change spawn location button
            SendSysMessage(who, "Selecione o local para o spawn de NPCs:");
            var loc := TargetCoordinates(who);
            if (loc)
                death_event.x := loc.x;
                death_event.y := loc.y;
                death_event.z := loc.z;
                death_event.realm := loc.realm;
                SendSysMessage(who, "Local de spawn definido para " + loc.x + ", " + loc.y + ", " + loc.z);
            endif
        elseif (input[0] == 104)
            // Add text line button
            SendSysMessage(who, "Digite a linha de texto. Cada frase será uma nova linha.");
            var text := RequestInput(who, who.backpack, "Digite o texto:");
            if (text)
                death_event.text.append(text);
                SendSysMessage(who, "Linha de texto adicionada");
            endif
        elseif (input[0] >= 200 && input[0] < 300)
            // Edit text line button
            var idx := input[0] - 200;
            if (idx <= death_event.text.size())
                SendSysMessage(who, "Edite a linha de texto.");
                var text := RequestInput(who, who.backpack, "Editar texto: " + death_event.text[idx]);
                if (text)
                    death_event.text[idx] := text;
                    SendSysMessage(who, "Linha de texto atualizada");
                endif
            endif
        elseif (input[0] >= 300 && input[0] < 400)
            // Delete item/text/object button
            var idx := input[0] - 300;
            case (death_event.type)
                "Criar Itens":
                "Abrir Porta":
                "Abrir Passagem":
                    if (idx <= death_event.items.size())
                        death_event.items.Erase(idx);
                        SendSysMessage(who, "Item/objeto removido");
                    endif
                    break;
                    
                "Ultimas Palavras":
                    if (idx <= death_event.text.size())
                        death_event.text.Erase(idx);
                        SendSysMessage(who, "Linha de texto removida");
                    endif
                    break;
            endcase
        endif
    endwhile
    
    return 0; // Should never reach here but added for safety
endfunction