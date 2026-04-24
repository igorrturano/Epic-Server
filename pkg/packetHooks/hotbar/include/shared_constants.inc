/*
 * Shared Constants - Single Source of Truth
 *
 * Centralizes all constants, skill mappings, and action types
 * used by both Hotbar (data) and BFCommand (execution).
 *
 * IMPORTANT: This is the ONLY place where action types are defined.
 * Skill mappings are discovered dynamically from attributes.cfg.
 */

use cfgfile;

include ":attributes:attributes";

// =============================================================================
// Action Types (Binary Protocol 0xBF/0x0099)
// =============================================================================

const ACTION_TYPE_POSTURE := 0x01;   // Combat stances (.slash, .piercing, .blunt)
const ACTION_TYPE_ABILITY := 0x02;   // Class abilities
const ACTION_TYPE_SPECIAL := 0x03;   // Special abilities (Extra Attack, Evasion, etc.)
const ACTION_TYPE_SPELL := 0x04;     // Custom spells (Paladin, Druid, etc.)
const ACTION_TYPE_COMMAND := 0x05;   // Text commands (.ficha, .golpe, etc.)
const ACTION_TYPE_SKILL := 0x06;     // Clickable skills (Alchemy, Sneak, etc.)

// =============================================================================
// Packet IDs
// =============================================================================

const PACKET_EXTENDED := 0xD7;

// Subcommands (Client ↔ Server)
const SUBCOMMAND_REQUEST := 0x50;
const SUBCOMMAND_EXECUTE := 0x51;
const SUBCOMMAND_RESPONSE := 0x52;
const SUBCOMMAND_TOOLTIP_REQUEST := 0x60;
const SUBCOMMAND_TOOLTIP_RESPONSE := 0x61;

// Response types (Server → Client)
const SUBCOMMAND_ABILITIES_RESPONSE := 0x53;
const SUBCOMMAND_SPECIALS_RESPONSE := 0x54;
const SUBCOMMAND_SPELLS_RESPONSE := 0x55;
const SUBCOMMAND_SKILLS_RESPONSE := 0x56;
const SUBCOMMAND_COMMANDS_RESPONSE := 0x59;
const SUBCOMMAND_CUSTOM_BUFF := 0x62;

// =============================================================================
// Active Skills Discovery (from attributes.cfg)
// =============================================================================
// Dynamically discovers skills with Script field = active skills for hotbar

var g_active_skills_cache := 0;

/*
 * Get all active skills (skills with Script field in attributes.cfg)
 * Returns array of struct{ name, skillid, script, desc }
 */
function GetActiveSkills()
    // Cache the result
    if (g_active_skills_cache)
        return g_active_skills_cache;
    endif

    var active_skills := array{};
    var attr_cfg := ReadConfigFile(":attributes:attributes");

    if (!attr_cfg)
        Print("[SharedConstants] ERROR: Could not load attributes.cfg");
        return array{};
    endif

    var all_attrs := GetConfigStringKeys(attr_cfg);

    foreach attr_name in all_attrs
        var attr_elem := FindConfigElem(attr_cfg, attr_name);
        var attr_type := GetConfigString(attr_elem, "AttributeType");

        // Only process Skills
        if (attr_type != "Skill")
            continue;
        endif

        // Check if skill has a Script (= active skill)
        var script := GetConfigString(attr_elem, "Script");
        if (!script || script == "")
            continue;  // Skip passive skills (no script)
        endif

        // This skill is active! Add to list
        var skill_data := struct{};
        skill_data.+name := attr_name;
        skill_data.+skillid := CInt(GetConfigString(attr_elem, "SkillId"));
        skill_data.+script := script;
        skill_data.+desc := GetConfigString(attr_elem, "TooltipDesc");

        active_skills.append(skill_data);
    endforeach

    Print($"[SharedConstants] Discovered {active_skills.size()} active skills");

    g_active_skills_cache := active_skills;
    return active_skills;
endfunction

/*
 * Get skill data by SkillId
 * @param skill_id Custom SkillId
 * @return struct{name, skillid, script, desc} or error
 */
function GetSkillDataById(skill_id)
    var active_skills := GetActiveSkills();

    foreach skill in active_skills
        if (skill.skillid == skill_id)
            return skill;
        endif
    endforeach

    return error;
endfunction

/*
 * Get skill data by name
 * @param skill_name Skill name (e.g., "Alchemy", "Sneak")
 * @return struct{name, skillid, script, desc} or error
 */
