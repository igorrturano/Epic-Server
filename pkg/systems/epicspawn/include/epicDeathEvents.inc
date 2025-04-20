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
        var gump := GFECreateGump("Death Event Configuration", 600, 500);
        
        // Basic event properties at the top
        GFTextLine(gump, 30, 60, 1153, "Event Name:");
        GFTextEntry(gump, 150, 60, 300, 20, 1153, death_event.name, 1);
        
        GFTextLine(gump, 30, 90, 1153, "Event Type:");
        
        // If type is already set, show it with option to change
        if (death_event.type)
            GFTextLine(gump, 150, 90, 1153, death_event.type);
            GFAddButton(gump, 350, 90, 2117, 2118, GF_CLOSE_BTN, 50); // Change type button
            GFTextLine(gump, 380, 90, 1153, "Change Type");
            
            // Show type-specific configuration options
            case (death_event.type)
                "Criar Itens":
                    GFAddButton(gump, 30, 120, 2117, 2118, GF_CLOSE_BTN, 101);
                    GFTextLine(gump, 60, 120, 1153, "Add Item");
                    
                    // List items
                    var list_y := 150;
                    if (death_event.items.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Configured Items:");
                        list_y += 25;
                        
                        var i := 1;
                        foreach iteminfo in (death_event.items)
                            GFTextLine(gump, 60, list_y, 2100, i + ". " + iteminfo.item + " at " + 
                                      iteminfo.x + "," + iteminfo.y + "," + iteminfo.z);
                            
                            // Add remove button
                            GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i);
                            
                            list_y += 20;
                            i += 1;
                        endforeach
                    else
                        GFTextLine(gump, 30, list_y, 2100, "No items configured yet.");
                    endif
                    break;
                    
                "Abrir Porta":
                "Abrir Porta/Passagem":
                "Abrir Passagem":
                    GFAddButton(gump, 30, 120, 2117, 2118, GF_CLOSE_BTN, 102);
                    GFTextLine(gump, 60, 120, 1153, "Target Door/Passage");
                    
                    // List targeted items
                    var list_y := 150;
                    if (death_event.items.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Targeted objects:");
                        list_y += 25;
                        
                        var i := 1;
                        foreach serial in (death_event.items)
                            var item := SystemFindObjectBySerial(serial);
                            var desc := item ? item.desc : "Unknown object";
                            GFTextLine(gump, 60, list_y, 2100, i + ". " + desc + " (" + serial + ")");
                            
                            // Add remove button
                            GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i);
                            
                            list_y += 20;
                            i += 1;
                        endforeach
                    else
                        GFTextLine(gump, 30, list_y, 2100, "No objects targeted yet.");
                    endif
                    break;
                    
                "Criar Npcs":
                    // NPC Template
                    GFTextLine(gump, 30, 120, 1153, "NPC Template:");
                    GFTextEntry(gump, 150, 120, 250, 20, 1153, death_event.npctemplate, 2);
                    
                    // Amount
                    GFTextLine(gump, 30, 150, 1153, "Number to Spawn:");
                    GFTextEntry(gump, 150, 150, 100, 20, 1153, death_event.amount, 3);
                    
                    // Range
                    GFTextLine(gump, 30, 180, 1153, "Spawn Range:");
                    GFTextEntry(gump, 150, 180, 100, 20, 1153, death_event.range, 4);
                    
                    // Duration
                    GFTextLine(gump, 30, 210, 1153, "Lifetime (seconds, 0 = permanent):");
                    GFTextEntry(gump, 260, 210, 100, 20, 1153, death_event.duracao, 5);
                    
                    // Spawn location
                    GFTextLine(gump, 30, 240, 1153, "Use custom spawn location?");
                    GFCheckBox(gump, 200, 240, 2510, 2511, (death_event.x != who.x && death_event.y != who.y), 21);
                    
                    if (death_event.x == who.x && death_event.y == who.y)
                        GFTextLine(gump, 30, 270, 1153, "Will spawn around the boss location");
                    else
                        GFTextLine(gump, 30, 270, 1153, "Spawn at: " + death_event.x + ", " + death_event.y + ", " + death_event.z);
                        GFAddButton(gump, 30, 300, 2117, 2118, GF_CLOSE_BTN, 103);
                        GFTextLine(gump, 60, 300, 1153, "Change Spawn Location");
                    endif
                    
                    // Boss options here if needed
                    break;
                    
                "Ultimas Palavras":
                    GFAddButton(gump, 30, 120, 2117, 2118, GF_CLOSE_BTN, 104);
                    GFTextLine(gump, 60, 120, 1153, "Add Text Line");
                    
                    // List text lines
                    var list_y := 150;
                    if (death_event.text.size() > 0)
                        GFTextLine(gump, 30, list_y, 1153, "Text Lines:");
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
                        GFTextLine(gump, 30, list_y, 2100, "No text lines added yet.");
                    endif
                    break;
            endcase
        else
            // Show event type selection buttons
            var event_types := array{
                "Criar Itens",
                "Abrir Porta",
                "Abrir Porta/Passagem",
                "Abrir Passagem",
                "Criar Npcs",
                "Ultimas Palavras"
            };
            
            var y_pos := 120;
            var i := 1;
            
            foreach type in event_types
                GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, 10 + i);
                GFTextLine(gump, 60, y_pos, 1153, type);
                y_pos += 25;
                i += 1;
            endforeach
        endif
        
        // Add Save and Cancel buttons
        GFAddButton(gump, 30, 430, 2128, 2129, GF_CLOSE_BTN, 999); // Save button
        GFTextLine(gump, 70, 430, 1153, "Save Event");
        
        GFAddButton(gump, 200, 430, 2119, 2120, GF_CLOSE_BTN, 998); // Cancel button
        GFTextLine(gump, 240, 430, 1153, "Cancel");
        
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
                    
                    // Handle custom location checkbox
                    if (!input[21]) // Checkbox not checked
                        death_event.x := 6;
                        death_event.y := 6;
                        death_event.z := 6;
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
        elseif (input[0] >= 11 && input[0] <= 16)
            // Event type selection
            var event_types := array{
                "Criar Itens",
                "Abrir Porta",
                "Abrir Porta/Passagem",
                "Abrir Passagem",
                "Criar Npcs",
                "Ultimas Palavras"
            };
            death_event.type := event_types[input[0] - 10];
        elseif (input[0] == 50)
            // Change type button
            death_event.type := "";
        elseif (input[0] == 101)
            // Add Item button
            SendSysMessage(who, "Select an item template:");
            var template := Target(who);
            if (template)
                SendSysMessage(who, "Now target the location to create the item:");
                var loc := TargetCoordinates(who);
                if (loc)
                    var item_info := struct{
                        item := template.objtype,
                        x := loc.x,
                        y := loc.y,
                        z := loc.z
                    };
                    death_event.items.append(item_info);
                    SendSysMessage(who, "Item added at " + loc.x + ", " + loc.y + ", " + loc.z);
                endif
            endif
        elseif (input[0] == 102)
            // Target Door/Passage button
            SendSysMessage(who, "Target the door or passage to open on death:");
            var targ := Target(who);
            if (targ)
                death_event.items.append(targ.serial);
                SendSysMessage(who, "Object " + targ.desc + " added");
            endif
        elseif (input[0] == 103)
            // Change spawn location button
            SendSysMessage(who, "Target the location for the NPC spawn:");
            var loc := TargetCoordinates(who);
            if (loc)
                death_event.x := loc.x;
                death_event.y := loc.y;
                death_event.z := loc.z;
                death_event.realm := loc.realm;
                SendSysMessage(who, "Spawn location set to " + loc.x + ", " + loc.y + ", " + loc.z);
            endif
        elseif (input[0] == 104)
            // Add text line button
            SendSysMessage(who, "Enter the text line. Use ~ for line breaks.");
            var text := RequestInput(who, who.backpack, "Enter text:");
            if (text)
                death_event.text.append(text);
                SendSysMessage(who, "Text line added");
            endif
        elseif (input[0] >= 200 && input[0] < 300)
            // Edit text line button
            var idx := input[0] - 200;
            if (idx <= death_event.text.size())
                SendSysMessage(who, "Edit the text line. Use ~ for line breaks.");
                var text := RequestInput(who, who.backpack, "Edit text:" + death_event.text[idx]);
                if (text)
                    death_event.text[idx] := text;
                    SendSysMessage(who, "Text line updated");
                endif
            endif
        elseif (input[0] >= 300 && input[0] < 400)
            // Delete item/text/object button
            var idx := input[0] - 300;
            case (death_event.type)
                "Criar Itens":
                "Abrir Porta":
                "Abrir Porta/Passagem":
                "Abrir Passagem":
                    if (idx <= death_event.items.size())
                        death_event.items.Erase(idx);
                        SendSysMessage(who, "Item/object removed");
                    endif
                    break;
                    
                "Ultimas Palavras":
                    if (idx <= death_event.text.size())
                        death_event.text.Erase(idx);
                        SendSysMessage(who, "Text line removed");
                    endif
                    break;
            endcase
        endif
    endwhile
    
    return 0; // Should never reach here but added for safety
endfunction 