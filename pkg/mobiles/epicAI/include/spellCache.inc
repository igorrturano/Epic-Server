/*
 * Spell Cache System
 *
 * Purpose: Eliminate repeated config file reads during spell selection.
 * GetSpellCircle() was iterating 4 config files with sleepms(2) per entry,
 * called O(n^2) times inside SortSpellsByPriority's bubble sort.
 *
 * All spell data is loaded ONCE (lazily on first use) into dictionaries
 * for O(1) lookups. Config files are: evocation, necromancy, illusion,
 * swordmage, shaman, priest.
 */

var _spell_cache_built := 0;

// spell name (lowercase) -> circle number
var _spell_circle_cache := dictionary;

// spell name (lowercase) -> full spell info struct
var _spell_info_cache := dictionary;

// school + "_" + tier -> array of spell names available at that tier
var _spells_by_school_tier := dictionary;

// "shaman_" + tier -> array of spell names
var _shaman_spells_by_tier := dictionary;

// "priest_" + tier -> array of spell names
var _priest_spells_by_tier := dictionary;

// "druid_" + tier -> array of spell names
var _druid_spells_by_tier := dictionary;

function InitSpellCache()
    if (_spell_cache_built)
        return;
    endif

    // Mage school configs
    var mage_configs := array{
        ":spells:config/evocation",
        ":spells:config/necromancy",
        ":spells:config/illusion",
        ":spells:config/swordmage"
    };

    foreach cfg_path in mage_configs
        var cfg := ReadConfigFile(cfg_path);
        if (!cfg)
            continue;
        endif
        foreach spellId in (GetConfigIntKeys(cfg))
            var elem := FindConfigElem(cfg, spellId);
            var name := GetConfigString(elem, "Name");
            if (!name)
                continue;
            endif

            var name_lower := Lower(name);
            var circle := CInt(GetConfigInt(elem, "Circle"));
            var school := GetConfigString(elem, "School");

            _spell_circle_cache[name_lower] := circle;

            _spell_info_cache[name_lower] := dictionary{
                "id" -> CInt(spellId),
                "circle" -> circle,
                "name" -> name,
                "Difficulty" -> GetConfigInt(elem, "Difficulty"),
                "School" -> school,
                "Target" -> GetConfigString(elem, "Target"),
                "script" -> GetConfigString(elem, "SpellScript"),
                "Cycles" -> GetConfigInt(elem, "Cycles"),
                "PowerWords" -> GetConfigString(elem, "PowerWords"),
                "mana" -> GetConfigInt(elem, "Mana")
            };

            // Index by school + tier (spells available at tier N = all spells with circle <= N)
            if (school && circle > 0)
                for tier := circle to 5
                    var key := school + "_" + CStr(tier);
                    if (!_spells_by_school_tier.exists(key))
                        _spells_by_school_tier[key] := array{};
                    endif
                    _spells_by_school_tier[key].append(name);
                endfor
            endif
        endforeach
    endforeach

    // Shaman config
    var shaman_cfg := ReadConfigFile(":spells:config/shaman");
    if (shaman_cfg)
        foreach spellId in (GetConfigIntKeys(shaman_cfg))
            var elem := FindConfigElem(shaman_cfg, spellId);
            var name := GetConfigString(elem, "Name");
            if (!name)
                continue;
            endif
            var name_lower := Lower(name);
            var circle := CInt(GetConfigInt(elem, "Circle"));

            _spell_circle_cache[name_lower] := circle;

            _spell_info_cache[name_lower] := dictionary{
                "id" -> CInt(spellId),
                "circle" -> circle,
                "name" -> name,
                "Difficulty" -> GetConfigInt(elem, "Difficulty"),
                "School" -> GetConfigString(elem, "School"),
                "Target" -> GetConfigString(elem, "Target"),
                "script" -> GetConfigString(elem, "SpellScript"),
                "Cycles" -> GetConfigInt(elem, "Cycles"),
                "PowerWords" -> GetConfigString(elem, "PowerWords"),
                "mana" -> GetConfigInt(elem, "Mana")
            };

            // Include circle 0 cantrips (Dardo Igneo, Raio de Gelo) — available at all tiers
            var start_tier := circle;
            if (start_tier < 1) start_tier := 1; endif
            for tier := start_tier to 5
                var key := "shaman_" + CStr(tier);
                if (!_shaman_spells_by_tier.exists(key))
                    _shaman_spells_by_tier[key] := array{};
                endif
                _shaman_spells_by_tier[key].append(name);
            endfor
        endforeach
    endif

    // Priest config
    var priest_cfg := ReadConfigFile(":spells:config/priest");
    if (priest_cfg)
        foreach spellId in (GetConfigIntKeys(priest_cfg))
            var elem := FindConfigElem(priest_cfg, spellId);
            var name := GetConfigString(elem, "Name");
            if (!name)
                continue;
            endif
            var name_lower := Lower(name);
            var circle := CInt(GetConfigInt(elem, "Circle"));

            _spell_circle_cache[name_lower] := circle;

            _spell_info_cache[name_lower] := dictionary{
                "id" -> CInt(spellId),
                "circle" -> circle,
                "name" -> name,
                "Difficulty" -> GetConfigInt(elem, "Difficulty"),
                "School" -> GetConfigString(elem, "School"),
                "Target" -> GetConfigString(elem, "Target"),
                "script" -> GetConfigString(elem, "SpellScript"),
                "Cycles" -> GetConfigInt(elem, "Cycles"),
                "PowerWords" -> GetConfigString(elem, "PowerWords"),
                "mana" -> GetConfigInt(elem, "Mana")
            };

            for tier := circle to 5
                var key := "priest_" + CStr(tier);
                if (!_priest_spells_by_tier.exists(key))
                    _priest_spells_by_tier[key] := array{};
                endif
                _priest_spells_by_tier[key].append(name);
            endfor
        endforeach
    endif

    // Druid config
    var druid_cfg := ReadConfigFile(":spells:config/druid");
    if (druid_cfg)
        foreach spellId in (GetConfigIntKeys(druid_cfg))
            var elem := FindConfigElem(druid_cfg, spellId);
            var name := GetConfigString(elem, "Name");
            if (!name)
                continue;
            endif
            var name_lower := Lower(name);
            var circle := CInt(GetConfigInt(elem, "Circle"));

            _spell_circle_cache[name_lower] := circle;

            _spell_info_cache[name_lower] := dictionary{
                "id" -> CInt(spellId),
                "circle" -> circle,
                "name" -> name,
                "Difficulty" -> GetConfigInt(elem, "Difficulty"),
                "School" -> GetConfigString(elem, "School"),
                "Target" -> GetConfigString(elem, "Target"),
                "script" -> GetConfigString(elem, "SpellScript"),
                "Cycles" -> GetConfigInt(elem, "Cycles"),
                "PowerWords" -> GetConfigString(elem, "PowerWords"),
                "mana" -> GetConfigInt(elem, "Mana")
            };

            // Include circle 0 cantrips (Fogo Fatuo, Queimadura por frio, etc.) — available at all tiers
            var start_tier := circle;
            if (start_tier < 1) start_tier := 1; endif
            for tier := start_tier to 5
                var key := "druid_" + CStr(tier);
                if (!_druid_spells_by_tier.exists(key))
                    _druid_spells_by_tier[key] := array{};
                endif
                _druid_spells_by_tier[key].append(name);
            endfor
        endforeach
    endif

    _spell_cache_built := 1;
