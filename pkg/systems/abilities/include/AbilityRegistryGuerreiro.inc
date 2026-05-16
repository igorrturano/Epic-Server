// AbilityRegistryGuerreiro.inc - Guerreiro archetype abilities (IDs 1-999)
// Arqueiro, Cacador, Combatente, Defensor, Paladino, Aklattur, Barbaro, Monge, Samurai, Mutante

include ":abilities:AbilityRegistry";

//===========================================
// ARQUEIRO (1-5)
//===========================================
include ":abilities:classes/arqueiro/PowerShot";
include ":abilities:classes/arqueiro/ChargedShot";
include ":abilities:classes/arqueiro/ShootWhileMoving";
include ":abilities:classes/arqueiro/FindOpening";
include ":abilities:classes/arqueiro/MultiShot";

//===========================================
// CACADOR (100-104)
//===========================================
include ":abilities:classes/cacador/WarBond";
include ":abilities:classes/cacador/BeastCommand";
include ":abilities:classes/cacador/BeastMaster";
include ":abilities:classes/cacador/WildStalker";
include ":abilities:classes/cacador/RelentlessHunter";

//===========================================
// COMBATENTE (200-204)
//===========================================
include ":abilities:classes/combatente/Halfsword";
include ":abilities:classes/combatente/MurderBlow";
include ":abilities:classes/combatente/CrossCut";
include ":abilities:classes/combatente/RageCut";
include ":abilities:classes/combatente/DeathSentence";

//===========================================
// DEFENSOR (300-304)
//===========================================
include ":abilities:classes/defensor/Taunt";
include ":abilities:classes/defensor/DefenseManeuver";
include ":abilities:classes/defensor/ChallengerManeuver";
include ":abilities:classes/defensor/ResilienceManeuver";
include ":abilities:classes/defensor/InvincibilityManeuver";

//===========================================
// PALADINO (400-404)
//===========================================
include ":abilities:classes/paladino/DivineConsecration";
include ":abilities:classes/paladino/DivineSmite";
include ":abilities:classes/paladino/AuraOfFaith";
include ":abilities:classes/paladino/DivinePresence";
include ":abilities:classes/paladino/DivineChampion";

//===========================================
// AKLATTUR (500-504)
//===========================================
include ":abilities:classes/aklattur/DesertFury";
include ":abilities:classes/aklattur/BloodForBlood";
include ":abilities:classes/aklattur/PreyMark";
include ":abilities:classes/aklattur/DesertResilience";
include ":abilities:classes/aklattur/Carnage";

//===========================================
// BARBARO (600-604)
//===========================================
include ":abilities:classes/barbaro/Fury";
include ":abilities:classes/barbaro/BarbarianResilience";
include ":abilities:classes/barbaro/UntamableFury";
include ":abilities:classes/barbaro/RelentlessFury";
include ":abilities:classes/barbaro/Destroyer";

//===========================================
// MONGE (700-704)
//===========================================
include ":abilities:classes/monge/MartialArtist";
include ":abilities:classes/monge/DragonStance";
include ":abilities:classes/monge/CrabStyle";
include ":abilities:classes/monge/TigerStance";
include ":abilities:classes/monge/BodyAndMind";

//===========================================
// SAMURAI (800-804)
//===========================================
include ":abilities:classes/samurai/WandererPath";
include ":abilities:classes/samurai/DragonFury";
include ":abilities:classes/samurai/PerfectExecution";
include ":abilities:classes/samurai/WindStrike";
include ":abilities:classes/samurai/SwordMaster";

//===========================================
// MUTANTE (900-904)
//===========================================
include ":abilities:classes/mutante/AdaptableForm";
include ":abilities:classes/mutante/HunterOil";
include ":abilities:classes/mutante/MetabolicBoost";
include ":abilities:classes/mutante/Transmute";
include ":abilities:classes/mutante/Mutagenic";

// === ABILITY_CLASSES Registrations ===

// Arqueiro (1-5)
ABILITY_CLASSES[ABILITY_ID::TIRO_PODEROSO] := @PowerShot;
ABILITY_CLASSES[ABILITY_ID::TIRO_CARREGADO] := @ChargedShot;
ABILITY_CLASSES[ABILITY_ID::TIRO_EM_MOVIMENTO] := @ShootWhileMoving;
ABILITY_CLASSES[ABILITY_ID::ENCONTRAR_BRECHA] := @FindOpening;
ABILITY_CLASSES[ABILITY_ID::DISPARO_MULTIPLO] := @MultiShot;

