// AbilityRegistryLadino.inc - Ladino archetype abilities (IDs 1000-1999)
// Malandro Arcano, Cacador de Tesouros, Assassino, Vivisseccionista,
// Esgrimista, Ladrao, Dancarino das Sombras, Cacador de Demonios

include ":abilities:AbilityRegistry";

//===========================================
// MALANDRO ARCANO (1000-1004)
//===========================================
include ":abilities:classes/malandro_arcano/ArcaneSensitivity";
include ":abilities:classes/malandro_arcano/SpellThief";
include ":abilities:classes/malandro_arcano/ChaosInvocation";
include ":abilities:classes/malandro_arcano/ReuseScroll";
include ":abilities:classes/malandro_arcano/Ghost";

//===========================================
// CACADOR DE TESOUROS (1100-1104)
//===========================================
include ":abilities:classes/cacador_de_tesouros/BornExplorer";
include ":abilities:classes/cacador_de_tesouros/DistractOpponents";
include ":abilities:classes/cacador_de_tesouros/AlwaysAlert";
include ":abilities:classes/cacador_de_tesouros/BreakInSpecialist";
include ":abilities:classes/cacador_de_tesouros/InfiniteBag";

//===========================================
// ASSASSINO (1200-1204)
//===========================================
include ":abilities:classes/assassino/Assassinate";
include ":abilities:classes/assassino/ShadowArsenal";
include ":abilities:classes/assassino/KillerInstinct";
include ":abilities:classes/assassino/ShadowCloak";
include ":abilities:classes/assassino/BloodScent";

//===========================================
// SHINOBI (1300-1304)
//===========================================
include ":abilities:classes/shinobi/NinjaPath";
include ":abilities:classes/shinobi/VitalStrike";
include ":abilities:classes/shinobi/SmokeScreen";
include ":abilities:classes/shinobi/StealthRun";
include ":abilities:classes/shinobi/PhantomStrike";

//===========================================
// VIVISSECCIONISTA (1400-1404)
//===========================================
include ":abilities:classes/vivisseccionista/PoisonMaster";
include ":abilities:classes/vivisseccionista/BloodyStrike";
include ":abilities:classes/vivisseccionista/PurePoison";
include ":abilities:classes/vivisseccionista/SurgicalPrecision";
include ":abilities:classes/vivisseccionista/Mummification";

//===========================================
// ESGRIMISTA (1500-1504)
//===========================================
include ":abilities:classes/esgrimista/GracefulMovement";
include ":abilities:classes/esgrimista/DuelStance";
include ":abilities:classes/esgrimista/Demoralize";
include ":abilities:classes/esgrimista/SwashbucklerFeint";
include ":abilities:classes/esgrimista/MasterDuelist";

//===========================================
// LADRAO (1600-1604)
//===========================================
include ":abilities:classes/ladrao/Pickpocket";
include ":abilities:classes/ladrao/DirtyTrick";
include ":abilities:classes/ladrao/Bully";
include ":abilities:classes/ladrao/ExpertTrickster";
include ":abilities:classes/ladrao/EmergencyEscape";

//===========================================
// BARDO V3 (1708-1712)
//===========================================
include ":abilities:classes/bardo/VoxArcana";
include ":abilities:classes/bardo/Resonance";
include ":abilities:classes/bardo/RealityTear";
include ":abilities:classes/bardo/ChaosMastery";
include ":abilities:classes/bardo/FinalSymphony";

//===========================================
// DANCARINO DAS SOMBRAS (1800-1804)
//===========================================
include ":abilities:classes/dancarino_das_sombras/ShadowDance";
include ":abilities:classes/dancarino_das_sombras/ShadowBlade";
include ":abilities:classes/dancarino_das_sombras/Valdruk";
include ":abilities:classes/dancarino_das_sombras/DeathDance";
include ":abilities:classes/dancarino_das_sombras/ShadowForm";

//===========================================
// CACADOR DE DEMONIOS (1900-1904)
//===========================================
include ":abilities:classes/cacador_de_demonios/ProtectionSigil";
include ":abilities:classes/cacador_de_demonios/ProfaneSentence";
include ":abilities:classes/cacador_de_demonios/CorruptionMark";
include ":abilities:classes/cacador_de_demonios/DemonicTrap";
include ":abilities:classes/cacador_de_demonios/AbyssalForm";

// === ABILITY_CLASSES Registrations ===

// Malandro Arcano (1000-1004)
ABILITY_CLASSES[ABILITY_ID::SENSIBILIDADE_ARCANA] := @ArcaneSensitivity;
ABILITY_CLASSES[ABILITY_ID::LADRAO_DE_MAGIAS] := @SpellThief;
ABILITY_CLASSES[ABILITY_ID::INVOCACAO_DO_CAOS] := @ChaosInvocation;
ABILITY_CLASSES[ABILITY_ID::REAPROVEITAR_PERGAMINHO] := @ReuseScroll;
ABILITY_CLASSES[ABILITY_ID::FANTASMA] := @Ghost;

