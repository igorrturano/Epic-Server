/*
 * SpellRegistry.inc - Central Spell Resolution System
 *
 * Single source of truth for resolving spell IDs to their config files.
 * Spell IDs are globally unique via non-overlapping ranges:
 *
 *   cantrips.cfg     →   1 -  99
 *   evocation.cfg    → 100 - 199
 *   necromancy.cfg   → 200 - 299
 *   illusion.cfg     → 300 - 399
 *   swordmage.cfg    → 400 - 499
 *   priest.cfg       → 500 - 599
 *   shaman.cfg       → 600 - 699
 *   druid.cfg        → 700 - 799
 *   paladin.cfg      → 800 - 899
 *   demonhunter.cfg  → 900 - 999
 *   trickster.cfg    → 1000 - 1099
 *   shinobi.cfg      → 1100 - 1199
 *   bardo.cfg        → 1200 - 1299
 */

use cfgfile;

/*
 * GetConfigFromId - Returns the config file path for a given spell ID
 *
 * @param spell_id - The unique spell ID
 * @return Config file path string, or empty string if invalid
 */
function GetConfigFromId(spell_id)
    spell_id := CInt(spell_id);

    if (spell_id < 1)
        return "";
    elseif (spell_id < 100)
        return ":spells:config/cantrips";
    elseif (spell_id < 200)
        return ":spells:config/evocation";
    elseif (spell_id < 300)
        return ":spells:config/necromancy";
    elseif (spell_id < 400)
        return ":spells:config/illusion";
    elseif (spell_id < 500)
        return ":spells:config/swordmage";
    elseif (spell_id < 600)
        return ":spells:config/priest";
    elseif (spell_id < 700)
        return ":spells:config/shaman";
    elseif (spell_id < 800)
        return ":spells:config/druid";
    elseif (spell_id < 900)
        return ":spells:config/paladin";
    elseif (spell_id < 1000)
        return ":spells:config/demonhunter";
    elseif (spell_id < 1100)
        return ":spells:config/trickster";
    elseif (spell_id < 1200)
        return ":spells:config/shinobi";
    elseif (spell_id < 1300)
        return ":spells:config/bardo";
    endif

    return "";
endfunction

/*
 * ResolveSpell - Resolves a spell ID to its config element
 *
 * @param spell_id - The unique spell ID
 * @return Config element, or error if not found
 */
function ResolveSpell(spell_id)
    var cfg_path := GetConfigFromId(spell_id);
    if (!cfg_path)
        return error;
    endif

    var cfg := ReadConfigFile(cfg_path);
    if (!cfg)
        return error;
    endif

    var elem := FindConfigElem(cfg, CInt(spell_id));
    if (!elem)
        return error;
    endif

    return elem;
endfunction

/*
 * GetSpellTypeFromId - Returns the spell type string for a given spell ID
 *
 * @param spell_id - The unique spell ID
 * @return Spell type string (e.g., "cantrip", "evocation", "priest")
 */
function GetSpellTypeFromId(spell_id)
    spell_id := CInt(spell_id);

    if (spell_id < 1)
        return "";
    elseif (spell_id < 100)
        return "cantrip";
    elseif (spell_id < 200)
        return "evocation";
    elseif (spell_id < 300)
        return "necromancy";
    elseif (spell_id < 400)
        return "illusion";
    elseif (spell_id < 500)
        return "swordmage";
    elseif (spell_id < 600)
        return "priest";
    elseif (spell_id < 700)
        return "shaman";
    elseif (spell_id < 800)
        return "druid";
    elseif (spell_id < 900)
        return "paladin";
    elseif (spell_id < 1000)
        return "demonhunter";
    elseif (spell_id < 1100)
        return "trickster";
    elseif (spell_id < 1200)
        return "shinobi";
    elseif (spell_id < 1300)
        return "bard";
    endif

    return "";
endfunction

/*
 * GetMageSchoolConfigPaths - Returns config paths for the 4 mage schools
 * (evocation, necromancy, illusion, swordmage)
 *
 * Useful for places that previously read the single spellcaster.cfg
 *
 * @return Array of mage school config paths
 */
function GetMageSchoolConfigPaths()
    return array{
        ":spells:config/evocation",
        ":spells:config/necromancy",
        ":spells:config/illusion",
        ":spells:config/swordmage"
    };
endfunction

/*
 * IsCantrip - Check if a spell ID is a cantrip
 *
 * @param spell_id - The spell ID to check
 * @return 1 if cantrip, 0 otherwise
 */
function IsCantrip(spell_id)
    spell_id := CInt(spell_id);
    return (spell_id >= 1 && spell_id < 100);
endfunction

