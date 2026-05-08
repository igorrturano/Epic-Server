// scrollRecipes.inc
// =================================================================
// Single source of truth for all recipe scroll and spell scroll
// assignment logic.
//
// HOW THE SYSTEM WORKS:
//
// Each scroll item objtype encodes both the skill domain and the
// level (1-N). When a scroll is created via CreateItemInBackpack,
// the POL engine calls the CreateScript defined in itemdesc.cfg.
// That script is a one-liner that calls the matching Assign*
// function below, which selects a random recipe filtered to the
// correct skill type and level.
//
// Calling code only needs the right objtype:
//   CreateItemInBackpack(who, GetScrollObjtype("MW", 3), 1)
//
// The CreateScript fires automatically. No caller needs to set
// recipe properties manually. If you change selection logic,
// change this file only.
//
// Skill type codes and their level ranges:
//   "MW"      MetalWork         levels 1-5
//   "WW"      WoodWork          levels 1-5
//   "DW"      DomesticWork      levels 1-5
//   "alchemy" Alchemy           levels 1-5
//   "dye"     Dye               levels 1-5
//   "spell"   Spell (spellcaster type only) levels 1-5
//   "void"    Void (chaos ritual)           levels 1-4
//   "purge"   Purge (purification ritual)   levels 1-4
//
// Void and purge scrolls do NOT need an Assign* function —
// their properties are already set via CProp in itemdesc.cfg.
// GetScrollObjtype is all you need for those two types.
// =================================================================

use cfgfile;
use util;
use uo;

include ":spells:SpellRegistry";

// ---- Objtype tables (array index matches level number) ----

var SCROLL_MW      := array{ 0xff990, 0xfe995, 0xfe996, 0xfe997, 0xfe998 };
var SCROLL_WW      := array{ 0xfe999, 0xfe99a, 0xfe99b, 0xfe99c, 0xfe99d };
var SCROLL_DW      := array{ 0xfe99e, 0xfe99f, 0xfea00, 0xfea01, 0xfea02 };
var SCROLL_ALCHEMY := array{ 0xef1f,  0xef2c,  0xef2d,  0xef2e,  0xef2f  };
var SCROLL_DYE     := array{ 0xef2b,  0xef3c,  0xef3d,  0xef3e,  0xef3f  };
var SCROLL_SPELL   := array{ 0x2060,  0x2064,  0x2066,  0x2063,  0x2065  };
var SCROLL_VOID    := array{ 0xe35,   0xe36,   0xe37,   0xe3f           };
var SCROLL_PURGE   := array{ 0xe38,   0xe39,   0xe3a,   0xe32           };

// =================================================================
// PUBLIC API
// =================================================================

// Returns the objtype for a given skill type and level.
// Returns 0 for invalid inputs.
function GetScrollObjtype(skillType, level)
    if (level < 1 || level > 5) return 0; endif
    case (skillType)
        "MW":      return SCROLL_MW[level];
        "WW":      return SCROLL_WW[level];
        "DW":      return SCROLL_DW[level];
        "alchemy": return SCROLL_ALCHEMY[level];
        "dye":     return SCROLL_DYE[level];
    endcase
    return 0;
endfunction

// Returns an objtype for a magic scroll (spell, void, or purge), chosen randomly.
// Levels 1-4 draw from all three pools; level 5 is spell-only (void/purge cap at 4).
function GetMagicScrollObjtype(level)
    if (level < 1 || level > 5) return 0; endif
    if (level == 5)
        return SCROLL_SPELL[level];
    endif
    var roll := RandomInt(10);
    if (roll <= 7) return SCROLL_SPELL[level]; endif  // 80%
    if (roll == 8) return SCROLL_VOID[level];  endif  // 10%
    return SCROLL_PURGE[level];                       // 10%
endfunction

// Assigns a random craft recipe to item. skillType: "MW", "WW", or "DW".
// Level is derived from item.objtype.
function AssignCraftRecipe(item, skillType)
    var level := _GetCraftLevel(item.objtype);
    if (!level) return; endif
    var key := _SelectCraftRecipe(level, skillType);
    if (key)
        SetObjProperty(item, "recipe", key);
    endif
endfunction

// Assigns a random alchemy recipe. Level derived from item.objtype.
function AssignAlchemyRecipe(item)
    var level := _GetAlchemyLevel(item.objtype);
    if (!level) return; endif

    var cfg := ReadConfigFile(":alchemy:config/recipes");
    if (cfg.errortext) return; endif

    var menuKeys := _BuildMenuKeys("alchemy");
    var pool := array{};
    foreach key in (GetConfigIntKeys(cfg))
        var elem := FindConfigElem(cfg, key);
        if (!elem || GetConfigInt(elem, "level") != level) continue; endif
        if (GetConfigString(elem, "Name") in menuKeys)
            pool.append(key);
        endif
    endforeach

    if (pool.size() > 0)
        var selected := pool[RandomInt(pool.size()) + 1];
        // alchemyrecipeclick.src reads "AlchemyRecipe" as a plain integer key,
        // then wraps it into struct{ "recipe" := key } before calling WriteAlchInfo.
        SetObjProperty(item, "AlchemyRecipe", selected);
    endif
endfunction

// Assigns a random dye recipe. Level derived from item.objtype.
function AssignDyeRecipe(item)
    var level := _GetDyeLevel(item.objtype);
    if (!level) return; endif

    var cfg := ReadConfigFile(":alchemy:config/recipestintas");
    if (cfg.errortext) return; endif

    var pool := array{};
    foreach key in (GetConfigIntKeys(cfg))
        var elem := FindConfigElem(cfg, key);
        if (elem && GetConfigInt(elem, "level") == level)
            pool.append(key);
        endif
    endforeach

    if (pool.size() > 0)
        var selected := pool[RandomInt(pool.size()) + 1];
        // dyerecipeclick.src reads "recipe" as a struct and passes it directly
        // to WriteDyeInfo, which does FindConfigElem(cfg, recipe.recipe).
        var r := struct;
        r.+recipe := selected;
        SetObjProperty(item, "recipe", r);
    endif
