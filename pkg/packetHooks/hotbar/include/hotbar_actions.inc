// ============================================================================
// Shared Hotbar Action Execution Functions
// ============================================================================
// This file contains common execution logic used by both:
// - pkg/packetHooks/Hotbar/hotbar_packets.inc (0xD7 protocol)
// - pkg/packetHooks/BFCommand/hotbarExecute.src (0xBF protocol)
//
// DRY: Single source of truth for action execution
// ============================================================================

use uo;
use os;

include ":attributes:attributes";
include ":hotbar:include/shared_constants";
include ":hotbar:include/hotbar_data";
include ":hotbar:include/hotbar_write";
include ":hotbar:hotbar_utils";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:AbilityRegistryBase";
include ":abilities:controllers/AbilityMonitor";
include ":abilities:models/Ability";

// ============================================================================
// ExecuteSkillAction - Unified skill execution
// ============================================================================
// Uses attributes.cfg as single source of truth for skill data
// 
// Parameters:
//   who      - Player character
//   skill_id - Skill ID from attributes.cfg
//   verbose  - If true, sends debug messages (default: false)
//
// Returns: 1 on success, 0 on failure

function ExecuteSkillAction(who, skill_id, verbose := 0)
    // Use shared_constants.inc to get skill data from attributes.cfg (single source of truth)
    var skill_data := GetSkillDataById(skill_id);
    
    if (!skill_data || skill_data == error)
        if (verbose)
            SendSysMessage(who, $"Skill ID {skill_id} não encontrada em attributes.cfg");
        endif
        return 0;
    endif
    
    var skill_name := skill_data.name;
    var script_path := skill_data.script;
    
    // Get the player's skill value
    var skill_value := CInt(AP_GetSkill(who, skill_name));
    
    if (skill_value <= 0)
        SendSysMessage(who, $"Você não possui pontos em {skill_name}!");
        return 0;
    endif
    
    // Script path already comes from attributes.cfg via GetSkillDataById
    if (!script_path || script_path == "")
        if (verbose)
            SendSysMessage(who, $"Skill {skill_name} não possui script!");
        endif
        return 0;
    endif
    
    // Check cooldown
    var delay := GetObjProperty(who, "delayskill");
    if (delay && delay > ReadGameClock())
        SendSysMessage(who, "Você ainda está fazendo outra coisa.");
        return 0;
    endif
    
    // Set cooldown (5 seconds)
    SetObjProperty(who, "delayskill", ReadGameClock() + 5);
    
    if (verbose)
        SendSysMessage(who, $"Usando {skill_name} (valor: {skill_value})");
    endif
    
    // Execute the skill script
    var result := start_script(script_path, who);
    if (!result)
        if (verbose)
            SendSysMessage(who, "Erro ao executar skill!");
        endif
        return 0;
    endif
    
    return 1;
endfunction

function ExecuteSpecialAction(who)
    // Execute special ability via existing system
    var script := Start_Script(":specialAbilities:specialAbilities", who);
    if (!script)
        SendSysMessage(who, "Erro ao executar habilidade especial.");
        return struct{ success := 0, cooldown_duration := 0 };
    endif
    
    // Calculate cooldown based on character level
    // Base cooldown: 180s, reduced by level (-30s every 4 levels, min 30s)
    var level := CInt(GetObjProperty(who, "level"));
    if (!level)
        level := 1;
    endif
    
    var cooldown_duration := 180 - (CInt(level / 4) * 30);
    if (cooldown_duration < 30)
        cooldown_duration := 30;
    endif
    
    return struct{ success := 1, cooldown_duration := cooldown_duration };
endfunction
// ============================================================================
// Execution Functions (delegate to shared/hotbar_actions.inc for DRY)
// ============================================================================

function ExecuteAbility(who, actionId)
    var hab_id := GetAbilityBySequentialId(who, actionId);
    if (hab_id == error)
        SendSysMessage(who, "Habilidade não encontrada.");
        return 0;
    endif

    var mobileWrapper := MobileWrapper(who);
    var abilityId := CInt(hab_id);
    var ability := Ability(abilityId);
    if (!ability.GetName())
        SendSysMessage(who, "Habilidade inválida.");
        return 0;
    endif

    var monitor := AbilityMonitor(ability);
    var result := monitor.ActivateOn(mobileWrapper);

    // If activation failed (validation error), don't send cooldown
    if (result.errortext)
        return 0;
    endif

    // Send cooldown notification to client
    // Format: [HAB_CD] Name|Seconds|IconHex - parsed by client AbilityCooldownManager
    if (ability.GetAbilityCooldown() > 0)
        SendSysMessageEx(who, $"[HAB_CD] {ability.GetName()}|{ability.GetAbilityCooldown()}|{ability.GetIcon()}", SSM_INFO);
    endif

    return 1;
endfunction

