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

// Spell type constants — every value here matches a real config file. There is
// no aggregate "mage" type: a spellcaster's repertoire is the union of the
// types returned by GetMageSpellTypes() below.
const SPELL_TYPE_CANTRIP      := "cantrip";
const SPELL_TYPE_EVOCATION    := "evocation";
const SPELL_TYPE_NECROMANCY   := "necromancy";
const SPELL_TYPE_ILLUSION     := "illusion";
const SPELL_TYPE_SWORDMAGE    := "swordmage";
const SPELL_TYPE_PRIEST       := "priest";
const SPELL_TYPE_SHAMAN       := "shaman";
const SPELL_TYPE_DRUID        := "druid";
const SPELL_TYPE_PALADIN      := "paladin";
const SPELL_TYPE_DEMONHUNTER  := "demonhunter";
const SPELL_TYPE_TRICKSTER    := "trickster";
const SPELL_TYPE_SHINOBI      := "shinobi";
const SPELL_TYPE_BARD         := "bard";

// Spellcaster repertoire — the schools a mage character can ever cast from.
// There is no single "mage" spell type: any check for "is this a mage spell?"
// goes through GetMageSpellTypes() below. Used by .m to resolve a spell name
// to its real school before reaching the executor.
function GetMageSpellTypes()
    return array{
        SPELL_TYPE_CANTRIP,
        SPELL_TYPE_EVOCATION,
        SPELL_TYPE_NECROMANCY,
        SPELL_TYPE_ILLUSION,
        SPELL_TYPE_SWORDMAGE
    };
endfunction

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
    
    // Every caller passes a concrete spell type — mage callers (.m) resolve the
    // school via ResolveMageSchoolForSpell before getting here.
    var cfg_path := GetSpellConfigPathForType(spell_type);
    if (!cfg_path)
        SendSysMessageEx(who, "Tipo de magia invalido.", SSM_FAIL);
        return 0;
    endif
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

    // Phase 5: Bard cantico entropy award
    if (spell_type == SPELL_TYPE_BARD)
        _AwardBardCanticoEntropy(who, spell_data.circle);
    endif

    return 1;
endfunction

