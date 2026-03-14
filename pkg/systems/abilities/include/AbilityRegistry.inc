// AbilityRegistry.inc - ABILITY_CLASSES dictionary + CreateAbility factory.
// Each AbilityRegistry{Archetype}.inc includes this, then registers its abilities.

include ":abilities:models/Ability";

var ABILITY_CLASSES := dictionary{};

// Creates ability instance from registry
// @param abilityId ABILITY_ID enum value
// @return CombatAbility subclass instance, or error if not registered
function GetAbilityArchetype(abilityId)
    if (abilityId < 1000)
        return "Guerreiro";
    elseif (abilityId < 2000)
        return "Ladino";
    elseif (abilityId < 3000)
        return "Sabio";
    else
        return "Caminho";
    endif
endfunction

function CreateAbility(abilityId)
    if (!ABILITY_CLASSES.exists(abilityId))
        return error{"errortext" := $"Ability {abilityId} not registered in ABILITY_CLASSES"};
    endif

    var ability := ABILITY_CLASSES[abilityId].new();

    return ability;
endfunction
