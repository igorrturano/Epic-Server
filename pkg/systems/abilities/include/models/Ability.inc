use os;

include "IAbility";
include ":IO:CFGFile";

const ABILITY_PKG := ":abilities:";

// Must be defined BEFORE class to use in class variable initializers
enum class ABILITY_ACTIVATION
	ON_MISS := "OnMiss",

	BEFORE_HIT := "BeforeHit",
	BEFORE_HIT_DEFENDER := "BeforeHitDefender",
	BEFORE_DAMAGE := "BeforeDamage",
	ON_HIT := "OnHit",
	ON_CRITICAL := "OnCritical",
	AFTER_DAMAGE := "AfterDamage",
	ON_KILL := "OnKill",
	ON_PARRY := "OnParry",

	ON_USE := "OnUse"
endenum

class Ability(IAbility)
	// === METADATA (from config) ===
	var Id := 0;
	var Name := "";
	var ClassVar := "";
	var Folk := "";
	var Description := "";
	var Requirement := "";
	var CostHP := 0;
	var CostSTAM := 0;
	var CostMANA := 0;
	var Cooldown := 0;
	var Icon := 0;

	// === BEHAVIOR (subclass defines these) ===
	var ActivationMoment := ABILITY_ACTIVATION::ON_USE;
	var EffectDuration := 0;

	function Ability(this, abilityEnum)
		var cfgFile := CFGFile("abilities", "configs/habilidades").SetElementName(abilityEnum);

		this.Id := abilityEnum;

		// === METADATA (read from config) ===
		this.Name := cfgFile.GetPropertyValue("nome")[1] ?: uninit;
		this.ClassVar := cfgFile.GetPropertyValue("classe") ?: uninit;
		this.Folk := cfgFile.GetPropertyValue("povo")[1] ?: uninit;
		this.Description := cfgFile.GetPropertyValue("descricao")[1] ?: uninit;
		this.Requirement := cfgFile.GetPropertyValue("requisito")[1] ?: uninit;
		this.CostHP := CInt(cfgFile.GetPropertyValue("hp")[1]) ?: 0;
		this.CostSTAM := CInt(cfgFile.GetPropertyValue("stam")[1]) ?: 0;
		this.CostMANA := CInt(cfgFile.GetPropertyValue("mana")[1]) ?: 0;
		this.Cooldown := CInt(cfgFile.GetPropertyValue("delay")[1]) ?: 0;
		this.Icon := cfgFile.GetPropertyValue("icon")[1] ?: uninit;

		// === BEHAVIOR (NOT read from config - subclass defines these) ===
		// ActivationMoment - defined by subclass var
		// ActivationMode - not needed (class knows if active/passive)
		// Script - replaced by class methods
		// PassiveMoment/PassiveScript - replaced by OnPassive* hooks
		// Charges - not needed
		// EffectDuration - GetDurationFor() calculates dynamically
	endfunction

	function GetId(this)
		return this.Id;
	endfunction

	function GetName(this)
		return this.Name;
	endfunction

	function GetClass(this)
		return this.ClassVar;
	endfunction

	function GetFolk(this)
		return this.Folk;
	endfunction

	function GetDescription(this)
		return this.Description;
	endfunction

	function GetActivationMoment(this)
		return this.ActivationMoment;
	endfunction

	function GetRequirement(this)
		return this.Requirement;
	endfunction

	function GetCostHP(this)
		return this.CostHP;
	endfunction

	function GetCostSTAM(this)
		return this.CostSTAM;
	endfunction

	function GetCostMANA(this)
		return this.CostMANA;
	endfunction

	function GetAbilityCooldown(this)
		return this.Cooldown;
	endfunction

	function GetDuration(this)
		return this.EffectDuration;
	endfunction

	function SetDuration(this, seconds)
		this.EffectDuration := seconds;
		return this; // Enable method chaining
	endfunction

	function GetIcon(this)
		return this.Icon;
	endfunction

	// === V3 LIFECYCLE HOOKS (override in subclass) ===

	function CanActivate(this, mobile)
		return true;
	endfunction

	function OnActivate(this, mobile)
		// Override for visual effects, etc.
	endfunction

	function GetDurationFor(this, mobile)
		return this.GetDuration();
	endfunction

	function ExecuteOn(this, attackInfo)
		return false;
	endfunction

	function ToString(this)
		return $"Ability(id={this.Id}, name={this.Name})";
	endfunction
