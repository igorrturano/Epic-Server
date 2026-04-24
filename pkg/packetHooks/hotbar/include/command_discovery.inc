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
            
            // Build lean command entry (desc resolved at packet-write time)
            var cmd_entry := struct{};
            cmd_entry.+command := "." + cmd_name;
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
 * Reads from config/command_descriptions.cfg to avoid data segment bloat
 */
function GetCommandDescription(cmd_name)
    var cfg := ReadConfigFile(":hotbar:config/command_descriptions");
    if (cfg)
        var elem := FindConfigElem(cfg, cmd_name);
        if (elem)
            var desc := GetConfigString(elem, "desc");
            if (desc)
                return desc;
            endif
        endif
    endif

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

                // Build lean command entry (desc resolved at packet-write time)
                var cmd_entry := struct{};
                cmd_entry.+command := "." + cmd_name;
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
