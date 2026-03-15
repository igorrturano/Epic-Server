/*
 * Spell Executor - Single Source of Truth for Spell Execution
 * 
 * This module provides unified spell execution used by:
 * - Text commands (.t, .m, .d, .p, .c)
 * - Hotbar system (hotbarExecute.src)
 * 
 * All spell execution flows through ExecuteSpellByName() to ensure
 * consistent behavior and maintainability.
 */

use uo;
use os;
use cfgfile;

include "include/say";
include ":attributes:attributes";
include ":charactercreation:habilidades";
include ":spells:spellbooks";
include ":spells:include/tnmagery";
include ":spells:SpellRegistry";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";

// Spell type constants
const SPELL_TYPE_CANTRIP      := "cantrip";
const SPELL_TYPE_MAGE         := "mage";
const SPELL_TYPE_EVOCATION    := "evocation";
const SPELL_TYPE_NECROMANCY   := "necromancy";
const SPELL_TYPE_ILLUSION     := "illusion";
const SPELL_TYPE_SWORDMAGE   := "swordmage";
const SPELL_TYPE_PRIEST       := "priest";
const SPELL_TYPE_SHAMAN       := "shaman";
const SPELL_TYPE_DRUID        := "druid";
const SPELL_TYPE_PALADIN      := "paladin";
const SPELL_TYPE_DEMONHUNTER  := "demonhunter";
const SPELL_TYPE_TRICKSTER    := "trickster";

/*
 * ExecuteSpellByName - Main entry point for all spell execution
 * 
 * @param who - The caster (mobile reference)
 * @param spell_name - Name of the spell to cast
 * @param spell_type - Type constant (SPELL_TYPE_*)
 * @return 1 on success, 0 on failure
 */
function ExecuteSpellByName(who, spell_name, spell_type)
    // Check if already casting
    if (GetObjProperty(who, "#Casting"))
        SendSysMessageEx(who, "Voce nao pode invocar uma magia agora.", SSM_FAIL);
        return 0;
    endif
    
    // Validate access based on spell type
    if (!ValidateSpellAccess(who, spell_type))
        return 0;
    endif
    
    // Get config path for this spell type
    var cfg_path := GetSpellConfigPathForType(spell_type);
    if (!cfg_path)
        SendSysMessageEx(who, "Tipo de magia invalido.", SSM_FAIL);
        return 0;
    endif
    
    // Find spell in config
    var spell_data := FindSpellByName(cfg_path, spell_name);
    if (!spell_data)
        SendSysMessageEx(who, "A magia " + spell_name + " nao existe.", SSM_FAIL);
        return 0;
    endif
    
    // Additional validations based on spell type
    if (!ValidateSpellRequirements(who, spell_data, spell_type))
        return 0;
    endif
    
    // Build spellinfo struct
    var spellinfo := BuildSpellInfo(who, spell_data, spell_type, cfg_path);
    
    // Determine booktype for cast_spell
    var booktype := GetBooktypeForSpell(who, spell_data, spell_type);
    
    // Execute the spell
    var script := Start_Script(":spells:cast_spell", {who, spellinfo, booktype});
    if (script.errortext)
        SendSysMessage(who, "Erro ao conjurar magia: " + script.errortext);
        return 0;
    endif
    
    return 1;
endfunction

/*
 * ExecuteSpellById - Execute spell by ID (for hotbar)
 * 
 * @param who - The caster
 * @param spell_id - Numeric spell ID
 * @param spell_type - Type constant
 * @return 1 on success, 0 on failure
 */
function ExecuteSpellById(who, spell_id, spell_type)
    // Check if already casting
    if (GetObjProperty(who, "#Casting"))
        SendSysMessageEx(who, "Voce nao pode invocar uma magia agora.", SSM_FAIL);
        return 0;
    endif

    // Validate access
    if (!ValidateSpellAccess(who, spell_type))
        return 0;
    endif

    // Use SpellRegistry to resolve spell by ID directly
    var cfg_path := GetConfigFromId(spell_id);
    if (!cfg_path)
        SendSysMessageEx(who, "Tipo de magia invalido.", SSM_FAIL);
        return 0;
    endif

    var cfg := ReadConfigFile(cfg_path);
    if (!cfg)
        return 0;
    endif

    var spell_elem := FindConfigElem(cfg, spell_id);
    if (!spell_elem)
        SendSysMessageEx(who, "Magia ID " + spell_id + " nao encontrada.", SSM_FAIL);
        return 0;
    endif
    
    var spell_data := struct{
        "id" := spell_id,
        "name" := GetConfigString(spell_elem, "Name"),
        "circle" := CInt(GetConfigInt(spell_elem, "Circle")),
        "elem" := spell_elem
    };
    
    // Additional validations
    if (!ValidateSpellRequirements(who, spell_data, spell_type))
        return 0;
    endif
    
    // Build spellinfo
    var spellinfo := BuildSpellInfo(who, spell_data, spell_type, cfg_path);
    
    // Determine booktype
    var booktype := GetBooktypeForSpell(who, spell_data, spell_type);
    
    // Execute
    var script := Start_Script(":spells:cast_spell", {who, spellinfo, booktype});
    if (script.errortext)
        SendSysMessage(who, "Erro ao conjurar magia: " + script.errortext);
        return 0;
    endif
    
    return 1;
