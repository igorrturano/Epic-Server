// AbilityRegistry.inc - Maps ABILITY_ID to class references
// Add new abilities here as they are migrated to V3

include ":abilities:models/Ability";
include ":abilities:classes/_index";  // All ability class includes

// Registry: ABILITY_ID -> Class reference
// Built programmatically because dictionary literals don't evaluate enum keys
var ABILITY_CLASSES := dictionary{};

// Combatente (200-204)
ABILITY_CLASSES[ABILITY_ID::HALFSWORD] := @Halfsword;
ABILITY_CLASSES[ABILITY_ID::GOLPE_MORTAL] := @MurderBlow;
ABILITY_CLASSES[ABILITY_ID::CONTRAGOLPE_CRUZADO] := @CrossCut;
ABILITY_CLASSES[ABILITY_ID::GOLPES_FURIOSOS] := @RageCut;
ABILITY_CLASSES[ABILITY_ID::SENTENCA_DE_MORTE] := @DeathSentence;

// Cacador (100-104)
ABILITY_CLASSES[ABILITY_ID::CAMUFLAGEM] := @Camouflage;
ABILITY_CLASSES[ABILITY_ID::CACADOR_EXPERIENTE] := @ExpertHunter;
ABILITY_CLASSES[ABILITY_ID::MESTRE_DE_FERAS] := @BeastMaster;
ABILITY_CLASSES[ABILITY_ID::ESPREITA_SELVAGEM] := @WildStalker;
ABILITY_CLASSES[ABILITY_ID::CACADOR_IMPLACAVEL] := @RelentlessHunter;

// Defensor (300-304)
ABILITY_CLASSES[ABILITY_ID::PROVOCAR] := @Taunt;
ABILITY_CLASSES[ABILITY_ID::MANOBRA_DE_DEFESA] := @DefenseManeuver;
ABILITY_CLASSES[ABILITY_ID::MANOBRA_DESAFIADORA] := @ChallengerManeuver;
ABILITY_CLASSES[ABILITY_ID::MANOBRA_DA_RESILIENCIA] := @ResilienceManeuver;
ABILITY_CLASSES[ABILITY_ID::MANOBRA_DA_INVENCIBILIDADE] := @InvincibilityManeuver;

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