function GetSkillDataByName(skill_name)
    var active_skills := GetActiveSkills();

    foreach skill in active_skills
        if (skill.name == skill_name)
            return skill;
        endif
    endforeach

    return error;
endfunction

// =============================================================================
// Spell Config Paths
// =============================================================================

/*
 * Get spell config path from spell type name
 * @param spell_type Internal name (e.g., "paladin", "priest")
 * @return Full config path or empty string
 */
function GetSpellConfigPath(spell_type)
    case (spell_type)
        "spellcaster":
        "evocation":
            return ":spells:config/evocation";
        "necromancy":
            return ":spells:config/necromancy";
        "illusion":
            return ":spells:config/illusion";
        "swordmage":
            return ":spells:config/swordmage";
        "paladin":
            return ":spells:config/paladin";
        "druid":
            return ":spells:config/druid";
        "priest":
            return ":spells:config/priest";
        "shaman":
            return ":spells:config/shaman";
        "demonhunter":
            return ":spells:config/demonhunter";
        "trickster":
            return ":spells:config/trickster";
        "cantrip":
            return ":spells:config/cantrips";
        default:
            return "";
    endcase
endfunction

/*
 * Get command prefix for spell type (for text command fallback)
 * @param spell_type Internal name
 * @return Command prefix (e.g., "p" for paladin = .p command)
 */
function GetSpellCommandPrefix(spell_type)
    case (spell_type)
        "spellcaster":
            return "m";   // .m (mage)
        "paladin":
            return "p";   // .p (paladin)
        "druid":
        "priest":
        "shaman":
            return "d";   // .d (divine/druid)
        "demonhunter":
            return "c";   // .c (caçador)
        "trickster":
            return "t";   // .t (trickster)
        "cantrip":
            return "t";   // .t (truque/cantrip)
        default:
            return "m";   // Default to mage
    endcase
endfunction

// =============================================================================
// Posture (Combat Stance) Mappings
// =============================================================================

/*
 * Get posture name from action ID
 * @param action_id Posture action ID (0-2)
 * @return struct{name: internal name, name_pt: Portuguese name for display}
 */
function GetPostureData(action_id)
    var posture := struct{};

    case (action_id)
        0:
            posture.+name := "slash";
            posture.+name_pt := "Cortante";
        1:
            posture.+name := "piercing";
            posture.+name_pt := "Perfurante";
        2:
            posture.+name := "blunt";
            posture.+name_pt := "Contusivo";
        default:
            return error;
    endcase

    return posture;
endfunction

// =============================================================================
// Special Ability Mappings
// =============================================================================

/*
 * Get special ability name from index
 * Used to identify which special a class has
 */
function GetSpecialNameByIndex(index)
    case (index)
        1: return "Ataque Extra";       // Warrior
        2: return "Evasao";             // Rogue
        3: return "Recarga Rapida";     // Engineer
        4: return "Conjuracao Rapida";  // Caster
        5: return "Velocidade Arcana";  // Arcane Archer
        default: return "";
    endcase
endfunction

// =============================================================================
// Default Icons
// =============================================================================

const DEFAULT_ABILITY_ICON := 2973;
const DEFAULT_SPELL_ICON := 0x9BAC;
const DEFAULT_COMMAND_ICON := 0x9b90;
const DEFAULT_SKILL_ICON := 0x9b90;
const DEFAULT_SPECIAL_ICON := 0x7E;

// =============================================================================
// Helper Functions
// =============================================================================

/*
 * Get action type name for debugging
 * @param action_type Action type constant
 * @return Human-readable action type name
 */
function GetActionTypeName(action_type)
    case (action_type)
        ACTION_TYPE_POSTURE: return "Posture";
        ACTION_TYPE_ABILITY: return "Ability";
        ACTION_TYPE_SPECIAL: return "Special";
        ACTION_TYPE_SPELL: return "Spell";
        ACTION_TYPE_COMMAND: return "Command";
        ACTION_TYPE_SKILL: return "Skill";
        default: return "Unknown";
    endcase
endfunction

/*
 * Validate action type is valid
 * @param action_type Action type to validate
 * @return 1 if valid, 0 if invalid
 */
function IsValidActionType(action_type)
    return (action_type >= ACTION_TYPE_POSTURE && action_type <= ACTION_TYPE_SKILL);
endfunction