// Execute combat hotkey - data-driven approach with script paths
function ExecuteCombatHotkey(who, actionId)
    var hotkey_data := GetCombatHotkeyByIndex(who, actionId);

    if (hotkey_data.errortext)
        SendSysMessage(who, "Erro: Combat hotkey inválido.");
        return 0;
    endif

    // Execute script (works for both abilities and commands)
    var result := Start_Script(hotkey_data.script_path, who);
    if (!result)
        SendSysMessage(who, "Erro ao executar ação de combate.");
        return 0;
    endif

    // For abilities (not commands), handle cooldown
    if (hotkey_data.command_type == 0 && hotkey_data.delay > 0)
        var cooldown_prop := "SpecialCooldown_" + actionId;
        SetObjProperty(who, cooldown_prop, ReadGameClock() + hotkey_data.delay);
    endif

    return 1;
endfunction

// Send all Hotbar data in a single unified packet
function SendAllHotbarData(who)
    // Collect all data
    var hab_list := GetAbilitiesData(who);
    var combat_hotkeys := GetCombatHotkeys(who);
    var magias := GetSpellsData(who);
    var commands := GetCommandsData(who);
    var skills := GetSkillsData(who);
    
    // Calculate total packet size
    var packet_size := 6; // Header: PacketID(1) + Length(2) + Subcommand(2) + DataType(1)
    
    // Add size for each data type (count + data)
    var hab_size := CalculateAbilitiesSize(hab_list);
    var spec_size := CalculateCombatHotkeysSize(combat_hotkeys);
    var mag_size := CalculateSpellsSize(magias);
    var cmd_size := CalculateCommandsSize(commands);
    var skill_size := CalculateSkillsSize(skills);
    
    packet_size := packet_size + 2 + hab_size;
    packet_size := packet_size + 2 + spec_size;
    packet_size := packet_size + 2 + mag_size;
    packet_size := packet_size + 2 + cmd_size;
    packet_size := packet_size + 2 + skill_size;
    
    var packet := CreatePacket(PACKET_EXTENDED, packet_size);
    
    // Header
    packet.SetInt8(0, PACKET_EXTENDED);
    packet.SetInt16(1, packet_size);
    packet.SetInt16(3, SUBCOMMAND_RESPONSE);
    packet.SetInt8(5, 0xFF); // Unified packet marker
    
    var offset := 6;
    
    // 1. Habilidades
    offset := SetInt16BE(packet, offset, hab_list.size());
    foreach item in hab_list
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := WriteStringBE(packet, offset, item.classe);
        offset := SetInt16BE(packet, offset, item.required_level);
        packet.SetInt8(offset, item.is_active ? 1 : 0); offset += 1;
        packet.SetInt8(offset, item.is_purchased ? 1 : 0); offset += 1;
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, item.hue);
        offset := SetInt16BE(packet, offset, item.stamina_cost);
        offset := SetInt16BE(packet, offset, item.mana_cost);
        offset := SetInt16BE(packet, offset, item.delay);
        offset := WriteStringBE(packet, offset, item.requisite);
        offset := SetInt16BE(packet, offset, item.duration);
        offset := WriteStringBE(packet, offset, item.script);
    endforeach
    
    // 2. Combat Hotkeys
    offset := SetInt16BE(packet, offset, combat_hotkeys.size());
    foreach item in combat_hotkeys
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.classe);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, item.hue);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := SetInt16BE(packet, offset, CInt(item.delay));
        offset := SetInt16BE(packet, offset, CInt(item.remaining));
    endforeach
    
    // 3. Magias
    offset := SetInt16BE(packet, offset, magias.size());
    foreach item in magias
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, CStr(item.circle));
        offset := WriteStringBE(packet, offset, item.desc);
        offset := WriteStringBE(packet, offset, item.school);
        offset := SetInt16BE(packet, offset, item.iconid);
        offset := SetInt16BE(packet, offset, 0);  // hue
        offset := SetInt16BE(packet, offset, item.mana_cost);
        offset := SetInt16BE(packet, offset, 0);  // min_skill
        offset := SetInt16BE(packet, offset, 0);  // reagents length
        offset := SetInt16BE(packet, offset, item.id);
        var cmd_prefix := item.command_prefix ? item.command_prefix : "m";
        offset := WriteStringBE(packet, offset, cmd_prefix);
    endforeach
    
    // 4. Commands
    offset := SetInt16BE(packet, offset, commands.size());
    foreach item in commands
        offset := WriteStringBE(packet, offset, item.command);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.iconid);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := WriteStringBE(packet, offset, item.category);
    endforeach
    
    // 5. Skills
    offset := SetInt16BE(packet, offset, skills.size());
    foreach item in skills
        offset := WriteStringBE(packet, offset, item.name);
        offset := WriteStringBE(packet, offset, item.desc);
        offset := SetInt16BE(packet, offset, item.hue);
        offset := SetInt16BE(packet, offset, item.iconid);
        packet.SetInt8(offset, item.index); offset += 1;
        offset := SetInt16BE(packet, offset, item.current_value);
        offset := SetInt16BE(packet, offset, item.base_value);
        offset := SetInt16BE(packet, offset, item.cap);
        packet.SetInt8(offset, item.lock); offset += 1;
    endforeach
    
    packet.SendPacket(who);
    return true;