endfunction

// O(1) lookup - replaces GetSpellCircle which iterated 4 configs with sleepms(2)
function GetSpellCircleCached(spellname)
    var circle := _spell_circle_cache[Lower(spellname)];
    if (circle)
        return circle;
    endif
    return 1;
endfunction

// O(1) lookup - replaces FindMageSpellInConfigs / FindSpellInConfig
function FindSpellInfoCached(spellname)
    return _spell_info_cache[Lower(spellname)];
endfunction

// O(1) lookup - replaces getAvailableSpells which iterated 4 configs with sleepms(2)
function GetAvailableSpellsCached(school, tier)
    if (!tier)
        tier := 1;
    endif
    var key := school + "_" + CStr(tier);
    var spells := _spells_by_school_tier[key];
    if (!spells)
        return array{"Misseis Magicos"};
    endif
    // Always include Misseis Magicos as fallback
    if (!("Misseis Magicos" in spells))
        spells.append("Misseis Magicos");
    endif
    return spells;
endfunction

// O(1) lookup - replaces GetAvailableShamanSpells which iterated config with sleepms(2)
function GetAvailableShamanSpellsCached(tier)
    if (!tier)
        tier := 1;
    endif
    var key := "shaman_" + CStr(tier);
    var spells := _shaman_spells_by_tier[key];
    if (!spells)
        return array{"Dardo Igneo"};
    endif
    return spells;
endfunction

// O(1) lookup - replaces GetAvailablePriestSpells which iterated config with sleepms(2)
function GetAvailablePriestSpellsCached(tier)
    if (!tier)
        tier := 1;
    endif
    var key := "priest_" + CStr(tier);
    var spells := _priest_spells_by_tier[key];
    if (!spells)
        return array{"Cura Leve"};
    endif
    if (!("Cura Leve" in spells))
        spells.append("Cura Leve");
    endif
    return spells;
endfunction

function GetAvailableDruidSpellsCached(tier)
    if (!tier)
        tier := 1;
    endif
    var key := "druid_" + CStr(tier);
    var spells := _druid_spells_by_tier[key];
    if (!spells)
        return array{"Fogo Fatuo"};
    endif
    if (!("Fogo Fatuo" in spells))
        spells.append("Fogo Fatuo");
    endif
    return spells;
endfunction