endclass

enum class ABILITY_ID
	// Deixado com o INT para facilitar adições futuras e a leitura do código

	/*
	 * Formato: 0000
	 * x000 - x -> Caminho: 0(white): Guerreiros, 1: Ladinos, 2: Sábios
	 * 0x00 - x -> Classe dentro do caminho
	 */

	////////////////////////////
	// Caminho das Guerreiros //
	////////////////////////////

	// ARQUEIRO
	TIRO_PODEROSO := 1,
	TIRO_CARREGADO := 2,
	TIRO_EM_MOVIMENTO := 3,
	ENCONTRAR_BRECHA := 4,
	DISPARO_MULTIPLO := 5,

	// CAÇADOR
	CAMUFLAGEM := 100,
	CACADOR_EXPERIENTE := 101,
	MESTRE_DE_FERAS := 102,
	ESPREITA_SELVAGEM := 103,
	CACADOR_IMPLACAVEL := 104,

	// COMBATENTE
	HALFSWORD := 200,
	GOLPE_MORTAL := 201,
	CONTRAGOLPE_CRUZADO := 202,
	GOLPES_FURIOSOS := 203,
	SENTENCA_DE_MORTE := 204,

	// DEFENSOR
	PROVOCAR := 300,
	MANOBRA_DE_DEFESA := 301,
	MANOBRA_DESAFIADORA := 302,
	MANOBRA_DA_RESILIENCIA := 303,
	MANOBRA_DA_INVENCIBILIDADE := 304,

	// PALADINO
	CONSAGRACAO_DIVINA := 400,
	GOLPE_DIVINO := 401,
	AURA_DE_FE := 402,
	PRESENCA_DIVINA := 403,
	CAMPEAO_DIVINO := 404,

	// AKLAT'TUR (Monster Hunter)
	FURIA_DO_DESERTO := 500,
	SANGUE_POR_SANGUE := 501,
	MARCA_DA_PRESA := 502,
	RESISTENCIA_DO_DESERTO := 503,
	CARNIFICINA := 504,

	// BÁRBARO
	FURIA := 600,
	RESILIENCIA_BARBARA := 601,
	FURIA_INDOMAVEL := 602,
	FURIA_IMPLACAVEL := 603,
	DESTRUIDOR := 604,

	// MONGE
	MARTIAL_ARTIST := 700,
	DRAGON_STANCE := 701,
	CRAB_STYLE := 702,
	TIGER_STANCE := 703,
	BODY_AND_MIND := 704,

	// SAMURAI
	WANDERER_PATH := 800,
	DRAGON_FURY := 801,
	PERFECT_EXECUTION := 802,
	WIND_STRIKE := 803,
	SWORD_MASTER := 804,

	// MUTANTE
	FORMA_ADAPTAVEL := 900,
	OLEO_DO_CACADOR := 901,
	IMPULSO_METABOLICO := 902,
	TRANSMUTAR := 903,
	MUTAGENICO := 904,

	/////////////////////////
	// Caminho dos Ladinos //
	/////////////////////////

	// MALANDRO ARCANO
	SENSIBILIDADE_ARCANA := 1000,
	LADRAO_DE_MAGIAS := 1001,
	INVOCACAO_DO_CAOS := 1002,
	REAPROVEITAR_PERGAMINHO := 1003,
	FANTASMA := 1004,

	// CAÇADOR DE TESOUROS
	DESBRAVADOR_NATO := 1100,
	DISTRAIR_OPONENTES := 1101,
	SEMPRE_ALERTA := 1102,
	ESPECIALISTA_EM_INVASOES := 1103,
	BOLSA_INFINITA := 1104,

	// ASSASSINO
	ASSASSINAR := 1200,
	INSTINTO_ASSASSINO := 1201,
	ARSENAL_SOMBRIO := 1202,
	FARO_POR_SANGUE := 1203,
	MANTO_DAS_SOMBRAS := 1204,

	// SHINOBI
	NINJA_PATH := 1300,
	VITAL_STRIKE := 1301,
	SMOKE_SCREEN := 1302,
	STEALTH_RUN := 1303,
	PHANTOM_STRIKE := 1304,

	// VIVISSECCIONISTA
	MESTRE_DOS_VENENOS := 1400,
	GOLPE_SANGRENTO := 1401,
	VENENO_PURO := 1402,
	PRECISAO_CIRURGICA := 1403,
	MUMIFICACAO := 1404,

	// ESGRIMISTA
	MOVIMENTACAO_GRACIOSA := 1500,
	POSTURA_DE_DUELO := 1501,
	DESMORALIZAR := 1502,
	FINTA_ESGRIMISTA := 1503,
	MESTRE_DUELISTA := 1504,

	// LADRÃO
	BATEDOR_DE_CARTEIRA := 1600,
	TRUQUE_SUJO := 1601,
	VALENTAO := 1602,
	TRAPACEIRO_EXPERIENTE := 1603,
	FUGA_DE_EMERGENCIA := 1604,

	// BARDO (legado V1: 1700-1707)
	COMPOSITOR_TALENTOSO := 1700,
	MUSICA_ALTA := 1701,
	MUSICA_PERSISTENTE := 1702,
	MUSICA_CONTAGIANTE := 1703,
	MUSICA_DA_ALMA := 1704,
	MESTRE_DO_IMPROVISO := 1705,
	HARMONIA_PERFEITA := 1706,
	VITUPERIO := 1707,

	// BARDO V3 (1708-1712)
	VOX_ARCANA := 1708,
	RESONANCE := 1709,
	REALITY_TEAR := 1710,
	CHAOS_MASTERY := 1711,
	FINAL_SYMPHONY := 1712,

	// DANÇARINO DAS SOMBRAS
	DANCA_DAS_SOMBRAS := 1800,
	LAMINA_DA_ESCURIDAO := 1801,
	VALDRUK := 1802,
	DANCA_DA_MORTE := 1803,
	FORMA_UMBRAL := 1804,

	// DEMON HUNTER
	SIGILO_DE_PROTECAO := 1900,
	SENTENCA_PROFANA := 1901,
	MANCHA_DA_CORRUPCAO := 1902,
	ARMADILHA_DEMONIACA := 1903,
	FORMA_ABISSAL := 1904,

	////////////////////////
	// Caminho dos Sábios //
	////////////////////////

	// NECROMANTE
	SACRIFICIO_DA_CARNE := 2000,
	INFLINGIR_MEDO := 2001,
	CETRO_DA_MORTE := 2002,
	CONTROLAR_MORTOS := 2003,
	LICH := 2004,

	// DRUIDA
	ELO_NATURAL := 2100,
	CONVOCACAO_SELVAGEM := 2101,
	INSTINTO_DE_SOBREVIVENCIA := 2102,
	CORACAO_DA_MATILHA := 2103,
	CHAMADO_DA_FORMA_PRIMORDIAL := 2104,

	// MAGUS
	DETERMINACAO_ARCANA := 2200,
	SALTO_ETEREO := 2201,
	EVOCATOR := 2202,
	PRISAO_DIMENSIONAL := 2203,
	ABRIR_PORTAIS := 2204,

	// LÂMINA ARCANA
	VINCULO_ELEMENTAL := 2300,
	FLUENCIA := 2301,
	DOMINIO_VITAL := 2302,
	ARMADURA_ARCANA := 2303,
	VORTICE_PRISMATICO := 2304,

	// ARQUEIRO ARCANO
	ARCANE_BOW := 2400,
	ELEMENTAL_AFFINITY := 2401,
	CHANNELED_SHOT := 2402,
	ARCANE_MASTERY := 2403,
	PRISMATIC_SHOT := 2404,

	// GRANADEIRO
	ALQUIMISTA_EXPERIENTE := 2500,
	INFUSAO_QUIMICA := 2501,
	BOMBARDEIRO_EFICIENTE := 2502,
	ESSENCIA_EXPLOSIVA := 2503,
	INFUSAO_APRIMORADA := 2504,

	// SHAMAN / XAMÃ
	SOUL_HARVEST := 2600,
	BLOODTHIRST := 2601,
	SAND_BROTHERS := 2602,
	ELEMENTAL_ANCESTRY := 2603,
	BLOOD_ARENA := 2604,

	// CONSTRUTOR DE GOLEMS (GOLEM MAKER)
	EXPLOSIVE_FIST := 2700,
	STEAM_JUMP := 2701,
	TACTICAL_SHOULDER := 2702,
	TARGETING_VISOR := 2703,
	GOLEM_ASSEMBLY := 2704,

	// CLÉRIGO
	CONEXAO_DIVINA := 2800,
	GOLPE_RADIANTE := 2801,
	AURA_DIVINA := 2802,
	AUTORIDADE_DIVINA := 2803,
	APOTEOSE := 2804,

	// CAMINHO - Weapon Specializations (3000-3013)
	WEAPON_SPEC_DAGGER := 3000,
	WEAPON_SPEC_SWORD := 3001,
	WEAPON_SPEC_GREATSWORD := 3002,
	WEAPON_SPEC_AXE := 3003,
	WEAPON_SPEC_SPEAR := 3004,
	WEAPON_SPEC_MACE := 3005,
	WEAPON_SPEC_STAFF := 3006,
	WEAPON_SPEC_THROWING := 3007,
	WEAPON_SPEC_POLEARM := 3008,
	WEAPON_SPEC_WHIP := 3009,
	WEAPON_SPEC_BOW := 3010,
	WEAPON_SPEC_CROSSBOW := 3011,
	WEAPON_SPEC_MAGICBOW := 3012,
	WEAPON_SPEC_FIREARM := 3013,

	// CAMINHO - Fight Styles (3100-3105)
	SINGLE_WEAPON_STYLE := 3100,
	SWORD_AND_BOARD := 3101,
	TWO_HANDED_STYLE := 3102,
	DUAL_WIELD_STYLE := 3103,
	RANGED_STYLE := 3104,
	TAVERN_BRAWLER := 3105,

	// CAMINHO - Armor Specializations (3200-3202)
	LIGHT_ARMOR_SPEC := 3200,
	MEDIUM_ARMOR_SPEC := 3201,
	HEAVY_ARMOR_SPEC := 3202,

	// CAMINHO - Shield & Mounted (3300-3301)
	SHIELD_SPECIALIZATION := 3300,
	MOUNTED_COMBAT := 3301,

	// CAMINHO - Caster Core (3400-3403)
	QUIET_MIND := 3400,
	SUPPORT_CASTER := 3401,
	FOCO_ARCANO := 3402,
	SPELL_RANGE := 3403,

	// CAMINHO - Elemental Specializations (3410-3413)
	ELEMENTAL_SPEC_FIRE := 3410,
	ELEMENTAL_SPEC_COLD := 3411,
	ELEMENTAL_SPEC_ENERGY := 3412,
	ELEMENTAL_SPEC_POISON := 3413,

	// CAMINHO - School Specializations (3420-3423)
	SCHOOL_SPEC_EVOCATIUM := 3420,
	SCHOOL_SPEC_VELKYN := 3421,
	SCHOOL_SPEC_AHTALELORETH := 3422,
	SCHOOL_SPEC_KAIJIN := 3423,

	// CAMINHO - Extrator / Extractor (3560-3569)
	EXTRACTOR_VIGOR := 3560,
	GEM_MINER := 3561,
	CLEAN_CUT := 3562,
	MINE_RARITIES := 3563,
	TREASURE_FROM_TRASH := 3565,
	PACK_MULE := 3566,
	TALENTED_MINER := 3567,
	TALENTED_LUMBERJACK := 3568,
	PROSPECTOR := 3569,

	// CAMINHO - Agropecuarista / Farmer (3580-3588)
	RURAL_LIFE := 3580,
	GREEN_THUMB := 3581,
	TALENTED_FARMER := 3583,
	NO_WASTE := 3584,
	GUARANTEED_REPLANT := 3585,
	SHEPHERD := 3586,
	HARVEST_LUCK := 3588,

	// CAMINHO - Cross-Class (3600+)
	ARCHAEOLOGIST := 3608,
	EXPERT_HERBALIST := 3609,
	RECOVERY := 3612,
	ARCHITECT := 3617,
	TOOL_PRESERVATION := 3618,
	WRITE_SCROLLS := 3624,
	HEALING_HERBS := 3625,
	ACADEMIC_TRAINING := 3626,
	PRESTIDIGITATOR := 3627,
	UNARMORED_COMBAT := 3628,
endenum