endfunction

function ExecutePosture(who, actionId)
    
    // Map actionId to postura type
    // 0 = Slash, 1 = Piercing, 2 = Blunt
    var posturaName;
    var posturaNamePT; // Nome em português para feedback
    
    case (actionId)
        0: 
            posturaName := "slash";
            posturaNamePT := "Cortante";
        1: 
            posturaName := "piercing";
            posturaNamePT := "Perfurante";
        2: 
            posturaName := "blunt";
            posturaNamePT := "Contusivo";
        default:
            SendSysMessage(who, "Erro: Postura inválida.");
            return 1;
    endcase
    
    
    // Execute postura change via existing system
    var script := Start_Script(":equipsys:golpe", {who, posturaName});
    if (!script)
        SendSysMessage(who, "Erro ao executar postura.");
        return 0;
    endif
    
    // Send feedback message in Portuguese for PosturaManager detection
    // This message will be detected by PosturaManager to show red mask on active postura
    SendSysMessage(who, $"Você adota postura {posturaNamePT}");
    
    return 1;
endfunction

function ExecuteSpell(who, actionId)
    
    // Get player's class and magia to prioritize correct spell configs
    var chardata := GetObjProperty(who, "chardata");
    var player_magia := "";
    
    if (chardata)
        player_magia := chardata.magia;
    endif
    
    // Build prioritized spell config list based on player type
    // Order matters! First match wins.
    var spell_configs := array{};
    var spell_types := array{};  // Parallel array for spell type
    
    // Check for Natureza Arcana trait (gives access to cantrips regardless of class)
    var has_natureza_arcana := (chardata.trait == "Natureza Arcana");
    
    // IMPORTANT: Cantrips FIRST for anyone with Natureza Arcana trait
    if (has_natureza_arcana)
        spell_configs.append(":spells:config/cantripspells");
        spell_types.append(SPELL_TYPE_CANTRIP);
    endif

    // Then add class-specific configs
    if (player_magia == "Shaman")
        spell_configs.append(":spells:config/shamanspells");
        spell_types.append(SPELL_TYPE_SHAMAN);
    elseif (player_magia == "spellcaster" || player_magia == "Arqueiro Arcano")
        spell_configs.append(":spells:config/cantripspells");
        spell_types.append(SPELL_TYPE_CANTRIP);
        spell_configs.append(":spells:config/spellcaster");
        spell_types.append(SPELL_TYPE_MAGE);
    elseif (player_magia == "Druid")
        spell_configs.append(":spells:config/druidspells");
        spell_types.append(SPELL_TYPE_DRUID);
    elseif (player_magia == "Sacerdote")
        spell_configs.append(":spells:config/priestspells");
        spell_types.append(SPELL_TYPE_PRIEST);
    endif
    
    // Ability-based spell access
    if (TemHabilidade(who, "Consagracao Divina"))
        spell_configs.append(":spells:config/paladinspells");
        spell_types.append(SPELL_TYPE_PALADIN);
    endif
    if (MobileWrapper(who).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
        spell_configs.append(":spells:config/demonhunterspells");
        spell_types.append(SPELL_TYPE_DEMONHUNTER);
    endif
    if (TemHabilidade(who, "Sensibilidade Arcana"))
        spell_configs.append(":spells:config/tricksterspells");
        spell_types.append(SPELL_TYPE_TRICKSTER);
    endif
    
    var found_spell_type := "";
    
    // Search configs in priority order - first match wins
    var i := 1;
    foreach cfg_path in spell_configs
        var spell_cfg := ReadConfigFile(cfg_path);
        if (!spell_cfg)
            i += 1;
            continue;
        endif
        
        var spell_elem := FindConfigElem(spell_cfg, actionId);
        if (spell_elem)
            found_spell_type := spell_types[i];
            break;
        endif
        i += 1;
    endforeach
    
    if (found_spell_type)
        // Execute via unified spell executor (by ID for efficiency)
        return ExecuteSpellById(who, actionId, found_spell_type);
    else
        SendSysMessage(who, $"[BINARY] Magia ID {actionId} não encontrada");
    endif
    
    return 1;
endfunction

function ExecuteCommand(who, actionId)
    // Use command_discovery.inc to get command by index (single source of truth)
    // actionId is the sequential index (1, 2, 3...) sent by client
    // Pass who so it picks GM vs player command list
    var cmd_data := GetCommandByIndex(actionId, who);
    
    if (cmd_data && cmd_data != error)
        var result := Start_Script(cmd_data.script_path, who);
        if (!result)
            SendSysMessage(who, "Erro ao executar comando.");
            return 0;
        endif
        return 1;
    endif
    
    SendSysMessage(who, $"Comando não encontrado (Index: {actionId})");
    return 0;
endfunction

function ExecuteSkill(who, actionId)
    // DRY: Use shared implementation from hotbar_actions.inc
    // verbose=1 because 0xBF protocol is used for active hotbar clicks
    return ExecuteSkillAction(who, actionId, 1);
endfunction
