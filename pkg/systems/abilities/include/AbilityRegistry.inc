// AbilityRegistry.inc - Maps ABILITY_ID to class references
// Add new abilities here as they are migrated to V3

include ":abilities:models/Ability";
include ":abilities:classes/_index";  // All ability class includes

// Registry: ABILITY_ID -> Class reference
// Built programmatically because dictionary literals don't evaluate enum keys
var ABILITY_CLASSES := dictionary{};

// Combatente (200-204)
ABILITY_CLASSES[ABILITY_ID::HALFSWORD] := @Halfswording;
ABILITY_CLASSES[ABILITY_ID::GOLPE_MORTAL] := @GolpeMortal;
ABILITY_CLASSES[ABILITY_ID::CONTRAGOLPE_CRUZADO] := @ContragolpeCruzado;
ABILITY_CLASSES[ABILITY_ID::GOLPES_FURIOSOS] := @GolpesFuriosos;
ABILITY_CLASSES[ABILITY_ID::SENTENCA_DE_MORTE] := @SentencaDeMorte;

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
