// AbilityRegistrySabio.inc - Sabio archetype abilities (IDs 2000-2999)
// Necromante, Druida, Magus, Lamina Arcana, Arqueiro Arcano, Granadeiro,
// Shaman, Engenheiro, Cleric

include ":abilities:AbilityRegistry";

//===========================================
// NECROMANTE (2000-2004)
//===========================================
include ":abilities:classes/necromante/FleshSacrifice";
include ":abilities:classes/necromante/InflictFear";
include ":abilities:classes/necromante/DeathScepter";
include ":abilities:classes/necromante/ControlUndead";
include ":abilities:classes/necromante/Lich";

//===========================================
// DRUIDA (2100-2104)
//===========================================
include ":abilities:classes/druida/NaturalBond";
include ":abilities:classes/druida/WildSummoning";
include ":abilities:classes/druida/SurvivalInstinct";
include ":abilities:classes/druida/PackHeart";
include ":abilities:classes/druida/PrimordialFormCall";

//===========================================
// MAGUS (2200-2204)
//===========================================
include ":abilities:classes/magus/ArcaneDetermination";
include ":abilities:classes/magus/EtherealJump";
include ":abilities:classes/magus/Evocator";
include ":abilities:classes/magus/DimensionalPrison";
include ":abilities:classes/magus/OpenPortals";

//===========================================
// LAMINA ARCANA (2300-2304)
//===========================================
include ":abilities:classes/lamina_arcana/ElementalBond";
include ":abilities:classes/lamina_arcana/Fluency";
include ":abilities:classes/lamina_arcana/VitalDomain";
include ":abilities:classes/lamina_arcana/ArcaneArmor";
include ":abilities:classes/lamina_arcana/PrismaticVortex";

//===========================================
// ARQUEIRO ARCANO (2400-2404)
//===========================================
include ":abilities:classes/arqueiro_arcano/ElementalBow";
include ":abilities:classes/arqueiro_arcano/ElementalAffinity";
include ":abilities:classes/arqueiro_arcano/ChanneledShot";
include ":abilities:classes/arqueiro_arcano/ArcaneMastery";
include ":abilities:classes/arqueiro_arcano/PrismaticShot";

//===========================================
// GRANADEIRO (2500-2504)
//===========================================
include ":abilities:classes/granadeiro/TriggerEngineering";
include ":abilities:classes/granadeiro/PressureZone";
include ":abilities:classes/granadeiro/Minefield";
include ":abilities:classes/granadeiro/GrenadeTurret";
include ":abilities:classes/granadeiro/MasterProtocol";

//===========================================
// SHAMAN / XAMÃ (2600-2604)
//===========================================
include ":abilities:classes/shaman/SoulHarvest";
include ":abilities:classes/shaman/Bloodthirst";
include ":abilities:classes/shaman/SandBrothers";
include ":abilities:classes/shaman/ElementalAncestry";
include ":abilities:classes/shaman/BloodArena";

//===========================================
// CONSTRUTOR DE GOLEMS / ENGENHEIRO (2700-2704)
//===========================================
include ":abilities:classes/engenheiro/ExplosiveFist";
include ":abilities:classes/engenheiro/TargetingVisor";
include ":abilities:classes/engenheiro/RocketJump";
include ":abilities:classes/engenheiro/TacticalShoulder";
include ":abilities:classes/engenheiro/GolemAssembly";

//===========================================
// CLERIC / CLERIGO (2800-2804)
//===========================================
include ":abilities:classes/cleric/DivineConnection";
include ":abilities:classes/cleric/RadiantSmite";
include ":abilities:classes/cleric/DivineAura";
include ":abilities:classes/cleric/DivineAuthority";
include ":abilities:classes/cleric/Apotheosis";

// === ABILITY_CLASSES Registrations ===

// Necromante (2000-2004)
ABILITY_CLASSES[ABILITY_ID::SACRIFICIO_DA_CARNE] := @FleshSacrifice;
ABILITY_CLASSES[ABILITY_ID::INFLINGIR_MEDO] := @InflictFear;
ABILITY_CLASSES[ABILITY_ID::CETRO_DA_MORTE] := @DeathScepter;
ABILITY_CLASSES[ABILITY_ID::CONTROLAR_MORTOS] := @ControlUndead;
ABILITY_CLASSES[ABILITY_ID::LICH] := @Lich;

