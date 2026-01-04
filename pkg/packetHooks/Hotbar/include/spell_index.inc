/*
 * Spell Index System - O(1) Spell Lookups
 *
 * Provides O(1) spell lookups by ID or name instead of O(n) loops.
 * Indexes are built once at system startup and reused.
 */

use cfgfile;

// Global spell indices (name → ID mapping)
var g_spell_name_indices := dictionary;
var g_spell_configs_loaded := 0;

/*
 * Initialize all spell configs and build name indices
 * Called once at system startup
 */
function InitializeSpellConfigs()
    if (g_spell_configs_loaded)
        return 1;
    endif

    Print("[SpellIndex] Initializing spell configs and building indices...");
    var start := ReadMillisecondClock();

    // Build index for each spell type
    BuildSpellNameIndex("spellcaster", ":spells:config/spellcaster");
    BuildSpellNameIndex("paladin", ":spells:config/paladinspells");
    BuildSpellNameIndex("druid", ":spells:config/druidspells");
    BuildSpellNameIndex("priest", ":spells:config/priestspells");
    BuildSpellNameIndex("shaman", ":spells:config/shamanspells");
    BuildSpellNameIndex("demonhunter", ":spells:config/demonhunterspells");
    BuildSpellNameIndex("trickster", ":spells:config/tricksterspells");
    BuildSpellNameIndex("cantrip", ":spells:config/cantripspells");

    g_spell_configs_loaded := 1;

    var elapsed := ReadMillisecondClock() - start;
    Print($"[SpellIndex] Initialization complete in {elapsed}ms");

    return 1;
endfunction

/*
 * Build name→ID index for a specific spell config
 *
 * @param config_name Internal name (e.g., "paladin")
 * @param config_path Full config path (e.g., ":spells:config/paladinspells")
 */
function BuildSpellNameIndex(config_name, config_path)
    var spell_cfg := ReadConfigFile(config_path);
    if (!spell_cfg)
        Print($"[SpellIndex] WARNING: Failed to load {config_path}");
        return 0;
    endif

    var index := dictionary;
    var all_ids := GetConfigIntKeys(spell_cfg);
    var count := 0;

    foreach spell_id in all_ids
        var elem := FindConfigElem(spell_cfg, spell_id);
        if (elem)
            var name := GetConfigString(elem, "Name");
            if (name)
                index[Lower(name)] := spell_id;  // Store lowercase for case-insensitive
                count := count + 1;
            endif
        endif
        sleepms(2);  // Keep for async safety during index build
    endforeach

    g_spell_name_indices[config_name] := index;
    Print($"[SpellIndex] Indexed {count} spells from {config_name}");

    return 1;
endfunction

/*
 * Get spell config element by ID (O(1) lookup)
 *
 * @param config_name Spell type (e.g., "paladin", "priest")
 * @param spell_id Spell ID
 * @return ConfigElem or error
 */
function GetSpellById(config_name, spell_id)
    var config_path := GetSpellConfigPath(config_name);
    var spell_cfg := ReadConfigFile(config_path);

    if (!spell_cfg)
        return error;
    endif

    // FindConfigElem is O(1) lookup by key
    var spell_elem := FindConfigElem(spell_cfg, spell_id);
    return spell_elem ? spell_elem : error;
endfunction

/*
 * Get spell ID by name (O(1) lookup using pre-built index)
 *
 * @param config_name Spell type (e.g., "paladin", "priest")
 * @param spell_name Spell name (case-insensitive)
 * @return Spell ID or error
 */
function GetSpellIdByName(config_name, spell_name)
    // Ensure indices are initialized
    if (!g_spell_configs_loaded)
        InitializeSpellConfigs();
    endif

    var index := g_spell_name_indices[config_name];
    if (!index)
        return error;
    endif

    // O(1) dictionary lookup (case-insensitive)
    var spell_id := index[Lower(spell_name)];
    return spell_id ? spell_id : error;
