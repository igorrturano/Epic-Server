// ============================================================================
// Hotbar Data Retrieval Module
// ============================================================================
// Purpose: Functions that retrieve player-specific hotbar data
// Functions: GetSpecialsData, GetCommandsData, GetSpellsData, GetAbilitiesData, GetSkillsData
// ============================================================================

use uo;
use os;
use cfgfile;

include ":charactercreation:habilidades";
include ":attributes:attributes";
include ":hotbar:include/icon_manager";
include ":hotbar:include/spell_index";
include ":hotbar:hotbar_utils";

// ============================================================================
// GetSpecialsData - Get player's special ability based on class
// ============================================================================
function GetSpecialsData(who)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        return array{};
    endif

    var specials := array{};
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
            "remaining" := remaining_cooldown
        };

        specials.append(special1);
    endif

    return specials;
endfunction

// ============================================================================
// GetCommandsData - Get list of available commands
// ============================================================================
// TODO CLIENT-SIDE WORK: Client should cache commands after first load
// Commands rarely change - cache to avoid re-transmission
// Expected savings: ~2-3KB per request after first load (15-20% reduction)
// ============================================================================
function GetCommandsData(who)
    var commands := array{};
    var index := 1;
    var icon_data;

    // Comandos Gerais (6)
    icon_data := GetCommandIcon(".ficha");
    commands.append(struct{"command" := ".ficha", "desc" := "Ficha do personagem", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    icon_data := GetCommandIcon(".point");
    commands.append(struct{"command" := ".point", "desc" := "Apontar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    icon_data := GetCommandIcon(".pularcerca");
    commands.append(struct{"command" := ".pularcerca", "desc" := "Pular cerca", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    icon_data := GetCommandIcon(".revistar");
    commands.append(struct{"command" := ".revistar", "desc" := "Revistar alguem", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    icon_data := GetCommandIcon(".senha");
    commands.append(struct{"command" := ".senha", "desc" := "Trocar senha", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    icon_data := GetCommandIcon(".showdmg");
    commands.append(struct{"command" := ".showdmg", "desc" := "Mostrar dano", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Geral"}); index += 1;

    // Magias (6)
    icon_data := GetCommandIcon(".arcoelemental");
    commands.append(struct{"command" := ".arcoelemental", "desc" := "Arco elemental", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    icon_data := GetCommandIcon(".arremessar");
    commands.append(struct{"command" := ".arremessar", "desc" := "Arremessar magia", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    icon_data := GetCommandIcon(".assumirforma");
    commands.append(struct{"command" := ".assumirforma", "desc" := "Assumir forma animal", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    icon_data := GetCommandIcon(".pilotar");
    commands.append(struct{"command" := ".pilotar", "desc" := "Pilotar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    icon_data := GetCommandIcon(".rituais");
    commands.append(struct{"command" := ".rituais", "desc" := "Rituais magicos", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    icon_data := GetCommandIcon(".voltar");
    commands.append(struct{"command" := ".voltar", "desc" := "Voltar forma original", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Magia"}); index += 1;

    // Combate (1)
    icon_data := GetCommandIcon(".golpe");
    commands.append(struct{"command" := ".golpe", "desc" := "Golpe especial de arma", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Combate"}); index += 1;

    // Skills (4)
    icon_data := GetCommandIcon(".acalmar");
    commands.append(struct{"command" := ".acalmar", "desc" := "Acalmar animal", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;

    icon_data := GetCommandIcon(".adestrar");
    commands.append(struct{"command" := ".adestrar", "desc" := "Adestrar criatura", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;

    icon_data := GetCommandIcon(".camuflar");
    commands.append(struct{"command" := ".camuflar", "desc" := "Camuflar-se", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;

    icon_data := GetCommandIcon(".estudar");
    commands.append(struct{"command" := ".estudar", "desc" := "Estudar item", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Skill"}); index += 1;

    // Roleplay (6)
    icon_data := GetCommandIcon(".actions");
    commands.append(struct{"command" := ".actions", "desc" := "Menu de acoes de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    icon_data := GetCommandIcon(".arrastar");
    commands.append(struct{"command" := ".arrastar", "desc" := "Arrastar corpo", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    icon_data := GetCommandIcon(".erguer");
    commands.append(struct{"command" := ".erguer", "desc" := "Erguer objeto", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    icon_data := GetCommandIcon(".ficharp");
    commands.append(struct{"command" := ".ficharp", "desc" := "Ficha de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    icon_data := GetCommandIcon(".lingua");
    commands.append(struct{"command" := ".lingua", "desc" := "Trocar idioma falado", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    icon_data := GetCommandIcon(".sounds");
    commands.append(struct{"command" := ".sounds", "desc" := "Tocar sons ambiente", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Roleplay"}); index += 1;

    // Pets (2)
    icon_data := GetCommandIcon(".pe");
    commands.append(struct{"command" := ".pe", "desc" := "Abrir menu de pet", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Pet"}); index += 1;

    icon_data := GetCommandIcon(".pet");
    commands.append(struct{"command" := ".pet", "desc" := "Comandos de pet", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Pet"}); index += 1;

    // Outros (5)
    icon_data := GetCommandIcon(".conquistas");
    commands.append(struct{"command" := ".conquistas", "desc" := "Ver conquistas", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;

    icon_data := GetCommandIcon(".fome");
    commands.append(struct{"command" := ".fome", "desc" := "Ver fome", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;

    icon_data := GetCommandIcon(".meditar");
    commands.append(struct{"command" := ".meditar", "desc" := "Meditar", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;

    icon_data := GetCommandIcon(".pontosrp");
    commands.append(struct{"command" := ".pontosrp", "desc" := "Pontos de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;

    icon_data := GetCommandIcon(".rps");
    commands.append(struct{"command" := ".rps", "desc" := "Sistema de RP", "iconid" := icon_data.iconid, "hue" := icon_data.hue, "index" := index, "category" := "Outros"}); index += 1;

    return commands;
endfunction

// ============================================================================
// GetAbilitiesData - Get player's class abilities
// ============================================================================
// Uses shared filtering logic from hotbar_utils.inc (DRY principle)
// ============================================================================
function GetAbilitiesData(who)
    // Use shared function instead of duplicating filtering logic
    var filtered_hab_names := GetFilteredAbilities(who);

    // Load config to get full ability data
    var habcfg := ReadConfigFile(":charactercreation:config/habilidades");
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

    foreach hab_name in filtered_hab_names
        var habelem := FindConfigElem(habcfg, hab_name);
        if (!habelem) continue; endif

        // Get icon
        var icon_data := GetAbilityIcon(hab_name, char_class);

        // Read additional fields
        var classes := GetConfigStringDictionary(habelem, "classe");
        var required_level := CInt(classes[char_class]);
        var uso := GetConfigString(habelem, "uso");

        // Check if purchased
        var has_purchased := 0;
        foreach key in (myhabs.keys())
            if (Lower(myhabs[key]) == Lower(hab_name))
                has_purchased := 1;
                break;
            endif
        endforeach

        // Build struct
        var hab_data := struct{
            "habilidade_id" := hab_index,  // Sequential ID
            "name" := hab_name,
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
        sleepms(2);
    endforeach

    return filtered_habs;
endfunction

// ============================================================================
// GetSkillsData - Get player's skills with current values
// ============================================================================
function GetSkillsData(who)
    var skills := array{};

    // Display names (can have spaces)
    var skill_names := array{
        "Alchemy",
        "Arcana",
        "Sneak",
        "Pickpocketing",
        "Awareness",
        "Mechanics",
        "Medicine",
        "Survival",
        "Gathering",
        "Handle Animal",
        "Theology"
    };

    // Internal names (used for AP_GetSkill - must match attributes.cfg)
    var skill_internal_names := array{
        "Alchemy",
        "Arcana",
        "Sneak",
        "Pickpocketing",
        "Awareness",
        "Mechanics",
        "Medicine",
        "Survival",
        "Gathering",
        "Handle_Animal",  // Internal name with underscore
        "Theology"
    };

    var skill_descs := array{
        "Avalie uma poção",
        "Conhecimento arcano",
        "Mover furtivamente",
        "Roubar objetos",
        "Encontrar armadilhas",
        "Desarmar armadilhas",
        "Primeiros socorros",
        "Sobrevivencia",
        "Coletar recursos",
        "Domar animais",
        "Conhecimento religioso"
    };

    var skill_ids := array{2, 16, 21, 33, 38, 48, 17, 20, 32, 35, 23};

    for i := 1 to skill_names.size()
        var skill_name := skill_names[i];
        var skill_internal_name := skill_internal_names[i];

        var skill_value := CInt(AP_GetSkill(who, skill_internal_name));
        var skill_base := CInt(AP_GetTrueSkill(who, skill_internal_name));
        var skill_cap := CInt(AP_GetSkillCap(who, skill_internal_name));

        // Get icon from icons.cfg
        var icon_data := GetSkillIcon(skill_name);

        skills.append(struct{
            "name" := skill_name,  // Display name
            "desc" := skill_descs[i],
            "iconid" := icon_data.iconid,
            "hue" := icon_data.hue,
            "index" := skill_ids[i],
            "current_value" := skill_value,
            "base_value" := skill_base,
            "cap" := skill_cap,
            "lock" := 0  // Default: Up (0=up, 1=down, 2=locked)
        });
    endfor

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
    var has_demonhunter := TemHabilidade(who, "Sigilo de Protecao");

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
                sleepms(2);
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
                sleepms(2);
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
                sleepms(2);
            endforeach
        endif
    endif

    return spells;
endfunction