// Cacador (100-104)
ABILITY_CLASSES[ABILITY_ID::VINCULO_DE_GUERRA] := @WarBond;
ABILITY_CLASSES[ABILITY_ID::COMANDO_DE_FERAS] := @BeastCommand;
ABILITY_CLASSES[ABILITY_ID::MESTRE_DE_FERAS] := @BeastMaster;
ABILITY_CLASSES[ABILITY_ID::ESPREITA_SELVAGEM] := @WildStalker;
ABILITY_CLASSES[ABILITY_ID::CACADOR_IMPLACAVEL] := @RelentlessHunter;

// Combatente (200-204)
ABILITY_CLASSES[ABILITY_ID::HALFSWORD] := @Halfsword;
ABILITY_CLASSES[ABILITY_ID::GOLPE_MORTAL] := @MurderBlow;
ABILITY_CLASSES[ABILITY_ID::CONTRAGOLPE_CRUZADO] := @CrossCut;
ABILITY_CLASSES[ABILITY_ID::GOLPES_FURIOSOS] := @RageCut;
ABILITY_CLASSES[ABILITY_ID::SENTENCA_DE_MORTE] := @DeathSentence;

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

// Aklattur (500-504)
ABILITY_CLASSES[ABILITY_ID::FURIA_DO_DESERTO] := @DesertFury;
ABILITY_CLASSES[ABILITY_ID::SANGUE_POR_SANGUE] := @BloodForBlood;
ABILITY_CLASSES[ABILITY_ID::MARCA_DA_PRESA] := @PreyMark;
ABILITY_CLASSES[ABILITY_ID::RESISTENCIA_DO_DESERTO] := @DesertResilience;
ABILITY_CLASSES[ABILITY_ID::CARNIFICINA] := @Carnage;

// Barbaro (600-604)
ABILITY_CLASSES[ABILITY_ID::FURIA] := @Fury;
ABILITY_CLASSES[ABILITY_ID::RESILIENCIA_BARBARA] := @BarbarianResilience;
ABILITY_CLASSES[ABILITY_ID::FURIA_INDOMAVEL] := @UntamableFury;
ABILITY_CLASSES[ABILITY_ID::FURIA_IMPLACAVEL] := @RelentlessFury;
ABILITY_CLASSES[ABILITY_ID::DESTRUIDOR] := @Destroyer;

// Monge (700-704)
ABILITY_CLASSES[ABILITY_ID::MARTIAL_ARTIST] := @MartialArtist;
ABILITY_CLASSES[ABILITY_ID::DRAGON_STANCE] := @DragonStance;
ABILITY_CLASSES[ABILITY_ID::CRAB_STYLE] := @CrabStyle;
ABILITY_CLASSES[ABILITY_ID::TIGER_STANCE] := @TigerStance;
ABILITY_CLASSES[ABILITY_ID::BODY_AND_MIND] := @BodyAndMind;

// Samurai (800-804)
ABILITY_CLASSES[ABILITY_ID::WANDERER_PATH] := @WandererPath;
ABILITY_CLASSES[ABILITY_ID::DRAGON_FURY] := @DragonFury;
ABILITY_CLASSES[ABILITY_ID::PERFECT_EXECUTION] := @PerfectExecution;
ABILITY_CLASSES[ABILITY_ID::WIND_STRIKE] := @WindStrike;
ABILITY_CLASSES[ABILITY_ID::SWORD_MASTER] := @SwordMaster;

// Mutante (900-904)
ABILITY_CLASSES[ABILITY_ID::FORMA_ADAPTAVEL] := @AdaptableForm;
ABILITY_CLASSES[ABILITY_ID::OLEO_DO_CACADOR] := @HunterOil;
ABILITY_CLASSES[ABILITY_ID::IMPULSO_METABOLICO] := @MetabolicBoost;
ABILITY_CLASSES[ABILITY_ID::TRANSMUTAR] := @Transmute;
ABILITY_CLASSES[ABILITY_ID::MUTAGENICO] := @Mutagenic;