endfunction

/*
 * Get spell element by name (O(1) lookup via index)
 *
 * @param config_name Spell type
 * @param spell_name Spell name
 * @return ConfigElem or error
 */
function GetSpellByName(config_name, spell_name)
    var spell_id := GetSpellIdByName(config_name, spell_name);
    if (spell_id == error)
        return error;
    endif

    return GetSpellById(config_name, spell_id);
endfunction

// /*
//  * Helper: Get config path from config name
//  * Centralizes path mapping in one place
//  */
// function GetSpellConfigPath(config_name)
//     case (config_name)
//         "spellcaster":
//             return ":spells:config/spellcaster";
//         "paladin":
//             return ":spells:config/paladinspells";
//         "druid":
//             return ":spells:config/druidspells";
//         "priest":
//             return ":spells:config/priestspells";
//         "shaman":
//             return ":spells:config/shamanspells";
//         "demonhunter":
//             return ":spells:config/demonhunterspells";
//         "trickster":
//             return ":spells:config/tricksterspells";
//         "cantrip":
//             return ":spells:config/cantripspells";
//         default:
//             return "";
//     endcase
// endfunction

/*
 * Load complete spell data from config element
 * Centralizes spell data loading logic
 *
 * @param spell_elem ConfigElem from FindConfigElem
 * @param spell_id Spell ID
 * @return Spell data struct
 */
function LoadSpellFromConfig(spell_elem, spell_id)
    var spell := struct{};
    spell.+id := spell_id;
    spell.+name := GetConfigString(spell_elem, "Name");
    spell.+circle := CInt(GetConfigString(spell_elem, "Circle"));
    spell.+mana_cost := CInt(GetConfigString(spell_elem, "Mana"));

    // NEW: Read icon from config, fallback to default
    var icon := CInt(GetConfigString(spell_elem, "Icon"));
    spell.+iconid := icon ? icon : 0x9BAC;

    var hue := CInt(GetConfigString(spell_elem, "Hue"));
    spell.+hue := hue ? hue : 0;

    return spell;
endfunction

/*
 * Benchmark: Test lookup performance
 * Compare O(n) loop vs O(1) index lookup
 */
function BenchmarkSpellLookup(test_spell_name)
    Print("[SpellIndex] === Benchmark: Spell Lookup Performance ===");

    // Ensure index is built
    if (!g_spell_configs_loaded)
        InitializeSpellConfigs();
    endif

    var config_path := ":spells:config/priestspells";
    var priest_cfg := ReadConfigFile(config_path);

    // Test 1: O(n) loop (OLD METHOD)
    Print("");
    Print("Method 1: O(n) loop through all spells");
    var start1 := ReadMillisecondClock();

    var found_id := 0;
    var all_ids := GetConfigIntKeys(priest_cfg);
    foreach cfg_id in all_ids
        var elem := FindConfigElem(priest_cfg, cfg_id);
        if (elem && GetConfigString(elem, "Name") == test_spell_name)
            found_id := cfg_id;
            break;
        endif
        sleepms(2);
    endforeach

    var elapsed1 := ReadMillisecondClock() - start1;
    Print($"  Found: {found_id}, Time: {elapsed1}ms");

    // Test 2: O(1) index lookup (NEW METHOD)
    Print("");
    Print("Method 2: O(1) index lookup");
    var start2 := ReadMillisecondClock();

    var found_id2 := GetSpellIdByName("priest", test_spell_name);

    var elapsed2 := ReadMillisecondClock() - start2;
    Print($"  Found: {found_id2}, Time: {elapsed2}ms");

    // Analysis
    Print("");
    Print("=== Results ===");
    Print($"O(n) loop:      {elapsed1}ms");
    Print($"O(1) index:     {elapsed2}ms");
    Print($"Speedup:        {CInt((elapsed1 * 100) / elapsed2)}x");

    return 1;
endfunction
