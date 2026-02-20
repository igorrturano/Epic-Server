use os;

include "IAbility";
include ":IO:CFGFile";

const ABILITY_PKG := ":abilities:";

// Must be defined BEFORE class to use in class variable initializers
enum class ABILITY_ACTIVATION
	ON_MISS := "OnMiss",

	BEFORE_HIT := "BeforeHit",
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

	function GetCooldown(this)
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
		// Override in CombatAbility
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
	BUDOKA := 700,
	RYU_NO_KAMAE := 701,
	KANI_NO_SUTAIRU := 702,
	TORA_NO_KAMAE := 703,
	SHINTAI_TO_KOKORO := 704,

	// SAMURAI
	TABIBITO_NO_MICHI := 800,
	RYU_NO_IKARI := 801,
	KANPEKI_NA_JIKKO := 802,
	KAZE_NO_ICHIGEKI := 803,
	KEN_NO_TATSUJIN := 804,

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
	SALTO_DAS_SOMBRAS := 1200,
	ATAQUE_FURTIVO := 1201,
	ARSENAL_SOMBRIO := 1202,
	INSTINTO_ASSASSINO := 1203,
	FARO_POR_SANGUE := 1204,

	// ESPIÃO
	ESPIONAGEM := 1300,
	DISSIMULACAO := 1301,
	FURTIVIDADE_APRIMORADA := 1302,
	EMBOSCADA := 1303,
	MESTRE_ESPIAO := 1304,

	// VIVISSECCIONISTA
	MESTRE_DOS_VENENOS := 1400,
	GOLPE_SANGRENTO := 1401,
	VENENO_PURO := 1402,
	PRECISAO_CIRURGICA := 1403,
	MUMIFICACAO := 1404,

	// ESGRIMISTA
	MOVIMENTACAO_GRACIOSA := 1500,
	REFLEXO_ESTRATEGICO := 1501,
	DESMORALIZAR := 1502,
	FINTA_ESGRIMISTA := 1503,
	MESTRE_DUELISTA := 1504,

	// LADRÃO
	BATEDOR_DE_CARTEIRA := 1600,
	TRUQUE_SUJO := 1601,
	VALENTAO := 1602,
	TRAPACEIRO_EXPERIENTE := 1603,
	FUGA_DE_EMERGENCIA := 1604,

	// BARDO
	COMPOSITOR_TALENTOSO := 1700,
	MUSICA_ALTA := 1701,
	MUSICA_PERSISTENTE := 1702,
	MUSICA_CONTAGIANTE := 1703,
	MUSICA_DA_ALMA := 1704,
	MESTRE_DO_IMPROVISO := 1705,
	HARMONIA_PERFEITA := 1706,
	VITUPERIO := 1707,

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
	CETRO_DA_MORTE := 2003,
	CONTROLAR_MORTOS := 2004,
	LICH := 2005,

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
	GWAENEDH := 2400,
	AINALAITH := 2401,
	DISPARO_CANALIZADO := 2402,
	TIROTHAN := 2403,
	TEMPESTADE_ELEMENTAL := 2404,

	// GRANADEIRO
	ALQUIMISTA_EXPERIENTE := 2500,
	INFUSAO_QUIMICA := 2501,
	BOMBARDEIRO_EFICIENTE := 2502,
	ESSENCIA_EXPLOSIVA := 2503,
	INFUSAO_APRIMORADA := 2504,

	// SHAMAN / XAMÃ
	COLHEITA_DE_ALMAS := 2600,
	SEDE_DE_SANGUE := 2601,
	IRMAOS_DE_AREIA := 2602,
	ANCESTRALIDADE_ELEMENTAL := 2603,
	ARENA_DE_SANGUE := 2604,

	// ENGENHEIRO DE ARTILHARIA
	ARTILHEIRO_PREPARADO := 2700,
	MUNICIADOR_EXPERIENTE := 2701,
	CANHONEIRO := 2702,
	QUINQUILHARIAS := 2703,
	CONSTRUTOR_DE_GOLEMS := 2704,

	// CLÉRIGO
	CONEXAO_DIVINA := 2800,
	GOLPE_RADIANTE := 2801,
	AURA_DIVINA := 2802,
	AUTORIDADE_DIVINA := 2803,
	APOTEOSE := 2804,
endenum
