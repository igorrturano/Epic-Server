// AbilityRegistryBase.inc - Lightweight ability registry interface
// Defines ABILITY_CLASSES dictionary and CreateAbility function.
// Include AbilityRegistry.inc (full) to populate with class references.

include ":abilities:models/Ability";

var ABILITY_CLASSES := dictionary{};

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