// Cacador de Tesouros (1100-1104)
ABILITY_CLASSES[ABILITY_ID::DESBRAVADOR_NATO] := @BornExplorer;
ABILITY_CLASSES[ABILITY_ID::DISTRAIR_OPONENTES] := @DistractOpponents;
ABILITY_CLASSES[ABILITY_ID::SEMPRE_ALERTA] := @AlwaysAlert;
ABILITY_CLASSES[ABILITY_ID::ESPECIALISTA_EM_INVASOES] := @BreakInSpecialist;
ABILITY_CLASSES[ABILITY_ID::BOLSA_INFINITA] := @InfiniteBag;

// Assassino (1200-1204)
ABILITY_CLASSES[ABILITY_ID::ASSASSINAR] := @Assassinate;
ABILITY_CLASSES[ABILITY_ID::INSTINTO_ASSASSINO] := @KillerInstinct;
ABILITY_CLASSES[ABILITY_ID::ARSENAL_SOMBRIO] := @ShadowArsenal;
ABILITY_CLASSES[ABILITY_ID::FARO_POR_SANGUE] := @BloodScent;
ABILITY_CLASSES[ABILITY_ID::MANTO_DAS_SOMBRAS] := @ShadowCloak;

// Shinobi (1300-1304)
ABILITY_CLASSES[ABILITY_ID::NINJA_PATH] := @NinjaPath;
ABILITY_CLASSES[ABILITY_ID::VITAL_STRIKE] := @VitalStrike;
ABILITY_CLASSES[ABILITY_ID::SMOKE_SCREEN] := @SmokeScreen;
ABILITY_CLASSES[ABILITY_ID::STEALTH_RUN] := @StealthRun;
ABILITY_CLASSES[ABILITY_ID::PHANTOM_STRIKE] := @PhantomStrike;

// Vivisseccionista (1400-1404)
ABILITY_CLASSES[ABILITY_ID::MESTRE_DOS_VENENOS] := @PoisonMaster;
ABILITY_CLASSES[ABILITY_ID::GOLPE_SANGRENTO] := @BloodyStrike;
ABILITY_CLASSES[ABILITY_ID::VENENO_PURO] := @PurePoison;
ABILITY_CLASSES[ABILITY_ID::PRECISAO_CIRURGICA] := @SurgicalPrecision;
ABILITY_CLASSES[ABILITY_ID::MUMIFICACAO] := @Mummification;

// Esgrimista (1500-1504)
ABILITY_CLASSES[ABILITY_ID::MOVIMENTACAO_GRACIOSA] := @GracefulMovement;
ABILITY_CLASSES[ABILITY_ID::POSTURA_DE_DUELO] := @DuelStance;
ABILITY_CLASSES[ABILITY_ID::DESMORALIZAR] := @Demoralize;
ABILITY_CLASSES[ABILITY_ID::FINTA_ESGRIMISTA] := @SwashbucklerFeint;
ABILITY_CLASSES[ABILITY_ID::MESTRE_DUELISTA] := @MasterDuelist;

// Ladrao (1600-1604)
ABILITY_CLASSES[ABILITY_ID::BATEDOR_DE_CARTEIRA] := @Pickpocket;
ABILITY_CLASSES[ABILITY_ID::TRUQUE_SUJO] := @DirtyTrick;
ABILITY_CLASSES[ABILITY_ID::VALENTAO] := @Bully;
ABILITY_CLASSES[ABILITY_ID::TRAPACEIRO_EXPERIENTE] := @ExpertTrickster;
ABILITY_CLASSES[ABILITY_ID::FUGA_DE_EMERGENCIA] := @EmergencyEscape;

// Bardo V3 (1708-1712)
ABILITY_CLASSES[ABILITY_ID::VOX_ARCANA]      := @VoxArcana;
ABILITY_CLASSES[ABILITY_ID::RESONANCE]       := @Resonance;
ABILITY_CLASSES[ABILITY_ID::REALITY_TEAR]    := @RealityTear;
ABILITY_CLASSES[ABILITY_ID::CHAOS_MASTERY]   := @ChaosMastery;
ABILITY_CLASSES[ABILITY_ID::FINAL_SYMPHONY]  := @FinalSymphony;

// Dancarino das Sombras (1800-1804)
ABILITY_CLASSES[ABILITY_ID::DANCA_DAS_SOMBRAS] := @ShadowDance;
ABILITY_CLASSES[ABILITY_ID::LAMINA_DA_ESCURIDAO] := @ShadowBlade;
ABILITY_CLASSES[ABILITY_ID::VALDRUK] := @Valdruk;
ABILITY_CLASSES[ABILITY_ID::DANCA_DA_MORTE] := @DeathDance;
ABILITY_CLASSES[ABILITY_ID::FORMA_UMBRAL] := @ShadowForm;

// Cacador de Demonios (1900-1904)
ABILITY_CLASSES[ABILITY_ID::SIGILO_DE_PROTECAO] := @ProtectionSigil;
ABILITY_CLASSES[ABILITY_ID::SENTENCA_PROFANA] := @ProfaneSentence;
ABILITY_CLASSES[ABILITY_ID::MANCHA_DA_CORRUPCAO] := @CorruptionMark;
ABILITY_CLASSES[ABILITY_ID::ARMADILHA_DEMONIACA] := @DemonicTrap;
ABILITY_CLASSES[ABILITY_ID::FORMA_ABISSAL] := @AbyssalForm;

