function HandleAddCombatEvent(who, input, byref form_data)
    // Initialize the combat events array if it doesn't exist
    if (!form_data.combatevents)
        form_data.+combatevents := array{};
    endif
    
    // Create a new event structure with proper initialization
    var combat_event := struct{
        name := "Combat Event " + (form_data.combatevents.size() + 1),
        cd := 30,
        onlybloodied := 0,
        dist := 0,
        actions := array{},
        steps := array{} // Keep this for backward compatibility
    };
    
    // Show the events management screen
    var result := ShowCombatEventsList(who, combat_event);
    
    // If the user completed the event configuration successfully
    if (result)
        // Only add the event if it has at least one action
        if (combat_event.actions.size() < 1)
            SendSysMessage(who, "Combat event must have at least one action. Event creation cancelled.");
            return 0;
        endif
        
        // Copy actions to steps for compatibility (no need to check if steps exists, we created it above)
        combat_event.steps.length := 0; // Clear the array first
        foreach action in (combat_event.actions)
            combat_event.steps.append(action);
        endforeach
        
        // Log for debugging
        SendSysMessage(who, "DEBUG: Event has " + combat_event.actions.size() + " actions and " + 
                      combat_event.steps.size() + " steps");
        
        // Add the event to form_data
        form_data.combatevents.append(combat_event);
        SendSysMessage(who, "Added combat event '" + combat_event.name + "' with " + 
                      combat_event.actions.size() + " actions.");
        return 1;
    endif
    
    SendSysMessage(who, "Combat event creation cancelled.");
    return 0;
endfunction

