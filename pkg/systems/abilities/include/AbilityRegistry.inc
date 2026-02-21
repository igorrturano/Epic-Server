// AbilityRegistry.inc - Full registry: class definitions + registrations
// For lightweight usage (no class defs), include AbilityRegistryBase.inc instead.

include ":abilities:AbilityRegistryBase";  // ABILITY_CLASSES dict + CreateAbility
include ":abilities:classes/_index";       // All ability class definitions

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

// Paladino (400-404)
ABILITY_CLASSES[ABILITY_ID::CONSAGRACAO_DIVINA] := @DivineConsecration;
ABILITY_CLASSES[ABILITY_ID::GOLPE_DIVINO] := @DivineSmite;
ABILITY_CLASSES[ABILITY_ID::AURA_DE_FE] := @AuraOfFaith;
ABILITY_CLASSES[ABILITY_ID::PRESENCA_DIVINA] := @DivinePresence;
ABILITY_CLASSES[ABILITY_ID::CAMPEAO_DIVINO] := @DivineChampion;

