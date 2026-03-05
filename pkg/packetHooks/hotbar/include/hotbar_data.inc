// ============================================================================
// Hotbar Data Retrieval Module
// ============================================================================
// Purpose: Functions that retrieve player-specific hotbar data
// Functions: GetCombatHotkeys, GetCommandsData, GetSpellsData, GetAbilitiesData, GetSkillsData
// ============================================================================

use uo;
use os;
use cfgfile;

include ":charactercreation:habilidades";
include ":attributes:attributes";
include ":hotbar:include/icon_manager";
include ":hotbar:include/spell_index";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";
include ":hotbar:include/command_discovery"; // Auto-discover commands
include ":hotbar:hotbar_utils";

// ============================================================================
// GetCombatHotkeys - Get player's combat hotkeys (ability + commands)
// ============================================================================
// Returns array with:
// - Class-specific special ability (index 1-5)
// - .golpe command (index 10)
// - .combate command (index 11)
// ============================================================================
function GetCombatHotkeys(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return array{};
    endif

    var combat_hotkeys := array{};
    var special_name := "";
    var special_class := "";
    var special_hue := 0;
    var special_index := 0;

    // Determine which special ability based on class/path
    case (chardata.classe)
        "Guerreiro":
            special_name := "Ataque Extra";
            special_class := "Guerreiro";
            special_hue := 33; // Red
            special_index := 1;

        "Ladino":
            special_name := "Evasao";
            special_class := "Ladino";
            special_hue := 63; // Green
            special_index := 2;

        "Sabio":
            // Check wisdom_path for Sabio
            if (chardata.wisdom_path == "Engenheiro")
                special_name := "Recarga Rapida";
                special_class := "Engenheiro";
                special_hue := 2101; // Gray
                special_index := 3;
            elseif (chardata.magia == "Arqueiro Arcano")
                special_name := "Velocidade Arcana";
                special_class := "Arqueiro Arcano";
                special_hue := 16; // Purple
                special_index := 5;
            else
                // Mago, Xamã, Clérigo, etc.
                special_name := "Conjuracao Rapida";
                special_class := "Conjurador";
                special_hue := 88; // Blue
                special_index := 4;
            endif
    endcase

    // Calculate dynamic cooldown based on level
    var level := CInt(GetObjProperty(who, "Level"));
    if (!level)
        level := 1;
    endif
    var base_cooldown := 180; // 3 minutes base
    var cooldown_reduction := Floor(level / 4) * 30; // Reduce 30s every 4 levels
    var final_cooldown := base_cooldown - cooldown_reduction;
    if (final_cooldown < 30)
        final_cooldown := 30; // Minimum 30 seconds
    endif

    // Check if special is currently on cooldown
    var remaining_cooldown := CInt(TS_GetCooldown(who, "special_ability"));
    if (!remaining_cooldown || remaining_cooldown < 0)
        remaining_cooldown := 0;
    endif

    // Create special if found
    if (special_name != "")
        // Set specific description based on special type
        var special_desc := "";
        case (special_index)
            1: // Ataque Extra (Guerreiro)
                special_desc := "Executa um ataque instantâneo adicional";
            2: // Evasão (Ladino)
                special_desc := "Fica invisível por 0.5s e recua 4 tiles";
            3: // Recarga Rápida (Engenheiro)
                special_desc := "Adiciona 1 bala instantaneamente na arma de fogo";
            4: // Conjuração Rápida (Conjurador)
                special_desc := "Permite conjurar a próxima magia instantaneamente";
            5: // Velocidade Arcana (Arqueiro Arcano)
                special_desc := "Aumenta a velocidade de movimento por 4 segundos";
            default:
                special_desc := "Habilidade especial de " + special_class;
        endcase

        // Get icon from icons.cfg
        var icon_data := GetSpecialIcon(special_name);

        var special1 := struct{
            "name" := special_name,
            "classe" := special_class,
            "desc" := special_desc,
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "index" := special_index,
            "delay" := final_cooldown,
            "remaining" := remaining_cooldown,
            "command_type" := 0,  // 0 = ability, 1 = command
            "script_path" := ":specialAbilities:specialAbilities"
        };

        combat_hotkeys.append(special1);
    endif

    // Add .golpe command as combat hotkey
    var golpe_icon := GetCommandIcon("golpe");
    if (!golpe_icon.errortext)
        var golpe := struct{
            "name" := ".golpe",
            "classe" := special_class,
            "desc" := "Golpe especial de arma",
            "iconid" := golpe_icon.iconid,
            "hue" := golpe_icon.hue,
            "index" := 10,  // New index for command-type hotkeys
            "delay" := 0,  // Commands handle their own cooldowns
            "remaining" := 0,
            "command_type" := 1,  // Flag to identify as command
            "script_path" := ":equipsys:commands/player/golpe"
        };
        combat_hotkeys.append(golpe);
    endif

    // Add .combate command as combat hotkey
    var combate_icon := GetCommandIcon("combate");
    if (!combate_icon.errortext)
        var combate := struct{
            "name" := ".combate",
            "classe" := special_class,
            "desc" := "Mudar postura de combate",
            "iconid" := combate_icon.iconid,
            "hue" := combate_icon.hue,
            "index" := 11,
            "delay" := 0,
            "remaining" := 0,
            "command_type" := 1,
            "script_path" := ":charactercreation:commands/player/combate"
        };
        combat_hotkeys.append(combate);
    endif

    return combat_hotkeys;
endfunction

// ============================================================================
// GetCombatHotkeyByIndex - Get specific combat hotkey by index
// ============================================================================
function GetCombatHotkeyByIndex(who, actionId)
    var hotkeys := GetCombatHotkeys(who);

    foreach hotkey in (hotkeys)
        if (hotkey.index == actionId)
            return hotkey;
        endif
    endforeach

    return error{"errortext" := "Combat hotkey not found"};
endfunction

// ============================================================================
// GetCommandsData - Get list of available commands
// ============================================================================
// ✅ PHASE 2: Now uses auto-discovery from filesystem instead of hardcoded list
// Filtering logic:
// - GM (cmdlevel > 1): Show ALL commands from all command folders
// - Player (cmdlevel <= 1): Show ONLY player commands
// ============================================================================
function GetCommandsData(who)
    if (who.cmdlevel > 0)
        return DiscoverGMCommands();
    endif

    return DiscoverPlayerCommands();
endfunction

// ============================================================================
// GetAbilitiesData - Get player's class abilities
// ============================================================================
// Uses shared filtering logic from hotbar_utils.inc (DRY principle)
// ============================================================================
function GetAbilitiesData(who)
    // Use shared function instead of duplicating filtering logic
    var filtered_hab_ids := GetFilteredAbilities(who);

    // Load config to get full ability data
    var habcfg := ReadConfigFile(":abilities:configs/habilidades");
    if (!habcfg)
        return array{};
    endif

    // Get player data for icon lookup
    var chardata := GetObjProperty(who, "chardata");
    var char_class := chardata.classe;
    var myhabs := GetObjProperty(who, "myhabs");
    if (!myhabs) myhabs := dictionary; endif

    // Build full data structures for each filtered ability
    var filtered_habs := array{};
    var hab_index := 1;

    foreach hab_id in filtered_hab_ids
        var habelem := FindConfigElem(habcfg, hab_id);
        if (!habelem) continue; endif
        // Get ability name from config
        var ability_name := GetConfigString(habelem, "nome");

        // Get icon
        var icon_data := GetAbilityIcon(hab_id);

        // Read additional fields
        var classes := GetConfigStringDictionary(habelem, "classe");
        var required_level := CInt(classes[char_class]);
        var uso := GetConfigString(habelem, "uso");

        // Check if purchased
        var has_purchased := 0;
        foreach key in (myhabs.keys())
            if (Lower(myhabs[key]) == Lower(ability_name))
                has_purchased := 1;
                break;
            endif
        endforeach

        // Build struct
        var hab_data := struct{
            "habilidade_id" := hab_index,  // Sequential ID
            "name" := ability_name,
            "classe" := char_class,
            "desc" := GetConfigString(habelem, "desc"),
            "required_level" := required_level,
            "is_active" := (uso == "Ativa"),
            "is_purchased" := has_purchased,
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "stamina_cost" := CInt(GetConfigString(habelem, "stam")),
            "mana_cost" := CInt(GetConfigString(habelem, "mana")),
            "delay" := CInt(GetConfigString(habelem, "delay")),
            "requisite" := GetConfigString(habelem, "requisito"),
            "duration" := CInt(GetConfigString(habelem, "duracao")),
            "script" := GetConfigString(habelem, "script")
        };

        filtered_habs.append(hab_data);
        hab_index := hab_index + 1;
    endforeach

    return filtered_habs;
endfunction

// ============================================================================
// GetSkillsData - Get player's skills with current values
// ============================================================================
// Filtering logic:
// - GM (cmdlevel > 1): Show ALL skills from attributes.cfg
// - Player (cmdlevel <= 1): Show ONLY skills from chardata.Skills
// ============================================================================
function GetSkillsData(who)
    var skills := array{};
    
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return skills;
    endif
    
    var is_gm := (who.cmdlevel > 1);
    var player_skills := chardata.Skills;
    if (!player_skills)
        player_skills := dictionary;
    endif
    
    // Load attributes.cfg
    var attr_cfg := ReadConfigFile(":attributes:attributes");
    if (!attr_cfg)
        return skills;
    endif
    
    // Read all skill names from config and filter by those with Script (active skills)
    var all_skill_names := GetConfigStringKeys(attr_cfg);
    
    foreach skill_name in all_skill_names
        // Get skill config element
        var skill_elem := FindConfigElem(attr_cfg, skill_name);
        if (!skill_elem)
            continue;
        endif
        
        // Only show skills that have a Script (active/usable skills)
        var skill_script := GetConfigString(skill_elem, "Script");
        if (!skill_script)
            continue;
        endif
        // Check if player should see this skill
        var is_eligible := 0;
        
        if (is_gm)
            // GM sees all skills
            is_eligible := 1;
        else
            // Player sees only skills in chardata.Skills
            if (player_skills[skill_name])
                is_eligible := 1;
            endif
        endif
        
        if (!is_eligible)
            continue;
        endif
        
        // Get values from attributes.cfg
        var skill_id := GetConfigInt(skill_elem, "SkillId");
        var tooltip_desc := GetConfigString(skill_elem, "TooltipDesc");
        var icon_id := GetConfigInt(skill_elem, "Icon");
        var icon_hue := GetConfigInt(skill_elem, "Hue");
        
        // Defaults if not defined
        if (!icon_id)
            icon_id := 0x9b90;
        endif
        if (!tooltip_desc)
            tooltip_desc := "";
        endif
        
        // Get player's current skill values
        var skill_value := CInt(AP_GetSkill(who, skill_name));
        var skill_base := CInt(AP_GetTrueSkill(who, skill_name));
        var skill_cap := CInt(AP_GetSkillCap(who, skill_name));
        
        // Display name: replace "_" with " "
        var display_name := StrReplace(skill_name, "_", " ");
        
        skills.append(struct{
            "name" := display_name,
            "desc" := tooltip_desc,
            "iconid" := icon_id,
            "hue" := icon_hue,
            "index" := skill_id,
            "current_value" := skill_value,
            "base_value" := skill_base,
            "cap" := skill_cap,
            "lock" := 0
        });
    endforeach
    
    return skills;
endfunction

// ============================================================================
// GetSpellsData - Get player's spells from grimoire and memorized lists
// ============================================================================
// Loads spells from:
// 1. Spellbook in backpack
// 2. Cantrips (for eligible classes)
// 3. Memorized spells (Shaman, Priest, Druid)
// 4. Ability-based spells (Paladin, Trickster, Demon Hunter)
// ============================================================================
function GetSpellsData(who)
    var spells := array{};
    var chardata := GetObjProperty(who, "chardata");

    // GM: load ALL spells from ALL config files
    // if (who.cmdlevel > 1)
    //     return GetAllSpellsForGM();
    // endif

    // Find spellbook in backpack
    var backpack := who.backpack;
    if (!backpack)
        return spells;
    endif

    // Search for all spellbooks
    foreach item in EnumerateItemsInContainer(backpack)
        // Check if it's a spellbook (0xEFA, 0x2253, 0x9E80)
        if (item.objtype == 0xEFA || item.objtype == 0x2253 || item.objtype == 0x9E80)
            var booktype := GetObjProperty(item, "Booktype");
            if (!booktype)
                booktype := "spellcaster";
            endif

            // Determine config path based on booktype
            var spell_cfg_path := ":spells:config/spellcaster";
            case (booktype)
                "spellcaster":
                "mage":
                    spell_cfg_path := ":spells:config/spellcaster";
                "priest":
                    spell_cfg_path := ":spells:config/priestspells";
                "druid":
                    spell_cfg_path := ":spells:config/druidspells";
                "shaman":
                "Shaman":
                    spell_cfg_path := ":spells:config/shamanspells";
                "paladin":
                    spell_cfg_path := ":spells:config/paladinspells";
                "demonhunter":
                    spell_cfg_path := ":spells:config/demonhunterspells";
                "trickster":
                    spell_cfg_path := ":spells:config/tricksterspells";
                "cantrip":
                    spell_cfg_path := ":spells:config/cantripspells";
            endcase

            var spell_cfg := ReadConfigFile(spell_cfg_path);
            var spells_array := GetObjProperty(item, "spells");

            if (!spells_array || TypeOf(spells_array) != "Array")
                continue;
            endif

            foreach spell_struct in spells_array
                if (TypeOf(spell_struct) == "Struct")
                    var spell_id := spell_struct.id;
                    var spell_elem := FindConfigElem(spell_cfg, spell_id);

                    var spell_name := "<unknown>";
                    var spell_circle := 1;
                    var spell_mana := 0;

                    if (spell_elem)
                        spell_name := GetConfigString(spell_elem, "Name");
                        spell_circle := CInt(GetConfigString(spell_elem, "Circle"));
                        spell_mana := CInt(GetConfigString(spell_elem, "Mana"));
                    endif

                    // Read Icon and Hue directly from config
                    var icon_id := GetConfigInt(spell_elem, "Icon");
                    var icon_hue := GetConfigInt(spell_elem, "Hue");
                    if (!icon_id)
                        icon_id := 0x5DCD; // Default spell scroll icon
                    endif

                    var spell := struct{};
                    spell.+id := spell_id;
                    spell.+name := spell_name;
                    spell.+magic_path := spell_struct.magic_path;
                    spell.+school := spell_struct.school;
                    spell.+circle := spell_circle;
                    spell.+mana_cost := spell_mana;
                    spell.+iconid := icon_id;
                    spell.+hue := icon_hue;

                    spells.append(spell);
                endif
            endforeach
        endif
    endforeach

    // Load cantrips if player has the trait
    if (chardata)
        var player_magia := chardata.magia;
        var trait := chardata.trait;

        if (player_magia == "spellcaster" || trait == "Natureza Arcana" || player_magia == "Arqueiro Arcano")
            var cantrip_cfg := ReadConfigFile(":spells:config/cantripspells");

            if (cantrip_cfg)
                foreach cantrip_id in GetConfigIntKeys(cantrip_cfg)
                    var cantrip_elem := FindConfigElem(cantrip_cfg, cantrip_id);
                    if (cantrip_elem)
                        var cantrip_name := GetConfigString(cantrip_elem, "Name");

                        // Read Icon and Hue directly from config
                        var icon_id := GetConfigInt(cantrip_elem, "Icon");
                        var icon_hue := GetConfigInt(cantrip_elem, "Hue");
                        if (!icon_id)
                            icon_id := 0x9b9f; // Default cantrip icon
                        endif

                        var spell := struct{};
                        spell.+id := cantrip_id;
                        spell.+name := cantrip_name;
                        spell.+magic_path := "cantripspells";
                        spell.+school := GetConfigString(cantrip_elem, "School");
                        spell.+circle := CInt(GetConfigString(cantrip_elem, "Circle"));
                        spell.+mana_cost := CInt(GetConfigString(cantrip_elem, "Mana"));
                        spell.+iconid := icon_id;
                        spell.+hue := icon_hue;

                        spells.append(spell);
                    endif
                endforeach
            endif
        endif
    endif

    // Load Shaman memorized spells
    if (chardata && chardata.magia == "Shaman")
        var shaman_list := GetObjProperty(who, "shaman_list");

        if (shaman_list && TypeOf(shaman_list) == "Array" && shaman_list.size() > 0)
            var shaman_cfg := ReadConfigFile(":spells:config/shamanspells");

            if (shaman_cfg)
                foreach spell_data in shaman_list
                    if (!spell_data.id)
                        continue;
                    endif

                    var spell_elem := FindConfigElem(shaman_cfg, spell_data.id);
                    if (!spell_elem)
                        continue;
                    endif

                    var spell_name := GetConfigString(spell_elem, "Name");
                    if (!spell_name)
                        continue;
                    endif

                    // Read Icon and Hue directly from config
                    var icon_id := GetConfigInt(spell_elem, "Icon");
                    var icon_hue := GetConfigInt(spell_elem, "Hue");
                    if (!icon_id)
                        icon_id := 0x59dd; // Default shaman icon
                        icon_hue := 1150;
                    endif

                    var spell := struct{};
                    spell.+id := spell_data.id;
                    spell.+name := spell_name;
                    spell.+magic_path := "Shaman";
                    spell.+school := "Shaman";
                    spell.+circle := CInt(spell_data.circle);
                    spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    spell.+iconid := icon_id;
                    spell.+hue := icon_hue;

                    spells.append(spell);
                endforeach
            endif
        endif
    endif

    // Load Priest memorized spells
    if (chardata && (chardata.magia == "Priest" || chardata.magia == "Sacerdote"))
        var priest_list := GetObjProperty(who, "priest_list");

        if (priest_list && TypeOf(priest_list) == "Array" && priest_list.size() > 0)
            var priest_cfg := ReadConfigFile(":spells:config/priestspells");

            if (priest_cfg)
                foreach spell_data in priest_list
                    var spell_elem;
                    var spell_id;
                    var spell_name;

                    if (spell_data.id)
                        spell_id := spell_data.id;
                        spell_elem := FindConfigElem(priest_cfg, spell_id);
                    elseif (spell_data.name)
                        spell_name := spell_data.name;
                        spell_id := GetSpellIdByName("priest", spell_name);
                        if (spell_id != error)
                            spell_elem := GetSpellById("priest", spell_id);
                        endif
                    else
                        continue;
                    endif

                    if (!spell_elem)
                        continue;
                    endif

                    if (!spell_name)
                        spell_name := GetConfigString(spell_elem, "Name");
                    endif

                    if (!spell_name)
                        continue;
                    endif

                    // Read Icon and Hue directly from config
                    var icon_id := GetConfigInt(spell_elem, "Icon");
                    var icon_hue := GetConfigInt(spell_elem, "Hue");
                    if (!icon_id)
                        icon_id := 0x59db; // Default priest icon
                        icon_hue := 1153;
                    endif

                    var spell := struct{};
                    spell.+id := spell_id;
                    spell.+name := spell_name;
                    spell.+magic_path := "Priest";
                    spell.+school := "Priest";
                    spell.+circle := CInt(spell_data.circle);
                    spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    spell.+iconid := icon_id;
                    spell.+hue := icon_hue;

                    spells.append(spell);
                endforeach
            endif
        endif
    endif

    // Load Druid memorized spells
    if (chardata && (chardata.magia == "Druid" || chardata.magia == "Druida"))
        var druid_list := GetObjProperty(who, "druid_list");

        if (druid_list && TypeOf(druid_list) == "Array" && druid_list.size() > 0)
            var druid_cfg := ReadConfigFile(":spells:config/druidspells");

            if (druid_cfg)
                foreach spell_data in druid_list
                    var spell_elem;
                    var spell_id;
                    var spell_name;

                    if (spell_data.id)
                        spell_id := spell_data.id;
                        spell_elem := FindConfigElem(druid_cfg, spell_id);
                    elseif (spell_data.name)
                        spell_name := spell_data.name;
                        spell_id := GetSpellIdByName("druid", spell_name);
                        if (spell_id != error)
                            spell_elem := GetSpellById("druid", spell_id);
                        endif
                    else
                        continue;
                    endif

                    if (!spell_elem)
                        continue;
                    endif

                    if (!spell_name)
                        spell_name := GetConfigString(spell_elem, "Name");
                    endif

                    if (!spell_name)
                        continue;
                    endif

                    // Read Icon and Hue directly from config
                    var icon_id := GetConfigInt(spell_elem, "Icon");
                    var icon_hue := GetConfigInt(spell_elem, "Hue");
                    if (!icon_id)
                        icon_id := 0x59de; // Default druid icon
                        icon_hue := 1152;
                    endif

                    var spell := struct{};
                    spell.+id := spell_id;
                    spell.+name := spell_name;
                    spell.+magic_path := "Druid";
                    spell.+school := "Druid";
                    spell.+circle := CInt(spell_data.circle);
                    spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                    spell.+iconid := icon_id;
                    spell.+hue := icon_hue;

                    spells.append(spell);
                endforeach
            endif
        endif
    endif

    // OPTIMIZED: Check all ability-based spell access once
    var has_paladin := TemHabilidade(who, "Consagracao Divina");
    var has_trickster := TemHabilidade(who, "Sensibilidade Arcana");
    var has_demonhunter := MobileWrapper(who).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO);

    // Load Paladin spells
    if (has_paladin)
        var paladin_cfg := ReadConfigFile(":spells:config/paladinspells");

        if (paladin_cfg)
            var theology_skill := CInt(AP_GetSkill(who, "Theology"));
            var max_circle := theology_skill / 20;
            if (max_circle < 1)
                max_circle := 1;
            endif

            foreach spell_id in GetConfigIntKeys(paladin_cfg)
                var spell_elem := FindConfigElem(paladin_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif

                var spell_circle := CInt(GetConfigInt(spell_elem, "Circle"));
                if (spell_circle > max_circle)
                    continue;
                endif

                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif

                // Read Icon and Hue directly from config
                var icon_id := GetConfigInt(spell_elem, "Icon");
                var icon_hue := GetConfigInt(spell_elem, "Hue");
                if (!icon_id)
                    icon_id := 0x9BA6; // Default paladin icon
                    icon_hue := 1153;
                endif

                var spell := struct{};
                spell.+id := spell_id;
                spell.+name := spell_name;
                spell.+school := "Paladino";
                spell.+circle := spell_circle;
                spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                spell.+iconid := icon_id;
                spell.+hue := icon_hue;
                spell.+command_prefix := "p";

                spells.append(spell);
            endforeach
        endif
    endif

    // Load Trickster spells
    if (has_trickster)
        var trickster_cfg := ReadConfigFile(":spells:config/tricksterspells");

        if (trickster_cfg)
            foreach spell_id in GetConfigIntKeys(trickster_cfg)
                var spell_elem := FindConfigElem(trickster_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif

                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif

                // Read Icon and Hue directly from config
                var icon_id := GetConfigInt(spell_elem, "Icon");
                var icon_hue := GetConfigInt(spell_elem, "Hue");
                if (!icon_id)
                    icon_id := 0x9b99; // Default trickster icon
                endif

                var spell := struct{};
                spell.+id := spell_id;
                spell.+name := spell_name;
                spell.+school := "Trickster";
                spell.+circle := CInt(GetConfigInt(spell_elem, "Circle"));
                spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                spell.+iconid := icon_id;
                spell.+hue := icon_hue;
                spell.+command_prefix := "m";

                spells.append(spell);
            endforeach
        endif
    endif

    // Load Demon Hunter spells
    if (has_demonhunter)
        var dh_cfg := ReadConfigFile(":spells:config/demonhunterspells");

        if (dh_cfg)
            foreach spell_id in GetConfigIntKeys(dh_cfg)
                var spell_elem := FindConfigElem(dh_cfg, spell_id);
                if (!spell_elem)
                    continue;
                endif

                var spell_name := GetConfigString(spell_elem, "Name");
                if (!spell_name)
                    continue;
                endif

                // Read Icon and Hue directly from config
                var icon_id := GetConfigInt(spell_elem, "Icon");
                var icon_hue := GetConfigInt(spell_elem, "Hue");
                if (!icon_id)
                    icon_id := 0x9b96; // Default demon hunter icon
                endif

                var spell := struct{};
                spell.+id := spell_id;
                spell.+name := spell_name;
                spell.+school := "Demon Hunter";
                spell.+circle := CInt(GetConfigInt(spell_elem, "Circle"));
                spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
                spell.+iconid := icon_id;
                spell.+hue := icon_hue;
                spell.+command_prefix := "c";

                spells.append(spell);
            endforeach
        endif
    endif

    return spells;
endfunction

// ============================================================================
// GetAllSpellsForGM - Load ALL spells from ALL config files (GM only)
// ============================================================================
function GetAllSpellsForGM()
    var spells := LoadSpellsFromCfg(":spells:config/cantripspells",     "Cantrip");
    // spells := spells + LoadSpellsFromCfg(":spells:config/priestspells",      "Priest");
    // spells := spells + LoadSpellsFromCfg(":spells:config/shamanspells",      "Shaman");
    // spells := spells + LoadSpellsFromCfg(":spells:config/druidspells",       "Druid");
    // spells := spells + LoadSpellsFromCfg(":spells:config/paladinspells",     "Paladino");
    // spells := spells + LoadSpellsFromCfg(":spells:config/demonhunterspells", "Demon Hunter");
    // spells := spells + LoadSpellsFromCfg(":spells:config/tricksterspells",   "Trickster");
    // spells := spells + LoadSpellsFromCfg(":spells:config/spellcaster",       "Spellcaster");
    return spells;
endfunction

function LoadSpellsFromCfg(cfg_path, default_school)
    var spells := array{};

    var spell_cfg := ReadConfigFile(cfg_path);
    if (!spell_cfg)
        return spells;
    endif

    var spell_ids := GetConfigIntKeys(spell_cfg);
    if (!spell_ids)
        return spells;
    endif

    foreach spell_id in spell_ids
        var spell_elem := FindConfigElem(spell_cfg, spell_id);
        if (!spell_elem)
            continue;
        endif

        var spell_name := GetConfigString(spell_elem, "Name");
        if (!spell_name)
            continue;
        endif

        var icon_id := GetConfigInt(spell_elem, "Icon");
        var icon_hue := GetConfigInt(spell_elem, "Hue");
        if (!icon_id)
            icon_id := 0x5DCD;
        endif

        var school := GetConfigString(spell_elem, "School");
        if (!school)
            school := default_school;
        endif

        var desc := GetConfigString(spell_elem, "Desc");
        if (!desc)
            desc := "";
        endif

        var spell := struct{};
        spell.+id := spell_id;
        spell.+name := spell_name;
        spell.+magic_path := default_school;
        spell.+school := school;
        spell.+circle := CInt(GetConfigString(spell_elem, "Circle"));
        spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));
        spell.+iconid := icon_id;
        spell.+hue := icon_hue;
        spell.+desc := desc;
        spell.+command_prefix := "m";

        spells.append(spell);
    endforeach

    return spells;
endfunction