// Function to display the combat event management screen
function ShowCombatEventsList(who, byref combat_event)
    var continue_editing := 1;
    
    while (continue_editing)
        // Create a gump with standardized style
        var gump := GFECreateGump("Combat Event Configuration", 600, 450);
        
        // Basic event properties at the top
        GFTextLine(gump, 30, 75, 1153, "Event Name:");
        GFTextEntry(gump, 150, 75, 300, 20, 1153, combat_event.name, 1);
        
        GFTextLine(gump, 30, 90, 1153, "Cooldown (seconds):");
        GFTextEntry(gump, 150, 90, 100, 20, 1153, combat_event.cd, 2);
        
        GFCheckBox(gump, 30, 120, 2510, 2511, combat_event.onlybloodied, 3);
        GFTextLine(gump, 60, 120, 1153, "Only when bloodied");
        
        GFTextLine(gump, 300, 120, 1153, "Required Distance:");
        GFTextEntry(gump, 420, 120, 100, 20, 1153, combat_event.dist, 4);
        
        // Divider line
        GFResizePic(gump, 30, 150, 3000, 540, 2);
        
        // Add action button
        GFAddButton(gump, 30, 165, 2117, 2118, GF_CLOSE_BTN, 100);
        GFTextLine(gump, 60, 165, 1153, "Add New Action");
        
        // List existing actions
        var list_y := 195;
        if (combat_event.actions.size() > 0)
            GFTextLine(gump, 30, list_y, 1153, "Configured Actions:");
            list_y += 25;
            
            var i := 1;
            foreach action in (combat_event.actions)
                GFTextLine(gump, 60, list_y, 2100, i + ". " + action.type);
                
                // Add edit and remove buttons
                GFAddButton(gump, 30, list_y, 2117, 2118, GF_CLOSE_BTN, 200 + i); // Edit
                GFAddButton(gump, 500, list_y, 2437, 2438, GF_CLOSE_BTN, 300 + i); // Delete
                
                // Add a tooltip with details about this action
                var details := "Type: " + action.type;
                if (action.target) 
                    details += ", Target: " + action.target;
                endif
                
                // Add type-specific details
                case (action.type)
                    "Move": details += ", Location: " + action.x + "," + action.y + "," + action.z; break;
                    "Teleport": details += ", Location: " + action.x + "," + action.y + "," + action.z; break;
                    "Perform Action": details += ", Action ID: " + action.id; break;
                    "Text": details += ", Text: '" + action.text + "'"; break;
                    "Play Sound": details += ", Sound ID: " + action.id; break;
                    "Delay": details += ", Duration: " + action.delay + "s"; break;
                    "Heal": details += ", Amount: " + action.healamt; break;
                    "Dmg": details += ", Amount: " + action.dmg + ", Type: " + action.dmgtype; break;
                    "Graphic Effect": details += ", Effect: " + action.fxid; break;
                    "Summon Npcs": details += ", Template: " + action.npctemplate + ", Count: " + action.amount; break;
                endcase
                
                GFTooltipText(gump, details);
                
                list_y += 20;
                i += 1;
                
                // Limit to showing 10 actions at a time
                if (i > 10)
                    GFTextLine(gump, 60, list_y, 2100, "... and " + (combat_event.actions.size() - 10) + " more actions");
                    break;
                endif
            endforeach
        else
            GFTextLine(gump, 30, list_y, 2100, "No actions configured yet. Add actions to create the event sequence.");
        endif
        
        // Add Save and Cancel buttons
        GFAddButton(gump, 30, 420, 2128, 2129, GF_CLOSE_BTN, 999); // Save button
        GFAddButton(gump, 200, 420, 2119, 2120, GF_CLOSE_BTN, 998); // Cancel button
        
        var input := GFSendGump(who, gump);
        
        // Update event properties
        if (input[0] != 998) // Not canceled
            var name := GFExtractData(input, 1);
            if (name && name != "")
                combat_event.name := name;
            endif
            
            var cooldown := CInt(GFExtractData(input, 2));
            if (cooldown > 0)
                combat_event.cd := cooldown;
            endif
            
            combat_event.onlybloodied := input[3] ? 1 : 0;
            
            var dist := CInt(GFExtractData(input, 4));
            if (dist >= 0)
                combat_event.dist := dist;
            endif
        endif
        
        // Handle button clicks
        if (input[0] == 998)
            // Cancel button
            return 0;
        elseif (input[0] == 999)
            // Save button
            return 1;
        elseif (input[0] == 100)
            // Add new action button
            var action := ConfigureCombatAction(who);
            if (action)
                combat_event.actions.append(action);
                SendSysMessage(who, "Added " + action.type + " action.");
            endif
        elseif (input[0] >= 200 && input[0] < 300)
            // Edit action button
            var idx := input[0] - 200;
            if (idx <= combat_event.actions.size())
                var updated_action := ConfigureCombatAction(who, combat_event.actions[idx]);
                if (updated_action)
                    combat_event.actions[idx] := updated_action;
                    SendSysMessage(who, "Action updated.");
                endif
            endif
        elseif (input[0] >= 300 && input[0] < 400)
            // Delete action button
            var idx := input[0] - 300;
            if (idx <= combat_event.actions.size())
                combat_event.actions.Erase(idx);
                SendSysMessage(who, "Action removed.");
            endif
        endif
    endwhile
    
    return 0; // Should never reach here but added for safety
endfunction