endfunction

// Assigns a random spellcaster spell to a spell scroll.
// Level derived from the spells itemdesc config (Circle field).
// Updates item.graphic based on spell school.
// Skips if spellinfo already set (allows grimorio.src to override with a specific spell).
function AssignSpellRecipe(item)
    if (GetObjProperty(item, "spellinfo") != error) return; endif

    var scrollcfg := ReadConfigFile(":spells:itemdesc");
    var lvl := scrollcfg[item.objtype].Circle;
    var spelltype := scrollcfg[item.objtype].SpellType;
    if (!lvl) lvl := 1; endif

    var pool := array{};
    foreach cfg_path in (GetMageSchoolConfigPaths())
        var spellcfg := ReadConfigFile(cfg_path);
        if (!spellcfg) continue; endif
        foreach key in (GetConfigIntKeys(spellcfg))
            var elem := FindConfigElem(spellcfg, key);
            if (GetConfigString(elem, "Type") == "spellcaster" && GetConfigInt(elem, "Circle") == lvl)
                pool.append(key);
            endif
        endforeach
    endforeach

    if (pool.size() == 0) return; endif

    var spellinfo := struct;
    spellinfo.+id := pool[RandomInt(pool.size()) + 1];
    spellinfo.+magic_path := spelltype;

    if (spellinfo.magic_path == "spellcaster")
        var elem := ResolveSpell(spellinfo.id);
        if (elem)
            spellinfo.+School := GetConfigString(elem, "School");
        endif
        case (spellinfo.School)
            "Kaijin":       item.graphic := 0x0E39;
            "Evocatium":    item.graphic := 0x0E35;
            "Velkyn":       item.graphic := 0x0E3A;
            "Ahtaleloreth": item.graphic := 0x0E38;
            "Aradalore":    item.graphic := 0x0E37;
        endcase
    endif

    SetObjProperty(item, "spellinfo", spellinfo);
endfunction

// Returns a random spellcaster spell ID for the given circle level.
// Used by pesquisar.src to pre-select a spell when starting research,
// so the same spell is guaranteed at completion 7 days later.
function SelectRandomSpell(power)
    var pool := array{};
    foreach cfg_path in (GetMageSchoolConfigPaths())
        var spellcfg := ReadConfigFile(cfg_path);
        if (!spellcfg) continue; endif
        foreach key in (GetConfigIntKeys(spellcfg))
            var elem := FindConfigElem(spellcfg, key);
            if (GetConfigString(elem, "Type") == "spellcaster" && GetConfigInt(elem, "Circle") == power)
                pool.append(GetConfigInt(elem, "SpellId"));
            endif
        endforeach
    endforeach
    if (pool.size() > 0)
        return pool[RandomInt(pool.size()) + 1];
    endif
    return 0;
endfunction

// =================================================================
// PRIVATE HELPERS
// =================================================================

function _GetCraftLevel(objtype)
    for i := 1 to 5
        if (objtype == SCROLL_MW[i] || objtype == SCROLL_WW[i] || objtype == SCROLL_DW[i])
            return i;
        endif
    endfor
    return 0;
endfunction

function _GetAlchemyLevel(objtype)
    for i := 1 to 5
        if (objtype == SCROLL_ALCHEMY[i]) return i; endif
    endfor
    return 0;
endfunction

function _GetDyeLevel(objtype)
    for i := 1 to 5
        if (objtype == SCROLL_DYE[i]) return i; endif
    endfor
    return 0;
endfunction

function _SelectCraftRecipe(level, skillType)
    var cfg := ReadConfigFile(":recipes:recipes");
    if (cfg.errortext) return 0; endif

    var menuKeys := _BuildMenuKeys(skillType);
    var pool := array{};
    foreach key in (GetConfigStringKeys(cfg))
        var elem := FindConfigElem(cfg, key);
        if (!elem || GetConfigInt(elem, "level") != level) continue; endif
        if (key in menuKeys)
            pool.append(key);
        endif
    endforeach

    if (pool.size() > 0)
        return pool[RandomInt(pool.size()) + 1];
    endif
    return 0;
endfunction

function _BuildMenuKeys(skillType)
    var paths := array{};
    case (skillType)
        "MW":
            paths.append(":metalworking:blacksmith/config/SmithMenu");
            paths.append(":metalworking:tinkering/config/TinkerMenu");
        "WW":
            paths.append(":woodworking:carpentry/config/CarpMenu");
            paths.append(":woodworking:bowcraft/config/FletchMenu");
        "DW":
            paths.append(":DomesticWork:TailorMenu");
        "alchemy":
            paths.append(":alchemy:AlchMenu");
    endcase

    var keys := array{};
    foreach path in paths
        var menuCfg := ReadConfigFile(path);
        if (menuCfg.errortext) continue; endif
        foreach groupKey in (GetConfigStringKeys(menuCfg))
            if (groupKey == "Index") continue; endif
            var elem := FindConfigElem(menuCfg, groupKey);
            if (!elem) continue; endif
            foreach itemStr in (GetConfigStringArray(elem, "Item"))
                var words := SplitWords(itemStr);
                if (words.size() >= 2 && words[1] == "Recipe")
                    keys.append(words[words.size()]);
                endif
            endforeach
        endforeach
    endforeach
    return keys;
endfunction
