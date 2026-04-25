// AbilityRegistryCaminho.inc - Caminho abilities (IDs 3000+)
// Weapon Specializations, Fight Styles, Armor Specializations,
// Shield & Mounted, Caster Specializations,
// Extrator, Agropecuarista, Cross-Class

include ":abilities:AbilityRegistry";

//===========================================
// CAMINHO - Weapon Specializations (3000-3013)
//===========================================
include ":abilities:classes/caminho/WeaponSpecialization";

//===========================================
// CAMINHO - Fight Styles (3100-3105)
//===========================================
include ":abilities:classes/caminho/SingleWeaponStyle";
include ":abilities:classes/caminho/SwordAndBoard";
include ":abilities:classes/caminho/TwoHandedStyle";
include ":abilities:classes/caminho/DualWieldStyle";
include ":abilities:classes/caminho/RangedStyle";
include ":abilities:classes/caminho/TavernBrawler";

//===========================================
// CAMINHO - Armor Specializations (3200-3202)
//===========================================
include ":abilities:classes/caminho/LightArmorSpec";
include ":abilities:classes/caminho/MediumArmorSpec";
include ":abilities:classes/caminho/HeavyArmorSpec";

//===========================================
// CAMINHO - Shield & Mounted (3300-3301)
//===========================================
include ":abilities:classes/caminho/ShieldSpecialization";
include ":abilities:classes/caminho/MountedCombat";

//===========================================
// CAMINHO - Caster (3400-3423)
//===========================================
include ":abilities:classes/caminho/QuietMind";
include ":abilities:classes/caminho/SupportCaster";
include ":abilities:classes/caminho/FocoArcano";
include ":abilities:classes/caminho/SpellRange";
include ":abilities:classes/caminho/ElementalSpecialization";
include ":abilities:classes/caminho/SchoolSpecialization";

//===========================================
// CAMINHO - Extrator (3560-3569)
//===========================================
include ":abilities:classes/caminho/ExtractorAbilities";

//===========================================
// CAMINHO - Agropecuarista (3580-3588)
//===========================================
include ":abilities:classes/caminho/FarmerAbilities";

//===========================================
// CAMINHO - Cross-Class (3600+)
//===========================================
include ":abilities:classes/caminho/CrossClassAbilities";
include ":abilities:classes/caminho/UnarmoredCombat";

// === ABILITY_CLASSES Registrations ===

// Caminho - Weapon Specializations (3000-3013)
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_DAGGER] := @WeaponSpecDagger;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_SWORD] := @WeaponSpecSword;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_GREATSWORD] := @WeaponSpecGreatsword;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_AXE] := @WeaponSpecAxe;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_SPEAR] := @WeaponSpecSpear;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_MACE] := @WeaponSpecMace;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_STAFF] := @WeaponSpecStaff;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_THROWING] := @WeaponSpecThrowing;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_POLEARM] := @WeaponSpecPolearm;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_WHIP] := @WeaponSpecWhip;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_BOW] := @WeaponSpecBow;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_CROSSBOW] := @WeaponSpecCrossbow;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_MAGICBOW] := @WeaponSpecMagicBow;
ABILITY_CLASSES[ABILITY_ID::WEAPON_SPEC_FIREARM] := @WeaponSpecFirearm;

// Caminho - Fight Styles (3100-3105)
ABILITY_CLASSES[ABILITY_ID::SINGLE_WEAPON_STYLE] := @SingleWeaponStyle;
ABILITY_CLASSES[ABILITY_ID::SWORD_AND_BOARD] := @SwordAndBoard;
ABILITY_CLASSES[ABILITY_ID::TWO_HANDED_STYLE] := @TwoHandedStyle;
ABILITY_CLASSES[ABILITY_ID::DUAL_WIELD_STYLE] := @DualWieldStyle;
ABILITY_CLASSES[ABILITY_ID::RANGED_STYLE] := @RangedStyle;
ABILITY_CLASSES[ABILITY_ID::TAVERN_BRAWLER] := @TavernBrawler;

// Caminho - Armor Specializations (3200-3202)
ABILITY_CLASSES[ABILITY_ID::LIGHT_ARMOR_SPEC] := @LightArmorSpec;
ABILITY_CLASSES[ABILITY_ID::MEDIUM_ARMOR_SPEC] := @MediumArmorSpec;
ABILITY_CLASSES[ABILITY_ID::HEAVY_ARMOR_SPEC] := @HeavyArmorSpec;