endfunction

/*
 * ValidateSpellAccess - Check if player can use this spell type
 */
function ValidateSpellAccess(who, spell_type)
    var chardata := GetObjProperty(who, "chardata");
    if (!chardata)
        SendSysMessageEx(who, "Erro ao acessar dados do personagem.", SSM_FAIL);
        return 0;
    endif
    
    var magia := chardata.magia;
    var trait := chardata.trait;
    
    case (spell_type)
        SPELL_TYPE_CANTRIP:
            if (magia != "spellcaster" && trait != "Natureza Arcana" && magia != "Arqueiro Arcano")
                SendSysMessageEx(who, "Voce nao possui acesso a truques.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_MAGE:
        SPELL_TYPE_TRICKSTER:
            // Mage spells - check grimoire or trickster ability
            // Validation done in requirements check
            return 1;
            
        SPELL_TYPE_PRIEST:
            if (magia != "Sacerdote")
                SendSysMessageEx(who, "Apenas Sacerdotes podem usar este tipo de magia.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_SHAMAN:
            if (magia != "Shaman")
                SendSysMessageEx(who, "Apenas Xamãs podem usar este tipo de magia.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_DRUID:
            if (magia != "Druid")
                SendSysMessageEx(who, "Apenas Druidas podem usar este tipo de magia.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_PALADIN:
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::CONSAGRACAO_DIVINA))
                SendSysMessageEx(who, "Apenas Paladinos podem usar este comando.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_DEMONHUNTER:
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
                SendSysMessageEx(who, "Apenas Cacadores de Demonios podem usar este comando.", SSM_FAIL);
                return 0;
            endif
    endcase
    
    return 1;
endfunction

/*
 * ValidateSpellRequirements - Check circle requirements, memorization, etc.
 */
function ValidateSpellRequirements(who, spell_data, spell_type)
    case (spell_type)
        SPELL_TYPE_PALADIN:
        SPELL_TYPE_DEMONHUNTER:
            // Check theology skill for circle access
            var theology_skill := CInt(AP_GetSkill(who, "Theology"));
            var max_circle := theology_skill / 20;
            if (max_circle < 1)
                max_circle := 1;
            endif
            
            if (spell_data.circle > max_circle)
                SendSysMessageEx(who, "Voce nao possui conhecimento teologico suficiente para esta magia.", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_PRIEST:
        SPELL_TYPE_SHAMAN:
        SPELL_TYPE_DRUID:
            // Check if spell is memorized
            var chardata := GetObjProperty(who, "chardata");
            var list_prop := GetMemorizedListProp(spell_type);
            var memorized_list := GetObjProperty(who, list_prop);
            
            if (!memorized_list || memorized_list.size() < 1)
                SendSysMessageEx(who, "Voce nao memorizou nenhum ritual.", SSM_FAIL);
                return 0;
            endif
            
            var found := 0;
            foreach spell in memorized_list
                if (spell.id == spell_data.id)
                    found := 1;
                    break;
                endif
            endforeach
            
            if (!found)
                SendSysMessageEx(who, "Voce nao memorizou o ritual " + spell_data.name + ".", SSM_FAIL);
                return 0;
            endif
            
        SPELL_TYPE_MAGE:
            // Check grimoire for non-GMs, unless they have Sensibilidade Arcana
            if (!who.cmdlevel && spell_data.circle != 0)
                if (!MobileWrapper(who).HasAbility(ABILITY_ID::SENSIBILIDADE_ARCANA))
                    var spellBookInfo := FindSpellInSpellBooks(who, spell_data.id);
                    if (!spellBookInfo.found)
                        SendSysMessageEx(who, "Voce nao possui a magia " + spell_data.name + " no seu grimorio.", SSM_FAIL);
                        return 0;
                    endif
                endif
            endif
    endcase
    
    return 1;
endfunction

/*
 * GetSpellConfigPathForType - Get config file path for spell type
 * (Named differently to avoid conflict with shared_constants.inc)
 */
function GetSpellConfigPathForType(spell_type)
    case (spell_type)
        SPELL_TYPE_CANTRIP:     return ":spells:config/cantrips";
        SPELL_TYPE_MAGE:        return ":spells:config/evocation";  // Default mage school; use GetConfigFromId() for specific spells
        SPELL_TYPE_EVOCATION:   return ":spells:config/evocation";
        SPELL_TYPE_NECROMANCY:  return ":spells:config/necromancy";
        SPELL_TYPE_ILLUSION:    return ":spells:config/illusion";
        SPELL_TYPE_SWORDMAGE:   return ":spells:config/swordmage";
        SPELL_TYPE_TRICKSTER:   return ":spells:config/trickster";
        SPELL_TYPE_PRIEST:      return ":spells:config/priest";
        SPELL_TYPE_SHAMAN:      return ":spells:config/shaman";
        SPELL_TYPE_DRUID:       return ":spells:config/druid";
        SPELL_TYPE_PALADIN:     return ":spells:config/paladin";
        SPELL_TYPE_DEMONHUNTER: return ":spells:config/demonhunter";
    endcase
    return "";
endfunction

/*
 * GetMemorizedListProp - Get property name for memorized spell list
 */
function GetMemorizedListProp(spell_type)
    case (spell_type)
        SPELL_TYPE_PRIEST:  return "priest_list";
        SPELL_TYPE_SHAMAN:  return "shaman_list";
        SPELL_TYPE_DRUID:   return "druid_list";
    endcase
    return "";
endfunction

/*
 * FindSpellByName - Search for spell by name in config
 */
function FindSpellByName(cfg_path, spell_name)
    var cfg := ReadConfigFile(cfg_path);
    if (!cfg)
        return 0;
    endif
    
    foreach spell_id in (GetConfigIntKeys(cfg))
        var elem := FindConfigElem(cfg, spell_id);
        if (Lower(GetConfigString(elem, "Name")) == Lower(spell_name))
            return struct{
                "id" := spell_id,
                "name" := GetConfigString(elem, "Name"),
                "circle" := CInt(GetConfigInt(elem, "Circle")),
                "elem" := elem
            };
        endif
        sleepms(2);
    endforeach
    
    return 0;
endfunction

/*
 * BuildSpellInfo - Create spellinfo struct for cast_spell
 */
function BuildSpellInfo(who, spell_data, spell_type, cfg_path)
    var spellinfo := struct{};
    spellinfo.+id := spell_data.id;
    
    // Set magic_path for cantrips (important for WriteCantripSpellInfo)
    if (spell_type == SPELL_TYPE_CANTRIP || IsCantrip(spell_data.id))
        spellinfo.+magic_path := "cantrips";
    endif
    
    // For some spell types, we need to populate full info
    // cast_spell.src will call WritespellInfo or WriteCantripSpellInfo as needed
    
    return spellinfo;
endfunction

/*
 * GetBooktypeForSpell - Determine booktype parameter for cast_spell
 */
function GetBooktypeForSpell(who, spell_data, spell_type)
    case (spell_type)
        SPELL_TYPE_CANTRIP:
            return 0;  // Cantrips don't need booktype
            
        SPELL_TYPE_MAGE:
        SPELL_TYPE_TRICKSTER:
            // Find in spellbook
            var spellBookInfo := FindSpellInSpellBooks(who, spell_data.id);
            if (spellBookInfo.found)
                return spellBookInfo.booktype;
            endif
            return "spellcaster";
            
        SPELL_TYPE_PRIEST:
            return "Sacerdote";
        SPELL_TYPE_SHAMAN:
            return "Shaman";
        SPELL_TYPE_DRUID:
            return "Druid";
        SPELL_TYPE_PALADIN:
            return "paladin";
        SPELL_TYPE_DEMONHUNTER:
            return "demonhunter";
    endcase
    
    return 0;
endfunction

/*
 * FindSpellInSpellBooks - Check if spell is in player's grimoire
 * (Copied from m.src for self-containment)
 */
function FindSpellInSpellBooks(who, spellId)
    foreach item in (ListEquippedItems(who))
        if (item.objtype == 0x9E80)
            var donoGrimorio := GetObjProperty(item, "serialDono");
            if (donoGrimorio == who.serial)
                var spells := GetObjProperty(item, "spells");
                foreach s in spells
                    if (s.id == spellId)
                        return dictionary{"found" -> 1, "booktype" -> GetBookType(item)};
                    endif
                endforeach
            endif
        else
            foreach item in (EnumerateItemsInContainer(who.backpack))
                if (item.objtype == 0x9E80)
                    var donoGrimorio := GetObjProperty(item, "serialDono");
                    if (donoGrimorio == who.serial)
                        var spells := GetObjProperty(item, "spells");
                        foreach s in spells
                            if (s.id == spellId)
                                return dictionary{"found" -> 1, "booktype" -> GetBookType(item)};
                            endif
                        endforeach
                    endif
                endif
            endforeach
        endif
    endforeach

    return dictionary{"found" -> 0, "booktype" -> 0};
endfunction
