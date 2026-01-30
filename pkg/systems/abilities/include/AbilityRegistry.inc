// AbilityRegistry.inc - Maps ABILITY_ID to class references
// Add new abilities here as they are migrated to V3

include ":abilities:models/Ability";

// Import ability classes
include ":abilities:classes/combatente/Halfswording";

// Registry: ABILITY_ID -> Class reference
// Built programmatically because dictionary literals don't evaluate enum keys
var ABILITY_CLASSES := dictionary{};
ABILITY_CLASSES[ABILITY_ID::HALFSWORD] := @Halfswording;

// Creates ability instance from registry
// @param abilityId ABILITY_ID enum value
// @return CombatAbility subclass instance, or error if not registered
function CreateAbility(abilityId)
    if (!ABILITY_CLASSES.exists(abilityId))
        return error{"errortext" := $"Ability {abilityId} not registered in ABILITY_CLASSES"};
    endif

    var ability := ABILITY_CLASSES[abilityId].new();

    return ability;
endfunction
