use uo;
use os;
use cfgfile;
include ":wrappers:Objects/Model/MobileWrapper";

// Constantes
const VOIDRIFT_MIN_CORRUPTION := -4000;

const VOIDRIFT_SPAWN_RANGE := 4;
const VOIDRIFT_CORRUPT_RANGE := 8;
const VOIDRIFT_PORTAL_OBJTYPE := 0xe33;

// Criaturas por tier
var TIER1_CREATURES := array{ "skeleton1", "zombie", "headless" };
var TIER2_CREATURES := array{ "skeletonarcher", "evilmage", "quasit" };
var TIER3_CREATURES := array{ "evilmage2", "skeleton3", "skeletonmage" };
var TIER4_CREATURES := array{ "voidling", "voidling3", "voidspawn2" };
var TIER5_CREATURES := array{ "voidling4", "voidspawn", "voidspawn3" };

function GetGlobalEntropy()
    var global_entropy := GetGlobalProperty("GlobalEntropy");
    if (!global_entropy)
        return 0;
    endif
    return CInt(global_entropy);
endfunction

// ============================================================================
// Funções de Cálculo
// ============================================================================

function GetSpawnAmount(who)
    var mobile := MobileWrapper(who);
    var entropy := mobile.GetEntropy();
    var amount := CInt(Abs(entropy) / 1000);
    if (amount < 4)
        amount := 4;
    endif
    if (amount > 20)
        amount := 20;
    endif
    return amount;
endfunction

function GetCreatureTiers(scroll_level)
    case (scroll_level)
        1: return array{ 1, 2 };
        2: return array{ 2, 3 };
        3: return array{ 3, 4 };
        4: return array{ 4, 5 };
    endcase
    return array{ 1, 2 };
endfunction

function GetCreaturesByTier(tier)
    case (tier)
        1: return TIER1_CREATURES;
        2: return TIER2_CREATURES;
        3: return TIER3_CREATURES;
        4: return TIER4_CREATURES;
        5: return TIER5_CREATURES;
    endcase
    return TIER1_CREATURES;
endfunction

function GetRandomCreature(scroll_level)
    var tiers := GetCreatureTiers(scroll_level);
    var tier := tiers[RandomInt(2) + 1];
    var creatures := GetCreaturesByTier(tier);
    return creatures[RandomInt(len(creatures)) + 1];
endfunction

// ============================================================================
// Funções de Ritual
// ============================================================================

function LoadRitualConfig(ritual_id)
    var cfg := ReadConfigFile(":voidrift:config/rituals");
    if (!cfg)
        return error{ errortext := "Não foi possível carregar rituals.cfg" };
    endif

    var ritual := FindConfigElem(cfg, ritual_id);
    if (!ritual)
        return error{ errortext := "Ritual " + ritual_id + " não encontrado" };
    endif

    return ritual;
endfunction

function GetRitualReagents(ritual_elem)
    var reagents := array{};
    var reg_list := GetConfigStringArray(ritual_elem, "Reg");

    foreach reg_entry in reg_list
        var parts := SplitWords(reg_entry);
        if (len(parts) >= 2)
            var reagent := struct{
                objtype := CInt(parts[1]),
                amount := CInt(parts[2])
            };
            reagents.append(reagent);
        endif
    endforeach

    return reagents;
endfunction

function CheckReagents(who, reagents)
    var items_to_consume := array{};

    foreach reagent in reagents
        var needed := reagent.amount;
        var found_items := array{};

        foreach item in EnumerateItemsInContainer(who.backpack)
            if (item.objtype == reagent.objtype && needed > 0)
                var consume_amount := item.amount;
                if (consume_amount > needed)
                    consume_amount := needed;
                endif

                found_items.append(struct{
                    item := item,
                    amount := consume_amount
                });
                needed := needed - consume_amount;
            endif
        endforeach

        if (needed > 0)
            return 0;
        endif

        foreach found in found_items
            items_to_consume.append(found);
        endforeach
    endforeach

    return items_to_consume;
endfunction

function ConsumeIngredients(items_to_consume)
    foreach entry in items_to_consume
        if (entry.item.amount <= entry.amount)
            DestroyItem(entry.item);
        else
            SubtractAmount(entry.item, entry.amount);
        endif
    endforeach
endfunction

// ============================================================================
// Funções de Portal
// ============================================================================

function FindNearbyPortal(who, max_level)
    foreach item in ListItemsNearLocation(who.x, who.y, who.z, 8, who.realm)
        if (item.objtype == VOIDRIFT_PORTAL_OBJTYPE)
            var portal_level := GetObjProperty(item, "portal_level");
            if (portal_level && portal_level <= max_level)
                return item;
            endif
        endif
    endforeach
    return 0;
endfunction

function DestroyPortalCreatures(portal)
    var creatures := GetObjProperty(portal, "creatures");
    if (!creatures)
        return;
    endif

    foreach serial in creatures
        var npc := SystemFindObjectBySerial(serial);
        if (npc)
            ApplyRawDamage(npc, 9999);
        endif
        sleepms(20);
    endforeach
endfunction