// Function to configure a single combat action
function ConfigureCombatAction(who, action := 0)
    // Create a gump with standardized style
    var gump := GFECreateGump("Configure Combat Action", 350, 480);
    
    // List of available action types
    var action_types := array{
        "Teleport",
        "Perform Action",
        "Text",
        "Play Sound",
        "Heal",
        "Dmg",
        "Graphic Effect",
        "Create Items",
        "Summon Npcs",
        "Delay"
    };
    
    // If editing an existing action, show its type at the top
    if (action)
        GFTextLine(gump, 30, 70, 1153, "Action Type: " + action.type);
        GFAddButton(gump, 30, 90, 2117, 2118, GF_CLOSE_BTN, 50); // Change type button
        GFTextLine(gump, 60, 90, 1153, "Change Action Type");
    else
        // If creating a new action, show the action type selection
        GFTextLine(gump, 30, 70, 1153, "Select Action Type:");

        var y_pos := 100;
        var i := 1;
        
        foreach type in action_types
            GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, i);
            GFTextLine(gump, 60, y_pos, 1153, type);
            y_pos += 25;
            i += 1;
            
            // Handle paging if too many types
            if (i > 10 && i < action_types.size())
                // TODO: Add paging if needed
            endif
        endforeach
        
        // Add Cancel button
        GFAddButton(gump, 30, 400, 2119, 2120, GF_CLOSE_BTN, 999);

        var result := GFSendGump(who, gump);
        
        if (result[0] == 999 || result[0] == 0)
            return 0; // Canceled
        endif
        
        if (result[0] >= 1 && result[0] <= action_types.size())
            // Create new action with the selected type
            action := struct{
                type := action_types[result[0]]
            };
        else
            return 0; // Invalid selection
        endif
    endif
    
    // Now configure the specific properties for this action type
    action := ConfigureActionProperties(who, action);
    
    return action;
endfunction

