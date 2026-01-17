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
 * Returns struct with command, script_path, etc. or error
 */
function GetCommandByIndex(index)
    // Ensure commands are discovered
    if (g_discovered_commands == uninit)
        DiscoverPlayerCommands();
    endif
    
    // Index is 1-based, array is 1-based in eScript
    if (index < 1 || index > g_discovered_commands.size())
        return error;
    endif
    
    return g_discovered_commands[index];
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
    var cmd_levels := array{"player", "gm", "admin", "owner", "seer", "counselor"};

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

                // Build command entry
                var cmd_entry := struct{};
                cmd_entry.+command := "." + cmd_name;
                cmd_entry.+desc := GetCommandDescription(cmd_name);
                cmd_entry.+category := category;
                cmd_entry.+index := index;
                
                // Get icon from icon manager
                var icon_data := GetCommandIcon("." + cmd_name);
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
        "counselor":  return "Counselor";
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
