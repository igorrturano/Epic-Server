/*
 * Command Discovery System
 *
 * Auto-discovers player commands from filesystem instead of hardcoding.
 * Scans package directories (pkg/packagename/commands/player) and builds command list.
 * Commands are cached globally to avoid re-scanning on every request.
 */

use file;
use os;
use polsys;
use basic;

include ":hotbar:include/icon_manager";

// Global cache (built once at startup or first request)
// Use uninit to differentiate "not yet scanned" from "scanned but found nothing"
var g_discovered_commands := uninit;
var g_discovered_gm_commands := uninit;

/*
 * Get command data by its index (1-based sequential ID)
 * Uses GM command list if who has cmdlevel > 0, else player list
 * Returns struct with command, script_path, etc. or error
 */
function GetCommandByIndex(index, who := 0)
    var cmd_list;

    if (who && who.cmdlevel > 0)
        if (g_discovered_gm_commands == uninit)
            DiscoverGMCommands();
        endif
        cmd_list := g_discovered_gm_commands;
    else
        if (g_discovered_commands == uninit)
            DiscoverPlayerCommands();
        endif
        cmd_list := g_discovered_commands;
    endif

    // Index is 1-based, array is 1-based in eScript
    if (index < 1 || index > cmd_list.size())
        return error;
    endif

    return cmd_list[index];
endfunction

/*
 * Discover all player commands from filesystem
 * Returns array of command structs
 */
function DiscoverPlayerCommands()
    // Only rescan if not yet initialized (cached globally)
    if (g_discovered_commands != uninit)
        return g_discovered_commands;
    endif

    var start := ReadMillisecondClock();

    g_discovered_commands := array{};
    var index := 1;

    // List of package directories with player commands
    // POL uses :packagename:path format for package-relative paths
    var command_packages := dictionary;
    command_packages[":commands:player/"] := "Geral";
    command_packages[":spells:commands/player/"] := "Magia";
    command_packages[":equipsys:commands/player/"] := "Combate";
    command_packages[":taming:commands/player/"] := "Skill";
    command_packages[":tn:commands/player/"] := "Roleplay";
    command_packages[":fls_core:commands/player/"] := "Roleplay";
    command_packages[":achievements:commands/player/"] := "Outros";
    command_packages[":charactercreation:commands/player/"] := "Outros";
    command_packages[":survival:commands/player/"] := "Skill";
    command_packages[":brainAI:commands/player/"] := "Pet";
    command_packages[":food:textcmd/player/"] := "Outros";
    command_packages[":fls_rppoints:commands/player/"] := "Roleplay";
    command_packages[":specialAbilities:commands/player/"] := "Skill";

    var total_found := 0;

    foreach pkg_path in (command_packages.keys())
        var category := command_packages[pkg_path];

        var files := ListDirectory(pkg_path);
        
        if (!files || files.errortext)
            continue;
        endif

        if (files.size() == 0)
            continue;
        endif

        foreach file in files
            // Only process .src files (skip .ecl, .dep, etc.)
            if (!file[".src"])
                continue;
            endif

            // Extract command name (remove .src extension)
            var cmd_name := file[1, len(file) - 4];
            
            // Skip common non-command files
            if (cmd_name["include"] || cmd_name["config"] || cmd_name["test"])
                continue;
            endif

            // Build script path from package path
            // pkg_path is like ":commands:player/", we need ":commands:player/cmdname"
            var script_path := pkg_path + cmd_name;
            
            // Build command entry
            var cmd_entry := struct{};
            cmd_entry.+command := "." + cmd_name;
            cmd_entry.+desc := GetCommandDescription(cmd_name);
            cmd_entry.+category := category;
            cmd_entry.+index := index;
            cmd_entry.+script_path := script_path;
            
            // Get icon from icon manager
            var icon_data := GetCommandIcon(cmd_name);
            cmd_entry.+iconid := icon_data.iconid;
            cmd_entry.+hue := icon_data.hue;

            g_discovered_commands.append(cmd_entry);
            index += 1;
            total_found += 1;
        endforeach
    endforeach

    var elapsed := ReadMillisecondClock() - start;
    Print($"[CommandDiscovery] Found {total_found} commands in {elapsed}ms");

    return g_discovered_commands;
