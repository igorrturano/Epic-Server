use uo;
use os;

function CreateGuardian(template, x, y, z, realm, who := 0)
    var npc := 0;
    var tries := 0;
    while (!npc && tries < 5)
        var spawn_x := x + RandomInt(7) - 3;
        var spawn_y := y + RandomInt(7) - 3;
        npc := CreateNPCFromTemplate(template, spawn_x, spawn_y, z, 0, realm);
        if (npc.errortext)
            print("[Guardian] FALHA (exception) '" + template + "': " + npc.errortext);
            return 0;
        endif
        tries := tries + 1;
    endwhile

    if (!npc)
        print("[Guardian] FALHA: '" + template + "' nao criado apos " + tries + " tentativas");
        return 0;
    endif

    print("[Guardian] OK: '" + template + "' serial=" + npc.serial + " name='" + npc.name + "'");
    SetObjProperty(npc, "GuardianCreated", ReadGameClock());
    SetObjProperty(npc, "TreasureGuardian", 1);
    return npc;
endfunction

/*
 * SpawnTreasureGuardians(who, level, x, y, z, realm)
 * Spawna os guardiões apropriados baseado no nível do mapa
 */
function SpawnTreasureGuardians(who, level, x, y, z, realm)
   case (level)
       1: return SpawnBanditGroup(who, x, y, z, realm);
       2: return SpawnMercenaryGroup(who, x, y, z, realm);
       3: return SpawnGhostGroup(who, x, y, z, realm);
       4: return SpawnDragonGroup(who, x, y, z, realm);
       5: return SpawnDracolich(who, x, y, z, realm);
       default: return 0;
   endcase
endfunction

/*
 * Level 1: Bandidos
 */
function SpawnBanditGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte ja fizeram!");
    Sleep(2);

    var npcs := SpawnGuardianWave(array{
        ":epicAI:bandoleiro",
        ":epicAI:Roguearcher",
        ":epicAI:bandoleiro",
        ":epicAI:Rogue",
        ":epicAI:Roguearcher"
    }, x, y, z, realm, who);

    if (!npcs.size())
        return 0;
    endif

    Sleep(60);

    PrintTextAbove(who, "*O ataque continua e o lider se aproxima*");
    var leader := CreateGuardian(":epicAI:Mercenario4", x, y, z, realm, who);
    if (leader)
        npcs.append(leader);
    endif

    return npcs;
endfunction

/*
 * Level 2: Mercenários
 */
function SpawnMercenaryGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Gritos nos arredores* Atacar rapazes! A pior parte ja fizeram!");
    Sleep(2);

    return SpawnGuardianWave(array{
        ":epicAI:bandido_aiglano_ladino",
        ":epicAI:Roguearcher",
        ":epicAI:Rogue",
        ":epicAI:Roguearcher",
        ":epicAI:bandido_aiglano_ladino",
        ":epicAI:Mercenario4"
    }, x, y, z, realm, who);
endfunction

/*
 * Level 3: Fantasmas (noite) ou Criaturas (dia)
 */
function SpawnGhostGroup(who, x, y, z, realm)
    if (GetGlobalProperty("dia") == "dia")
        PrintTextAbove(who, "*A terra treme ao redor do tesouro*");
        Sleep(2);
        return SpawnGuardianWave(array{
            ":epicAI:BlackGiantSpider",
            ":epicAI:ankheg",
            ":epicAI:BlackGiantSpider",
            ":epicAI:AranhaForte",
            ":epicAI:ankheg",
            ":epicAI:orog2"
        }, x, y, z, realm, who);
    endif

    PrintTextAbove(who, "*Gritos e lamurias de almas atormendas* Juntem-se a nos...");
    Sleep(5);

    var npcs := SpawnGuardianWave(array{
        ":epicAI:ghost",
        ":epicAI:ghost",
        ":epicAI:ghost",
        ":epicAI:Spectre",
        ":epicAI:Spectre",
        ":epicAI:Spectre"
    }, x, y, z, realm, who);

    Sleep(60);

    PrintTextAbove(who, "*Vozes Fantasmagoricas* Deliciosa ganancia...");
    npcs.append(CreateGuardian(":epicAI:ghost5", x, y, z, realm, who));
    npcs.append(CreateGuardian(":epicAI:ghost5", x, y, z, realm, who));
    npcs.append(CreateGuardian(":epicAI:ghost5", x, y, z, realm, who));

    return npcs;
endfunction

/*
 * Level 4: Dragões
 */
function SpawnDragonGroup(who, x, y, z, realm)
    PrintTextAbove(who, "*Criaturas gigantescas guardam o local*");
    Sleep(5);

    var npcs := SpawnGuardianWave(array{
        ":epicAI:Wyvern",
        ":epicAI:Wyvern",
        ":epicAI:Wyvern"
    }, x, y, z, realm, who);

    Sleep(60);

    var dragon := CreateGuardian(":epicAI:RedDragon", x, y, z, realm, who);
    if (dragon)
        npcs.append(dragon);
    endif

    return npcs;
endfunction

/*
 * Level 5: Dracolich (noite) ou Dragões/Demônios (dia)
 */
function SpawnDracolich(who, x, y, z, realm)
    if (GetGlobalProperty("dia") == "dia")
        PrintTextAbove(who, "*O ar envenena ao redor do tesouro*");
        Sleep(3);

        var npcs := SpawnGuardianWave(array{
            ":epicAI:balor",
            ":epicAI:Moloch",
            ":epicAI:orog"
        }, x, y, z, realm, who);

        Sleep(60);

        PrintTextAbove(who, "*O Dragão das Sombras desperta!*");
        var boss := CreateGuardian(":epicAI:blackdragon", x, y, z, realm, who);
        if (boss) npcs.append(boss); endif

        return npcs;
    endif

    PrintTextAbove(who, "*Algo terrível desperta das profundezas*");
    Sleep(3);

    var npcs := SpawnGuardianWave(array{
        ":epicAI:banshee2",
        ":epicAI:banshee2",
        ":epicAI:skeleton9",
        ":epicAI:skeleton9"
    }, x, y, z, realm, who);

    Sleep(60);

    PrintTextAbove(who, "*O Dracolich desperta!*");
    var boss := CreateGuardian(":epicAI:Dracolich", x, y, z, realm, who);
    if (boss) npcs.append(boss); endif

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