// Caminho - Shield & Mounted (3300-3301)
ABILITY_CLASSES[ABILITY_ID::SHIELD_SPECIALIZATION] := @ShieldSpecialization;
ABILITY_CLASSES[ABILITY_ID::MOUNTED_COMBAT] := @MountedCombat;

// Caminho - Caster Core (3400-3403)
ABILITY_CLASSES[ABILITY_ID::QUIET_MIND] := @QuietMind;
ABILITY_CLASSES[ABILITY_ID::SUPPORT_CASTER] := @SupportCaster;
ABILITY_CLASSES[ABILITY_ID::FOCO_ARCANO] := @FocoArcano;
ABILITY_CLASSES[ABILITY_ID::SPELL_RANGE] := @SpellRange;

// Caminho - Elemental Specializations (3410-3413)
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_SPEC_FIRE] := @ElementalSpecFire;
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_SPEC_COLD] := @ElementalSpecCold;
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_SPEC_ENERGY] := @ElementalSpecEnergy;
ABILITY_CLASSES[ABILITY_ID::ELEMENTAL_SPEC_POISON] := @ElementalSpecPoison;

// Caminho - School Specializations (3420-3423)
ABILITY_CLASSES[ABILITY_ID::SCHOOL_SPEC_EVOCATIUM] := @SchoolSpecEvocatium;
ABILITY_CLASSES[ABILITY_ID::SCHOOL_SPEC_VELKYN] := @SchoolSpecVelkyn;
ABILITY_CLASSES[ABILITY_ID::SCHOOL_SPEC_AHTALELORETH] := @SchoolSpecAhtaleloreth;
ABILITY_CLASSES[ABILITY_ID::SCHOOL_SPEC_KAIJIN] := @SchoolSpecKaijin;

// Caminho - Extrator (3560-3569)
ABILITY_CLASSES[ABILITY_ID::EXTRACTOR_VIGOR] := @ExtractorVigor;
ABILITY_CLASSES[ABILITY_ID::GEM_MINER] := @GemMiner;
ABILITY_CLASSES[ABILITY_ID::MINE_RARITIES] := @MineRarities;
ABILITY_CLASSES[ABILITY_ID::TREASURE_FROM_TRASH] := @TreasureFromTrash;
ABILITY_CLASSES[ABILITY_ID::PACK_MULE] := @PackMule;
ABILITY_CLASSES[ABILITY_ID::TALENTED_MINER] := @TalentedMiner;
ABILITY_CLASSES[ABILITY_ID::TALENTED_LUMBERJACK] := @TalentedLumberjack;
ABILITY_CLASSES[ABILITY_ID::PROSPECTOR] := @Prospector;

// Caminho - Agropecuarista (3580-3588)
ABILITY_CLASSES[ABILITY_ID::RURAL_LIFE] := @RuralLife;
ABILITY_CLASSES[ABILITY_ID::GREEN_THUMB] := @GreenThumb;
ABILITY_CLASSES[ABILITY_ID::TALENTED_FARMER] := @TalentedFarmer;
ABILITY_CLASSES[ABILITY_ID::NO_WASTE] := @NoWaste;
ABILITY_CLASSES[ABILITY_ID::GUARANTEED_REPLANT] := @GuaranteedReplant;
ABILITY_CLASSES[ABILITY_ID::SHEPHERD] := @Shepherd;
ABILITY_CLASSES[ABILITY_ID::HARVEST_LUCK] := @HarvestLuck;

// Caminho - Cross-Class (3600+)
ABILITY_CLASSES[ABILITY_ID::ARCHAEOLOGIST] := @Archaeologist;
ABILITY_CLASSES[ABILITY_ID::EXPERT_HERBALIST] := @ExpertHerbalist;
ABILITY_CLASSES[ABILITY_ID::RECOVERY] := @Recovery;
ABILITY_CLASSES[ABILITY_ID::ARCHITECT] := @Architect;
ABILITY_CLASSES[ABILITY_ID::TOOL_PRESERVATION] := @ToolPreservation;
ABILITY_CLASSES[ABILITY_ID::WRITE_SCROLLS] := @WriteScrolls;
ABILITY_CLASSES[ABILITY_ID::HEALING_HERBS] := @HealingHerbs;
ABILITY_CLASSES[ABILITY_ID::ACADEMIC_TRAINING] := @AcademicTraining;
ABILITY_CLASSES[ABILITY_ID::PRESTIDIGITATOR] := @Prestidigitator;
ABILITY_CLASSES[ABILITY_ID::UNARMORED_COMBAT] := @UnarmoredCombat;