endfunction

/*
 * Get human-readable description for a command
 * Uses common knowledge or returns generic description
 */
function GetCommandDescription(cmd_name)
    var descriptions := dictionary;
    
    // Geral
    descriptions["ficha"] := "Ficha do personagem";
    descriptions["point"] := "Aponta para um local";
    descriptions["pularcerca"] := "Pular cerca(pode ser visto como invasao!)";
    descriptions["revistar"] := "Pede permissao para revistar a mochila de outro jogador";
    descriptions["senha"] := "Trocar senha";
    descriptions["clima"] := "Indica horario e temperatura atual";
    descriptions["clima"] := "Indica horario e temperatura atual";
    descriptions["fix"] := "Impede um item de tomar decay, fixando-o no chao";
    descriptions["deitar"] := "Deita o personagem";
    descriptions["cabelo"] := "Muda o cabelo se ele for longo";

    // Magia
    descriptions["arcoelemental"] := "Arco elemental";
    descriptions["arremessar"] := "Arremessar coisas ou pessoas(players)";
    descriptions["assumirforma"] := "Assumir forma animal";
    descriptions["rituais"] := "Menu para visualizar as magias memorizadas(Druidas, clerigos e xamas)";
    descriptions["voltar"] := "Voltar forma original, caso esteja em forma animal(druidas)";
    descriptions["meditar"] := "Regenera mana mais rapido";
    descriptions["d"] := "Atalho server-side para magias divinas(Druidas, clerigos e xamas)";
    descriptions["p"] := "Atalho server-side para magias de Paladino";
    descriptions["p"] := "Atalho server-side para magias de Cacadores de Demonio";
    descriptions["m"] := "Atalho server-side para magias de Conjuradores";
    descriptions["m"] := "Atalho server-side para truques magicos(Cantrip)";
    descriptions["t"] := "Atalho server-side para truques magicos(Trickster)";
    descriptions["c"] := "Atalho server-side para magias de Cacadores de Demonios";

    // Combate
    descriptions["golpe"] := "Mesmo da aba Combate";
    descriptions["combate"] := "Mesmo da aba Combate";
    descriptions["special"] := "Mesmo da aba Combate";
    descriptions["postura"] := "Postura da classe Escudeiro";
    descriptions["arcoelemental"] := "Fogo/Gelo/Energia , troca rapida do elemento do arco em vez do duplo clique";
    descriptions["formaprimordial"] := "Fogo/Gelo/Energia , troca rapida do elemento do arco em vez do duplo clique";
    descriptions["assumirforma"] := "Assumir forma animal druida";

    // Gameplay
    descriptions["envenenar"] := "Envenena arma ou alimento. Obs.: Precisa da pocao";
    descriptions["invocacoes"] := "Gerencia suas invocacoes(Pode dessummonar)";
    descriptions["soltar"] := "Se solta de cordas, caso preso";
    descriptions["pilotar"] := "Pilota golens. Apenas Anoes";
    descriptions["adestrar"] := "Domesticar Criaturas";

    // Skills
    descriptions["acalmar"] := "Acalmar animal";
    descriptions["adestrar"] := "Adestrar criatura";
    descriptions["camuflar"] := "Camuflar-se";
    descriptions["estudar"] := "Estudar item";
    
    // Roleplay
    descriptions["actions"] := "Menu de acoes de RP";
    descriptions["arrastar"] := "Arrastar corpo";
    descriptions["erguer"] := "Erguer objeto";
    descriptions["ficharp"] := "Ficha de RP. Necessario para upar suas skills";
    descriptions["lingua"] := "Trocar idioma falado";
    descriptions["sounds"] := "Tocar sons ambiente";
    descriptions["r"] := "Reconhecer outro jogador por um nome (Muda a raca pelo nome escolhido)";

    // Pet
    descriptions["pe"] := "Pet Emote. Gera um emote para seu pet";
    descriptions["pet"] := "Comandos de pet";
    descriptions["assoviar"] := "Faz o seu pet ir para um lugar seguro/retornar a voce. Obs. Ele precisa estar guardado num estabulo";

    // Faccoes
    descriptions["guardastrip"] := "Remove equipamentos de um guarda de sua faccao";
    descriptions["guardaequip"] := "Equipa um item em um guarda de sua faccao";

    // Dungeons
    descriptions["feedback"] := "Da um feedback in game sobre a dungeon";
    
    // Outros
    descriptions["conquistas"] := "Ver conquistas";
    descriptions["fome"] := "Ver fome";
    descriptions["meditar"] := "Meditar";
    descriptions["pontosrp"] := "Pontos de RP";
    descriptions["rps"] := "Sistema de RP";
    descriptions["underwear"] := "Veste roupa intima no sprite";
    descriptions["esvaziar"] := "Joga fora o conteudo de uma pocao para reaver o frasco vazio";
    descriptions["msg"] := "Exibe jogadores online para enviar mensagens privadas \"out game\"";
    descriptions["flip"] := "Alguns itens podem ser virados (flipados) como portas";
    descriptions["gm"] := "Util apenas para gms testarem como gm ou jogador. Nao faz nada como jogador normal.";

    // ====== GM Commands ======
    descriptions["aceleradia"] := "[PERIGO] Acelera ciclo dia/noite. Desativado em producao";
    descriptions["andou"] := "Monitora quando um jogador se move da posicao atual";
    descriptions["avaliarficharp"] := "Avalia ficha de RP de um jogador";
    descriptions["bonusrecriar"] := "Concede bonus +20 de recriacao a uma conta";
    descriptions["bonusrp"] := "Define multiplicador global de XP de RP";
    descriptions["books"] := "Gerencia livros do mundo";
    descriptions["boost"] := "Aplica boost temporario em um jogador";
    descriptions["bufficon"] := "Testa icone de buff temporario";
    descriptions["cargo"] := "Gerencia cargo de um jogador";
    descriptions["checkacc"] := "Verifica informacoes de uma conta";
    descriptions["cidadania"] := "Gerencia cidadania de um jogador";
    descriptions["cleanai"] := "[PERIGO] Remove NPCs com AI antiga em massa";
    descriptions["controleclima"] := "Controla clima do servidor";
    descriptions["corpo"] := "Mata o fantasma de um jogador movendo-o ao corpo";
    descriptions["createmulti"] := "Cria uma estrutura multi (casa, barco, etc)";
    descriptions["criaritenslore"] := "Cria itens de lore/historia";
    descriptions["criaritenslore2"] := "Cria itens de lore/historia (v2)";
    descriptions["criarmateriais"] := "Cria materiais de crafting";
    descriptions["criarpergaminhos"] := "Cria pergaminhos magicos";
    descriptions["criarquadrorp"] := "Cria quadro de RP";
    descriptions["criarreagentes"] := "Cria reagentes magicos";
    descriptions["criartintas"] := "Cria tintas de coloracao";
    descriptions["delserial"] := "[PERIGO] Deleta objeto por serial. Irreversivel!";
    descriptions["destroymulti"] := "[PERIGO] Destroi estrutura multi (casa, barco). Irreversivel!";
    descriptions["dungeons"] := "Gerencia dungeons do servidor";
    descriptions["ekill"] := "[PERIGO] Mata instantaneamente um mobile";
    descriptions["epidemic"] := "[PERIGO] Gerencia epidemias/doencas no servidor";
    descriptions["equip"] := "Equipa itens em um NPC/jogador";
    descriptions["estudarcriar"] := "Cria item de estudo";
    descriptions["fabricar"] := "Abre menu de fabricacao";
    descriptions["flash"] := "Efeito visual de flash em um jogador";
    descriptions["fraj"] := "Gerencia frajolas (itens cosmeticos)";
    descriptions["freeze"] := "Congela/descongela um jogador (impede movimento)";
    descriptions["getscript"] := "Mostra o script de um item/NPC";
    descriptions["getstatic"] := "Mostra informacoes de statics na posicao";
    descriptions["getusescript"] := "Mostra o script de uso de um item";
    descriptions["goserial"] := "Teleporta para um objeto por serial";
    descriptions["gossip"] := "Cria mensagens de fofoca no mundo";
    descriptions["gossip_apagar"] := "Apaga mensagens de fofoca";
    descriptions["heal"] := "Cura completamente um mobile";
    descriptions["idade"] := "Mostra a idade em dias de um personagem";
    descriptions["ident"] := "Identifica propriedades de um item/NPC";
    descriptions["imersao"] := "Ativa emotes/animacoes imersivas em NPCs";
    descriptions["infectar"] := "[PERIGO] Infecta um mobile com doenca";
    descriptions["info"] := "Mostra informacoes detalhadas de um objeto";
    descriptions["infonovo"] := "Mostra informacoes detalhadas (versao nova)";
    descriptions["invisible"] := "Torna um mobile invisivel/visivel";
    descriptions["jump"] := "Pula para uma localizacao (teleporte curto)";
    descriptions["lerdesejo"] := "Ve desejos/pedidos dos jogadores (cmdlevel 3+)";
    descriptions["movetocont"] := "Move um item para dentro de um container";
    descriptions["npccount"] := "Conta NPCs no mundo";
    descriptions["partyspy"] := "Monitora atividades de party de um jogador";
    descriptions["pastor"] := "Gerencia criaturas/NPCs pastor";
    descriptions["petfiel"] := "Gerencia templates de pet fiel";
    descriptions["polcore"] := "Mostra informacoes do servidor POL";
    descriptions["possess"] := "Possui um NPC (controle direto)";
    descriptions["putinfire"] := "Coloca fogo em um objeto";
    descriptions["rectangles"] := "Cria e gerencia zonas retangulares";
    descriptions["regshift"] := "Registra NPC mercante para turno dia/noite";
    descriptions["sayspy"] := "Monitora e loga falas de um jogador";
    descriptions["setclerigo"] := "Define classe cleriga de um jogador";
    descriptions["setentropy"] := "Define nivel de entropia em um mobile";
    descriptions["setlingua"] := "Define idioma de um NPC";
    descriptions["setnpcprofile"] := "Define perfil de um NPC";
    descriptions["setscript"] := "[PERIGO] Altera o script de um item/NPC";
    descriptions["setuscript"] := "[PERIGO] Altera o script de uso de um item";
    descriptions["setusescript"] := "[PERIGO] Altera o script de uso de um item";
    descriptions["sfx"] := "Toca efeito sonoro";
    descriptions["shake"] := "Efeito de tremor de tela";
    descriptions["shave"] := "Remove cabelo de um personagem";
    descriptions["strip"] := "Remove todos equipamentos de um mobile";
    descriptions["summon"] := "Invoca uma criatura";
    descriptions["titulonobre"] := "Gerencia titulos nobres de jogadores";
    descriptions["unlock"] := "Destranca um item trancado";
    descriptions["unpossess"] := "Libera NPC possuido";
    descriptions["whereship"] := "Localiza barcos no mundo";
    descriptions["wolfform"] := "Alterna forma de lobo com bonus/penalidades";
    descriptions["writequip"] := "Salva equipamento atual de um char em EQUIP.CFG";
    descriptions["xp"] := "Concede XP a um jogador";

    // ====== Counselor Commands ======
    descriptions["comandos"] := "Lista comandos disponiveis";
    descriptions["conceal"] := "Esconde/revela o GM";
    descriptions["create"] := "Cria um item por template";
    descriptions["createat"] := "Cria item em coordenada especifica";
    descriptions["destroy"] := "[PERIGO] Destroi um item. Irreversivel!";
    descriptions["destroyradius"] := "[PERIGO] Destroi todos itens em um raio. Irreversivel!";
    descriptions["destruiveis"] := "Gerencia itens destrutiveis";
    descriptions["dupe"] := "Duplica um item";
    descriptions["fetch"] := "Traz um objeto para voce";
    descriptions["getprop"] := "Mostra propriedade de um objeto";
    descriptions["goto"] := "Teleporta para um jogador ou coordenada";
    descriptions["kill"] := "[PERIGO] Mata um mobile";
    descriptions["lkill"] := "[PERIGO] Mata um mobile com raio";
    descriptions["lockradius"] := "Tranca itens em um raio";
    descriptions["mpx"] := "Move objeto no eixo X";
    descriptions["mpy"] := "Move objeto no eixo Y";
    descriptions["mpz"] := "Move objeto no eixo Z";
    descriptions["mtele"] := "Teleporta um mobile para coordenadas";
    descriptions["ns"] := "Ativa nightsight (visao noturna)";
    descriptions["nsight"] := "Ativa nightsight (visao noturna)";
    descriptions["propedit"] := "Editor de propriedades de objetos";
    descriptions["rename"] := "Renomeia um objeto/NPC";
    descriptions["setdescmod"] := "Define descricao modificada de um objeto";
    descriptions["setmovable"] := "Define se um item pode ser movido";
    descriptions["setprop"] := "[PERIGO] Altera propriedade de um objeto";
    descriptions["tele"] := "Teleporta para coordenadas clicadas";
    descriptions["tileinfo"] := "Mostra informacoes de tile do mapa";

    // ====== Seer Commands ======
    descriptions["animations"] := "Testa animacoes em um mobile";
    descriptions["bcast"] := "Envia broadcast para todos jogadores";
    descriptions["copylook"] := "Copia aparencia de um mobile para outro";
    descriptions["copyskills"] := "Copia skills de um mobile para outro";
    descriptions["darken"] := "Escurece a area";
    descriptions["disable"] := "Desabilita um jogador";
    descriptions["enable"] := "Habilita um jogador";
    descriptions["gmbcast"] := "Broadcast visivel apenas para GMs";
    descriptions["go"] := "Teleporta para localizacao nomeada";
    descriptions["mteleto"] := "Teleporta mobile para outro mobile";
    descriptions["music"] := "Toca musica para jogadores";
    descriptions["openpack"] := "Abre mochila de um jogador";
    descriptions["players"] := "Lista jogadores online";
    descriptions["realms"] := "Gerencia reinos do mundo";
    descriptions["sayabove"] := "Mostra texto acima de um mobile";
    descriptions["sayaboveall"] := "Mostra texto acima de todos mobiles na area";
    descriptions["sayabovepvt"] := "Mostra texto privado acima de um mobile";
    descriptions["setlight"] := "Define nivel de luz da area";
    descriptions["shape"] := "Muda aparencia (graphic) de um mobile/item";
    descriptions["teleto"] := "Teleporta voce para um mobile/objeto";
    descriptions["topscripts"] := "Mostra scripts mais pesados rodando";
    descriptions["totalcap"] := "Mostra cap total de skills";
    descriptions["where"] := "Mostra localizacao atual";
    descriptions["resurrect"] := "Ressuscita um jogador morto";

    // ====== Admin Commands ======
    descriptions["givebonusrp"] := "Concede bonus de RP a uma conta";
    descriptions["maxchar"] := "[PERIGO] Maximiza stats de um personagem";
    descriptions["unloadcfg"] := "Recarrega arquivos de configuracao";
    descriptions["shutdown"] := "[PERIGO] Desliga o servidor!";
    descriptions["savenow"] := "Forca save do mundo imediatamente";

    // ====== Other Package GM Commands ======
    descriptions["acctools"] := "Ferramentas de gerenciamento de conta";
    descriptions["addspawn"] := "Adiciona ponto de spawn";
    descriptions["clearspawn"] := "[PERIGO] Limpa spawns de uma area";
    descriptions["cskill"] := "Modifica skill de um jogador";
    descriptions["makerope"] := "Cria corda";
    descriptions["povo"] := "Gerencia populacao de NPCs";
    descriptions["travel"] := "Sistema de viagem rapida";
    descriptions["giverp"] := "Concede pontos de RP a um jogador";
    descriptions["champcfg"] := "Configura champion spawn";
    descriptions["makeGold"] := "Cria moedas de ouro";
    descriptions["delhabs"] := "[PERIGO] Deleta habilidades de um jogador";
    descriptions["massmove"] := "Move todos objetos de uma area (gump com setas)";
    descriptions["criaritens"] := "Cria itens de equipamento";
    descriptions["showequip"] := "Mostra equipamento de um mobile";
    descriptions["addhitscript"] := "Adiciona script de hit a uma arma";
    descriptions["aosdebug"] := "Debug do sistema AOS";
    descriptions["newhit"] := "Testa novo sistema de hit";
    descriptions["pushback"] := "Empurra um mobile para tras";
    descriptions["epicspawner"] := "Gerencia Epic Spawner";
    descriptions["epicspawners"] := "Lista todos Epic Spawners";
    descriptions["epicboss"] := "Gerencia boss de evento";
    descriptions["exportspawners"] := "Exporta spawners para arquivo";
    descriptions["importspawners"] := "[PERIGO] Importa spawners de arquivo";
    descriptions["cspawn"] := "Cria ponto de spawn";
    descriptions["spawns"] := "Lista spawns ativos";
    descriptions["wipespawns"] := "[PERIGO] Remove todos spawns de uma area";
    descriptions["magicloot"] := "Gerencia loot magico";
    descriptions["motdnew"] := "Cria nova mensagem do dia (MOTD)";
    descriptions["instrucao"] := "Gerencia instrucoes/receitas";
    descriptions["criarscrolls"] := "Cria scrolls de magia";
    descriptions["roubavel"] := "Define item como roubavel";
    descriptions["faccoes"] := "Gerencia faccoes";
    descriptions["statsgump"] := "Mostra stats de faccao";
    descriptions["verfaccao"] := "Verifica faccao de um jogador";
    descriptions["event"] := "Gerencia eventos do servidor";
    descriptions["diceroll"] := "Rola dados (sistema de RPG)";
    descriptions["residencias"] := "Gerencia residencias de jogadores";
    descriptions["acctinfo"] := "Informacoes detalhadas de conta";
    descriptions["chests"] := "Gerencia baus de spawn";
    descriptions["resetcooldowns"] := "Reseta cooldowns de um jogador";
    descriptions["listtimers"] := "Lista timers ativos de um mobile";
    descriptions["addtimer"] := "Adiciona timer a um mobile";
    descriptions["curepoisons"] := "Cura todos venenos de um mobile";
    descriptions["lowertimer"] := "Reduz timer de um mobile";
    descriptions["idiot"] := "Marca um jogador como infrator";
    descriptions["tamecmds"] := "Comandos de domesticacao";
    descriptions["minas"] := "Gerencia pontos de mineracao";
    descriptions["shrink"] := "Encolhe um NPC para item carregavel";
    descriptions["mobtype"] := "Mostra tipo de mobile";
    descriptions["createnpc"] := "Cria NPC por template";
    descriptions["massacre"] := "[PERIGO] Mata todos NPCs na area";
    descriptions["cmassacre"] := "[PERIGO] Mata todos NPCs na area (com filtro)";
    descriptions["massacreevil"] := "[PERIGO] Mata todos NPCs hostis na area";
    descriptions["attack"] := "Faz NPC atacar um alvo";
    descriptions["cor"] := "Muda cor de um item/NPC";
    descriptions["powerup"] := "Aumenta stats temporariamente";
    descriptions["refresh"] := "Restaura vida/mana/stamina de um mobile";
    descriptions["recalcvitals"] := "Recalcula vitals de um mobile";
    descriptions["makeTele"] := "Cria teleportador no chao";
    descriptions["control"] := "Controla um NPC remotamente";
    descriptions["copy"] := "Copia um objeto/area";
    descriptions["paste"] := "Cola objeto/area copiada";
    descriptions["mpaste"] := "Cola multiplas copias";
    descriptions["mdestroy"] := "[PERIGO] Destroi multiplos objetos";
    descriptions["createroof"] := "Cria telhado";
    descriptions["dungeondoor"] := "Cria porta de dungeon";
    descriptions["iteminfo"] := "Mostra info detalhada de item";
    descriptions["setdesc"] := "Define descricao de um objeto";
    descriptions["settrap"] := "Configura armadilha em um item";
    descriptions["setupmb"] := "Configura movel de casa";

    var desc := descriptions[cmd_name];
    if (desc)
        return desc;
    endif

    // Generic description
    return "Comando " + cmd_name;
