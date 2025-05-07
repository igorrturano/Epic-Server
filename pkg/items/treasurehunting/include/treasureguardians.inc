

function CreateGuardian(template, x, y, z, realm, who := 0)
    print("DEBUG: Tentando criar NPC: " + template);
    
    var template_name := template;
    if (template_name[":"])
        template_name := template_name[Find(template_name, ":", 2)+1, len(template_name)];
    endif
    
    var pkg_list := array{
        ":brainAI:",
        "" 
    };
    
    var npc := 0;
    foreach pkg in pkg_list
        var full_template := pkg + template_name;
        print("DEBUG: Tentando criar com template: " + full_template);
        
        var spawn_x := x + RandomInt(3) - 1;
        var spawn_y := y + RandomInt(3) - 1;
        
        npc := CreateNPCFromTemplate(full_template, spawn_x, spawn_y, z, 0, realm);
        if (!npc.errortext)
            break;
        else 
            print("DEBUG: Falha com " + full_template + " - " + npc.errortext);
        endif
        sleepms(2);
    endforeach
    
    if (!npc.errortext)
        SetObjProperty(npc, "GuardianCreated", ReadGameClock());
        SetObjProperty(npc, "TreasureGuardian", 1);
        print("DEBUG: NPC criado com sucesso usando " + npc.npctemplate);
        return npc;
    endif
    
    print("DEBUG: Falha ao criar NPC após todas as tentativas");
    return 0;
endfunction

/*
 * SpawnTreasureGuardians(who, level, x, y, z, realm)
 * Spawna os guardiões apropriados baseado no nível do mapa
 */
function SpawnTreasureGuardians(who, level, x, y, z, realm)
   print("DEBUG: SpawnTreasureGuardians - Nível: " + level);
   
   case (level)
       1:  // BANDITS
           print("DEBUG: Spawnando grupo de bandidos");
           return SpawnBanditGroup(who, x, y, z, realm);
       2:  // MERCENARIES
           return SpawnMercenaryGroup(who, x, y, z, realm);
       3:  // GHOSTS
           return SpawnGhostGroup(who, x, y, z, realm);
       4:  // DRAGONS
           return SpawnDragonGroup(who, x, y, z, realm);
       5:  // DRACOLICH
           return SpawnDracolich(who, x, y, z, realm);
       default: 
           print("DEBUG: Nível inválido: " + level);
           return 0;
   endcase
endfunction

/*
 * Level 1: Bandidos
 */
function SpawnBanditGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte ja fizeram!");
    Sleep(2);
    
    // Primeira onda com templates simplificados
    var npcs := array{};
    var templates := array{
        "bandoleiro",
        "roguearcher",
        "bandoleiro",
        "assassin",
        "roguearcher"
    };
    
    foreach template in templates
        var npc := CreateGuardian(template, x, y, z, realm, who);
        if (npc)
            npcs.append(npc);
        endif
        sleepms(2);
    endforeach
    
    if (npcs.size() == 0)
        SendSysMessage(who, "DEBUG: Falha ao criar qualquer NPC do grupo de bandidos");
        return 0;
    endif
    
    Sleep(15);
    
    // Segunda onda com líder
    if (npcs.size() > 0)
        PrintTextAbove(who, "*O ataque continua e o lider se aproxima*");
        var leader := CreateGuardian("MercenarioLider", x, y, z, realm, who);
        if (leader)
            npcs.append(leader);
        endif
    endif
    
    return npcs;
endfunction

/*
 * Level 2: Mercenários
 */
function SpawnMercenaryGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte ja fizeram!");
    Sleep(2);
    
    var npcs := array{
        CreateGuardian(":brainai:assassin", x, y, z, realm, who),
        CreateGuardian(":brainai:roguearcher", x, y, z, realm, who),
        CreateGuardian(":brainai:Rogue", x, y, z, realm, who),
        CreateGuardian(":brainai:roguearcher", x, y, z, realm, who),
        CreateGuardian(":brainai:assassin", x, y, z, realm, who),
        CreateGuardian(":brainai:assassin", x, y, z, realm, who)
    };
    
    return npcs;
endfunction

/*
 * Level 3: Fantasmas
 */
function SpawnGhostGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Gritos e lamurias de almas atormendas* Juntem-se a nos...");
    Sleep(5);
    
    // Primeira onda
    var npcs := array{
        CreateGuardian(":brainai:Ghost", x, y, z, realm, who),
        CreateGuardian(":brainai:Ghost", x, y, z, realm, who),
        CreateGuardian(":brainai:Ghost", x, y, z, realm, who),
        CreateGuardian(":brainai:Spectre", x, y, z, realm, who),
        CreateGuardian(":brainai:Spectre", x, y, z, realm, who),
        CreateGuardian(":brainai:Spectre", x, y, z, realm, who)
    };
    
    Sleep(15);
    
    // Segunda onda
    PrintTextAbove(who, "*Vozes Fantasmagoricas* Deliciosa ganancia...");
    npcs.append(CreateGuardian(":brainai:espectroanciao", x, y, z, realm, who));
    npcs.append(CreateGuardian(":brainai:espectroanciao", x, y, z, realm, who));
    npcs.append(CreateGuardian(":brainai:espectroanciao", x, y, z, realm, who));
    
    return npcs;
endfunction

/*
 * Level 4: Dragões
 */
function SpawnDragonGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Criaturas gigantescas guardam o local*");
    Sleep(5);
    
    // Wyverns primeiro
    var npcs := array{
        CreateGuardian(":brainai:Wyvern", x, y, z, realm, who),
        CreateGuardian(":brainai:Wyvern", x, y, z, realm, who),
        CreateGuardian(":brainai:Wyvern", x, y, z, realm, who)
    };
    
    Sleep(15);
    
    // Dragão Vermelho por último
    npcs.append(CreateGuardian(":brainai:RedDragon", x, y, z, realm, who));
    
    return npcs;
endfunction

/*
 * Level 5: Dracolich
 */
function SpawnDracolich(who, x, y, z, realm)
    var npcs := array{
        CreateGuardian(":brainai:Dracolich", x, y, z, realm, who)
    };
    
    return npcs;
endfunction

/*
 * SpawnGuardianWave(templates, x, y, z, realm, who)
 * Função auxiliar para spawnar grupos de guardiões
 */
function SpawnGuardianWave(templates, x, y, z, realm, who := 0)
    var npcs := array{};
    
    foreach template in templates
        var npc := CreateGuardian(template, x, y, z, realm, who);
        if (npc)
            npcs.append(npc);
        endif
    endforeach
    
    return npcs;
endfunction