// Druida (2100-2104)
ABILITY_CLASSES[ABILITY_ID::ELO_NATURAL] := @NaturalBond;
ABILITY_CLASSES[ABILITY_ID::CONVOCACAO_SELVAGEM] := @WildSummoning;
ABILITY_CLASSES[ABILITY_ID::INSTINTO_DE_SOBREVIVENCIA] := @SurvivalInstinct;
ABILITY_CLASSES[ABILITY_ID::CORACAO_DA_MATILHA] := @PackHeart;
ABILITY_CLASSES[ABILITY_ID::CHAMADO_DA_FORMA_PRIMORDIAL] := @PrimordialFormCall;

// Magus (2200-2204)
ABILITY_CLASSES[ABILITY_ID::DETERMINACAO_ARCANA] := @ArcaneDetermination;
ABILITY_CLASSES[ABILITY_ID::SALTO_ETEREO] := @EtherealJump;
ABILITY_CLASSES[ABILITY_ID::EVOCATOR] := @Evocator;
ABILITY_CLASSES[ABILITY_ID::PRISAO_DIMENSIONAL] := @DimensionalPrison;
ABILITY_CLASSES[ABILITY_ID::ABRIR_PORTAIS] := @OpenPortals;

// Lamina Arcana (2300-2304)
ABILITY_CLASSES[ABILITY_ID::VINCULO_ELEMENTAL] := @ElementalBond;
ABILITY_CLASSES[ABILITY_ID::FLUENCIA] := @Fluency;
ABILITY_CLASSES[ABILITY_ID::DOMINIO_VITAL] := @VitalDomain;
ABILITY_CLASSES[ABILITY_ID::ARMADURA_ARCANA] := @ArcaneArmor;
ABILITY_CLASSES[ABILITY_ID::VORTICE_PRISMATICO] := @PrismaticVortex;

// Arqueiro Arcano (2400-2404)
ABILITY_CLASSES[ABILITY_ID::ARCANE_BOW] := @ElementalBow;
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_AFFINITY] := @ElementalAffinity;
ABILITY_CLASSES[ABILITY_ID::CHANNELED_SHOT] := @ChanneledShot;
ABILITY_CLASSES[ABILITY_ID::ARCANE_MASTERY] := @ArcaneMastery;
ABILITY_CLASSES[ABILITY_ID::PRISMATIC_SHOT] := @PrismaticShot;

// Granadeiro (2500-2504)
ABILITY_CLASSES[ABILITY_ID::ENGENHARIA_DE_GATILHOS] := @TriggerEngineering;
ABILITY_CLASSES[ABILITY_ID::ZONA_DE_PRESSAO] := @PressureZone;
ABILITY_CLASSES[ABILITY_ID::CAMPO_MINADO] := @Minefield;
ABILITY_CLASSES[ABILITY_ID::TORRETA_GRANADEIRA] := @GrenadeTurret;
ABILITY_CLASSES[ABILITY_ID::PROTOCOLO_MESTRE] := @MasterProtocol;

// Shaman / Xamã (2600-2604)
ABILITY_CLASSES[ABILITY_ID::SOUL_HARVEST] := @SoulHarvest;
ABILITY_CLASSES[ABILITY_ID::BLOODTHIRST] := @Bloodthirst;
ABILITY_CLASSES[ABILITY_ID::SAND_BROTHERS] := @SandBrothers;
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_ANCESTRY] := @ElementalAncestry;
ABILITY_CLASSES[ABILITY_ID::BLOOD_ARENA] := @BloodArena;

// Construtor de Golems / Engenheiro (2700-2704)
ABILITY_CLASSES[ABILITY_ID::EXPLOSIVE_FIST] := @ExplosiveFist;
ABILITY_CLASSES[ABILITY_ID::TARGETING_VISOR] := @TargetingVisor;
ABILITY_CLASSES[ABILITY_ID::STEAM_JUMP] := @RocketJump;
ABILITY_CLASSES[ABILITY_ID::TACTICAL_SHOULDER] := @TacticalShoulder;
ABILITY_CLASSES[ABILITY_ID::GOLEM_ASSEMBLY] := @GolemAssembly;

// Cleric / Clerigo (2800-2804)
ABILITY_CLASSES[ABILITY_ID::CONEXAO_DIVINA] := @DivineConnection;
ABILITY_CLASSES[ABILITY_ID::GOLPE_RADIANTE] := @RadiantSmite;
ABILITY_CLASSES[ABILITY_ID::AURA_DIVINA] := @DivineAura;
ABILITY_CLASSES[ABILITY_ID::AUTORIDADE_DIVINA] := @DivineAuthority;
ABILITY_CLASSES[ABILITY_ID::APOTEOSE] := @Apotheosis;