endfunction

/*
 * Discover ALL commands from filesystem (for GMs)
 * Scans all command folders (player, gm, admin, owner, etc.)
 * Returns array of command structs
 */
function DiscoverGMCommands()
    // Only rescan if not yet initialized (cached globally)
    if (g_discovered_gm_commands != uninit)
        return g_discovered_gm_commands;
    endif

    var start := ReadMillisecondClock();

    g_discovered_gm_commands := array{};
    var index := 1;

    // Package base paths to scan for command folders
    var package_bases := array{
        ":commands:",
        ":spells:commands/",
        ":equipsys:commands/",
        ":taming:commands/",
        ":tn:commands/",
        ":fls_core:commands/",
        ":achievements:commands/",
        ":charactercreation:commands/",
        ":survival:commands/",
        ":brainAI:commands/",
        ":food:textcmd/",
        ":fls_rppoints:commands/",
        ":specialAbilities:commands/"
    };

    // Command level folders to scan
    var cmd_levels := array{"player", "gm", "admin", "owner", "seer", "coun"};

    var total_found := 0;

    foreach pkg_base in package_bases
        foreach cmd_level in cmd_levels
            var pkg_path := pkg_base + cmd_level + "/";
            var category := GetCategoryFromLevel(cmd_level);

            var files := ListDirectory(pkg_path);
            
            if (!files || files.errortext)
                continue;
            endif

            if (files.size() == 0)
                continue;
            endif

            foreach file in files
                // Only process .src files (skip .ecl, .dep, etc.)
                if (!file[".src"])
                    continue;
                endif

                // Extract command name (remove .src extension)
                var cmd_name := file[1, len(file) - 4];
                
                // Skip common non-command files
                if (cmd_name["include"] || cmd_name["config"] || cmd_name["test"])
                    continue;
                endif

                // Build script path from package path
                var script_path := pkg_path + cmd_name;

                // Build command entry
                var cmd_entry := struct{};
                cmd_entry.+command := "." + cmd_name;
                cmd_entry.+desc := GetCommandDescription(cmd_name);
                cmd_entry.+category := category;
                cmd_entry.+index := index;
                cmd_entry.+script_path := script_path;

                // Get icon from icon manager
                var icon_data := GetCommandIcon(cmd_name);
                cmd_entry.+iconid := icon_data.iconid;
                cmd_entry.+hue := icon_data.hue;

                g_discovered_gm_commands.append(cmd_entry);
                index += 1;
                total_found += 1;
            endforeach
        endforeach
    endforeach

    var elapsed := ReadMillisecondClock() - start;
    Print($"[CommandDiscovery] Found {total_found} GM commands in {elapsed}ms");

    return g_discovered_gm_commands;
endfunction

/*
 * Get category name based on command level folder
 */
function GetCategoryFromLevel(cmd_level)
    case (cmd_level)
        "player":     return "Player";
        "gm":         return "GM";
        "admin":      return "Admin";
        "owner":      return "Owner";
        "seer":       return "Seer";
        "coun":       return "Counselor";
        default:      return "Outros";
    endcase
endfunction

/*
 * Force refresh of command cache (for development/testing)
 */
function RefreshCommandCache()
    g_discovered_commands := uninit;
    g_discovered_gm_commands := uninit;
    return DiscoverPlayerCommands();
endfunction