// Inline bard entropy (avoids cross-package include of musicianship)
function _AwardBardCanticoEntropy(who, circle)
    var amt;
    case (circle)
        0: 1: amt := 150;   break;
        2:    amt := 350;   break;
        3:    amt := 750;   break;
        4:    amt := -2000; break; // C4 canticos generate negative entropy
        default: return; break;
    endcase
    var cur := CInt(GetObjProperty(who, "entropy"));
    if (!cur) cur := 0; endif
    SetObjProperty(who, "entropy", cur + amt);
    var global_ent := CInt(GetGlobalProperty("GlobalEntropy"));
    if (!global_ent) global_ent := 0; endif
    SetGlobalProperty("GlobalEntropy", global_ent + amt);
    if (amt > 0)
        SendSysMessageEx(who, $"Voce ganhou {amt} pontos de entropia", SSM_REQUEST);
    else
        SendSysMessageEx(who, $"Voce perdeu {Abs(amt)} pontos de entropia", SSM_FAIL);
    endif
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
            if (magia != "spellcaster" && trait != "Natureza Arcana" && magia != "arcane_archer")
                SendSysMessageEx(who, "Voce nao possui acesso a truques.", SSM_FAIL);
                return 0;
            endif
            
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
            // Signature ability is the single source of truth (granted at
            // creation + backfilled on logon by GrantSignatureAbility).
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::CONSAGRACAO_DIVINA))
                SendSysMessageEx(who, "Apenas Paladinos podem usar este comando.", SSM_FAIL);
                return 0;
            endif

        SPELL_TYPE_DEMONHUNTER:
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::SIGILO_DE_PROTECAO))
                SendSysMessageEx(who, "Apenas Cacadores de Demonios podem usar este comando.", SSM_FAIL);
                return 0;
            endif

        SPELL_TYPE_SHINOBI:
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::NINJA_PATH))
                SendSysMessageEx(who, "Apenas Shinobis podem usar este comando.", SSM_FAIL);
                return 0;
            endif

        SPELL_TYPE_BARD:
            if (!MobileWrapper(who).HasAbility(ABILITY_ID::VOX_ARCANA))
                SendSysMessageEx(who, "Apenas Bardos podem usar canticos.", SSM_FAIL);
                return 0;
            endif
            if (GetObjProperty(who, "#bard_silenced"))
                SendSysMessageEx(who, "Voce esta silenciado e nao pode cantar.", SSM_FAIL);
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
        SPELL_TYPE_SHINOBI:
            // Check sneak skill against spell difficulty
            var sneak_skill := CInt(EffectiveSkill(who, "Stealth"));
            if (sneak_skill < spell_data.difficulty)
                SendSysMessageEx(who, "Voce nao possui furtividade suficiente para esta tecnica.", SSM_FAIL);
                return 0;
            endif

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
            
        SPELL_TYPE_BARD:
            var mobile_bard := MobileWrapper(who);
            // Circle access gates
            var max_cantico_circle := 1;
            if (mobile_bard.HasAbility(ABILITY_ID::CHAOS_MASTERY))
                max_cantico_circle := 4;
            elseif (mobile_bard.HasAbility(ABILITY_ID::REALITY_TEAR))
                max_cantico_circle := 3;
            elseif (mobile_bard.HasAbility(ABILITY_ID::RESONANCE))
                max_cantico_circle := 2;
            endif
            if (spell_data.circle > max_cantico_circle)
                SendSysMessageEx(who, "Voce nao possui acesso a canticos deste circulo.", SSM_FAIL);
                return 0;
            endif
            // Learned check for C1+ (GMs bypass)
            if (spell_data.circle > 0 && !who.cmdlevel)
                var cantico_list := GetObjProperty(who, "cantico_list");
                var cantico_found := 0;
                if (cantico_list)
                    foreach c in cantico_list
                        if (c == spell_data.id)
                            cantico_found := 1;
                            break;
                        endif
                    endforeach
                endif
                if (!cantico_found)
                    SendSysMessageEx(who, "Voce nao aprendeu o cantico '" + spell_data.name + "'.", SSM_FAIL);
                    return 0;
                endif
            endif

        SPELL_TYPE_EVOCATION:
        SPELL_TYPE_NECROMANCY:
        SPELL_TYPE_ILLUSION:
        SPELL_TYPE_SWORDMAGE:
            // Every spellcaster school requires the spell to be in the
            // grimoire. Trickster (SENSIBILIDADE_ARCANA) and GMs bypass. The
            // ability is the single source of truth — granted at creation and
            // backfilled on logon by GrantSignatureAbility.
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
        SPELL_TYPE_SHINOBI:     return ":spells:config/shinobi";
        SPELL_TYPE_BARD:        return ":spells:config/bardo";
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
 * ResolveMageSchoolForSpell - Find which mage school owns a spell name.
 * Returns the SPELL_TYPE_* constant of the school containing the spell, or 0
 * if the name is not in any mage repertoire. Cantrips are excluded — they go
 * through .t, not .m.
 */
function ResolveMageSchoolForSpell(spell_name)
    foreach spell_type in GetMageSpellTypes()
        if (spell_type == SPELL_TYPE_CANTRIP)
            continue;
        endif
        var cfg_path := GetSpellConfigPathForType(spell_type);
        if (cfg_path && FindSpellByName(cfg_path, spell_name))
            return spell_type;
        endif
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
            
        SPELL_TYPE_EVOCATION:
        SPELL_TYPE_NECROMANCY:
        SPELL_TYPE_ILLUSION:
        SPELL_TYPE_SWORDMAGE:
        SPELL_TYPE_TRICKSTER:
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
        SPELL_TYPE_SHINOBI:
            return "shinobi";
        SPELL_TYPE_BARD:
            return "bardo";
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