// Function to configure the specific properties of a combat action
function ConfigureActionProperties(who, action)
    if (!action)
        return 0;
    endif
    
    // Create a gump with standardized style
    var title := "Configure " + action.type + " Action";
    var gump := GFECreateGump(title, 500, 450);
    
    var y_pos := 80;
    
    // Common properties for many action types
    if (action.type in array{"Teleport", "Perform Action", "Text", "Graphic Effect", "Dmg"})
        GFTextLine(gump, 30, y_pos, 1153, "Target:");
        y_pos += 25;
        
        // Radio buttons for target selection
        if (!action.target)
            action.+target := "Self"; // Default
        endif
        
        GFSetRadioGroup(gump, 1);
        GFRadioButton(gump, 30, y_pos, 210, 211, action.target == "Self", 1001);
        GFTextLine(gump, 60, y_pos, 1153, "Self");
        
        GFRadioButton(gump, 150, y_pos, 210, 211, action.target == "Opponent", 1002);
        GFTextLine(gump, 180, y_pos, 1153, "Opponent");
        
        GFRadioButton(gump, 300, y_pos, 210, 211, action.target == "All Opponents", 1003);
        GFTextLine(gump, 330, y_pos, 1153, "All Opponents");
        
        y_pos += 35;
    endif
    
    // Type-specific properties
    case (action.type)
        "Move":
            GFTextLine(gump, 30, y_pos, 1153, "This action will move the NPC to a specific location.");
            y_pos += 25;
            GFTextLine(gump, 30, y_pos, 1153, "Click 'Target Location' to set the destination.");
            y_pos += 35;
            GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, 100);
            GFTextLine(gump, 60, y_pos, 1153, "Target Location");
            break;
            
        "Teleport":
            GFTextLine(gump, 30, y_pos, 1153, "This action will teleport the target to a specific location.");
            y_pos += 25;

            // Display coordinates if they exist
            if (action.x && action.y && action.z)
                GFTextLine(gump, 30, y_pos, 1153, "Current Location: " + action.x + ", " + action.y + ", " + action.z);
                y_pos += 25;
            endif
            
            GFTextLine(gump, 30, y_pos, 1153, "Click 'Target Location' to set the destination.");
            y_pos += 35;
            GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, 100);
            GFTextLine(gump, 60, y_pos, 1153, "Target Location");
            break;
            
        "Perform Action":
            GFTextLine(gump, 30, y_pos, 1153, "Animation ID:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.id ? action.id : "", 1);
            break;
            
        "Text":
            GFTextLine(gump, 30, y_pos, 1153, "Text Message:");
            GFTextEntry(gump, 30, y_pos + 25, 400, 20, 1153, action.text ? action.text : "", 1);
            y_pos += 55;
            
            GFTextLine(gump, 30, y_pos, 1153, "Text Color:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.color ? action.color : "1153", 2);
            break;
            
        "Play Sound":
            GFTextLine(gump, 30, y_pos, 1153, "Sound ID:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.id ? action.id : "", 1);
            break;
            
        "Delay":
            GFTextLine(gump, 30, y_pos, 1153, "Delay (seconds):");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.delay ? action.delay : "3", 1);
            break;
            
        "Heal":
            GFTextLine(gump, 30, y_pos, 1153, "Heal Amount:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.healamt ? action.healamt : "100", 1);
            break;
            
        "Dmg":
            GFTextLine(gump, 30, y_pos, 1153, "Damage Amount:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.dmg ? action.dmg : "50", 1);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Damage Type:");
            
            // Initialize damage types array if it doesn't exist
            if (!action.dmgtypes)
                action.+dmgtypes := array{}; 
                // Default to Physical if nothing selected
                if (!action.dmgtypes.size())
                    action.dmgtypes.append("Physical");
                endif
            endif
            
            // Convert legacy dmgtype string to array if needed
            if (action.dmgtype && !action.dmgtypes.size())
                action.dmgtypes.append(action.dmgtype);
                action.dmgtype := 0; // Clear the old format
            endif
            
            // Add checkboxes for damage types
            var dmg_types := array{"Physical", "Fire", "Cold", "Poison", "Energy"};
            var dmg_type_y := y_pos + 25;
            var dmg_type_x := 30;
            var btn_id := 2001;
            
            foreach type in dmg_types
                GFCheckBox(gump, dmg_type_x, dmg_type_y, 2510, 2511, 
                          (type in action.dmgtypes), btn_id);
                GFTextLine(gump, dmg_type_x + 30, dmg_type_y, 1153, type);
                dmg_type_x += 90;
                btn_id += 1;
            endforeach
            
            y_pos += 60;
            
            // Checkboxes for which vitals to affect
            GFTextLine(gump, 30, y_pos, 1153, "Affect Vitals:");
            
            if (!action.vitals)
                action.+vitals := array{"Hits"}; // Default
            endif
            
            y_pos += 25;
            GFCheckBox(gump, 30, y_pos, 2510, 2511, "Hits" in action.vitals, 2101);
            GFTextLine(gump, 60, y_pos, 1153, "Hits");
            
            GFCheckBox(gump, 150, y_pos, 2510, 2511, "Stam" in action.vitals, 2102);
            GFTextLine(gump, 180, y_pos, 1153, "Stamina");
            
            GFCheckBox(gump, 300, y_pos, 2510, 2511, "Mana" in action.vitals, 2103);
            GFTextLine(gump, 330, y_pos, 1153, "Mana");
            break;
            
        "Graphic Effect":
            // This is a complex one, simplified for now
            GFTextLine(gump, 30, y_pos, 1153, "Effect Type:");
            
            // Initialize graphic effect types array if it doesn't exist
            if (!action.graphictypes)
                action.+graphictypes := array{};
                // Default to Stationary if nothing selected
                if (!action.graphictypes.size())
                    action.graphictypes.append("Stationary");
                endif
            endif
            
            // Convert legacy graphictype string to array if needed
            if (action.graphictype && !action.graphictypes.size())
                action.graphictypes.append(action.graphictype);
                action.graphictype := ""; // Clear the old format
            endif
            
            // Add checkboxes for effect types
            var effect_types := array{"Stationary", "Moving"};
            var moving_checked := "Moving" in action.graphictypes;
            
            GFCheckBox(gump, 30, y_pos + 25, 2510, 2511, "Stationary" in action.graphictypes, 3001);
            GFTextLine(gump, 60, y_pos + 25, 1153, "Stationary");
            
            GFCheckBox(gump, 200, y_pos + 25, 2510, 2511, moving_checked, 3002);
            GFTextLine(gump, 230, y_pos + 25, 1153, "Moving");
            
            // Always show direction options so they're immediately visible when "Moving" is checked
            y_pos += 65;
            GFTextLine(gump, 30, y_pos, 1153, "Moving Effect Direction:");
            
            // Initialize effect direction if it doesn't exist
            if (!action.effect_direction)
                action.+effect_direction := "NpcToTarget"; // Default
            endif
            
            y_pos += 35;
            GFSetRadioGroup(gump, 3);
            GFRadioButton(gump, 30, y_pos, 210, 211, action.effect_direction == "NpcToTarget", 3101);
            GFTextLine(gump, 60, y_pos, 1153, "NPC → Target");
            
            GFRadioButton(gump, 200, y_pos, 210, 211, action.effect_direction == "TargetToNpc", 3102);
            GFTextLine(gump, 230, y_pos, 1153, "Target → NPC");
            
            y_pos += 60;
            
            GFTextLine(gump, 30, y_pos, 1153, "Effect ID:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.fxid ? action.fxid : "", 1);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Speed:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.speed ? action.speed : "7", 2);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Loops/Duration:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.loops ? action.loops : "5", 3);
            break;
            
        "Summon Npcs":
            GFTextLine(gump, 30, y_pos, 1153, "NPC Template:");
            GFTextEntry(gump, 150, y_pos, 250, 20, 1153, action.npctemplate ? action.npctemplate : "", 1);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Number to Summon:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.amount ? action.amount : "3", 2);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Spawn Range:");
            GFTextEntry(gump, 150, y_pos, 100, 20, 1153, action.range ? action.range : "5", 3);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Lifetime (seconds, 0 = permanent):");
            GFTextEntry(gump, 260, y_pos, 100, 20, 1153, action.tempo ? action.tempo : "120", 4);
            y_pos += 35;
            
            GFTextLine(gump, 30, y_pos, 1153, "Use custom spawn location?");
            GFCheckBox(gump, 200, y_pos, 2510, 2511, (action.x != 6 && action.y != 6), 4001);
            y_pos += 35;
            
            if (action.x == 6 && action.y == 6)
                GFTextLine(gump, 30, y_pos, 1153, "Will spawn around the boss");
            else
                GFTextLine(gump, 30, y_pos, 1153, "Click to set spawn location:");
                y_pos += 25;
                GFAddButton(gump, 30, y_pos, 2117, 2118, GF_CLOSE_BTN, 100);
                GFTextLine(gump, 60, y_pos, 1153, "Target Spawn Location");
            endif
            break;
    endcase
    
    y_pos := 400; // Position for the bottom buttons
    
    // Add Save and Cancel buttons
    GFAddButton(gump, 30, y_pos, 2128, 2129, GF_CLOSE_BTN, 999); // Save button
    GFAddButton(gump, 200, y_pos, 2119, 2120, GF_CLOSE_BTN, 998); // Cancel button
    
    var result := GFSendGump(who, gump);
    
    if (result[0] == 998 || result[0] == 0)
        return 0; // Canceled
    endif
    
    // Process special button actions
    if (result[0] == 50) // Change action type
        return ConfigureCombatAction(who); // Start over with type selection
    elseif (result[0] == 100) // Target location
        // Update action.target based on the radio buttons' state from THIS gump submission
        // This ensures the selection made by the user for target type is captured before targeting.
        if (result[1001])
            action.target := "Self";
        elseif (result[1002])
            action.target := "Opponent";
        elseif (result[1003])
            action.target := "All Opponents";
        // If no radio button was selected (e.g. if somehow defaults were not set), 
        // action.target would retain its previous value. The initialization of action.target
        // at the start of ConfigureActionProperties and in ConfigureCombatAction ensures it has a sensible default.
        endif
        
        SendSysMessageEx(who, "Target the location for this action.");
        var targ := TargetCoordinates(who);
        
        // If targeting was successful, update coordinates.
        // The 'action' struct now has the updated 'target' (from radio buttons above) 
        // and potentially new x,y,z.
        if (targ)
            action.+x := targ.x;
            action.+y := targ.y;
            action.+z := targ.z;
        endif
        
        // Refresh the gump with the updated action (which includes the correct target type and new coordinates if set).
        return ConfigureActionProperties(who, action); 
    endif
    
    // Process the common target radio buttons for other submissions (e.g., Save)
    if (result[1001])
        action.target := "Self";
    elseif (result[1002])
        action.target := "Opponent";
    elseif (result[1003])
        action.target := "All Opponents";
    endif
    
    // Process damage type checkboxes
    if (action.type == "Dmg")
        var dmg_types := array{"Physical", "Fire", "Cold", "Poison", "Energy"};
        
        // Reset damage types array
        action.dmgtypes := array{};
        
        // Process each damage type checkbox
        for i := 0 to 4
            if (result[2001 + i])
                action.dmgtypes.append(dmg_types[i+1]);
            endif
        endfor
        
        // Make sure at least one damage type is selected
        if (!action.dmgtypes.size())
            action.dmgtypes.append("Physical"); // Default to Physical if nothing selected
        endif
        
        // For backwards compatibility
        action.dmgtype := action.dmgtypes[1];
        
        // Process vital checkboxes
        action.vitals := array{};
        if (result[2101])
            action.vitals.append("Hits");
        endif
        if (result[2102])
            action.vitals.append("Stam");
        endif
        if (result[2103])
            action.vitals.append("Mana");
        endif
    endif
    
    // Process graphic effect type checkboxes
    if (action.type == "Graphic Effect")
        // Reset graphic types array
        action.graphictypes := array{};
        
        // Process each effect type checkbox
        if (result[3001])
            action.graphictypes.append("Stationary");
        endif
        if (result[3002])
            action.graphictypes.append("Moving");
        endif
        
        // Make sure at least one effect type is selected
        if (!action.graphictypes.size())
            action.graphictypes.append("Stationary"); // Default to Stationary if nothing selected
        endif
        
        // Always process direction radio buttons since the fields are always shown
        // This ensures direction is saved even if Moving is newly checked
        if (result[3101])
            action.effect_direction := "NpcToTarget";
        elseif (result[3102])
            action.effect_direction := "TargetToNpc";
        else
            // Default if somehow nothing selected
            action.effect_direction := "NpcToTarget";
        endif
        
        // For backwards compatibility
        action.graphictype := action.graphictypes[1];
    endif
    
    // Process summon NPC custom location checkbox
    if (action.type == "Summon Npcs")
        if (!result[4001])
            action.+x := 6;
            action.+y := 6;
            action.+z := 6;
        endif
    endif
    
    // Process type-specific text entries
    case (action.type)
        "Perform Action":
            action.id := CInt(GFExtractData(result, 1));
            break;
            
        "Text":
            action.text := GFExtractData(result, 1);
            action.color := CInt(GFExtractData(result, 2));
            break;
            
        "Play Sound":
            action.id := CInt(GFExtractData(result, 1));
            break;
            
        "Delay":
            action.delay := CInt(GFExtractData(result, 1));
            break;
            
        "Heal":
            action.healamt := CInt(GFExtractData(result, 1));
            break;
            
        "Dmg":
            action.dmg := CInt(GFExtractData(result, 1));
            break;
            
        "Graphic Effect":
            action.fxid := GFExtractData(result, 1);
            action.speed := CInt(GFExtractData(result, 2));
            action.loops := CInt(GFExtractData(result, 3));
            break;
            
        "Summon Npcs":
            action.npctemplate := GFExtractData(result, 1);
            action.amount := CInt(GFExtractData(result, 2));
            action.range := CInt(GFExtractData(result, 3));
            action.tempo := CInt(GFExtractData(result, 4));
            break;
    endcase
    
    return action;
endfunction