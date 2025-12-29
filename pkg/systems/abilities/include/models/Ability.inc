use os;

include "IAbility";

include ":diceroll:model/DiceRoll";
include ":IO:CFGFile";

const ABILITY_PKG := ":abilities:scripts/";

class Ability(IAbility)
	var _id; // ABILITY_ID enum
	var _name := "";
	var _description := "";
	var _activationMoment := ABILITY_ACTIVATION::ON_USE;
	var _costHP := 0;
	var _costSTAM := 0;
	var _costMANA := 0;
	var _cooldown := 0.0; // em segundos
	var _effectDuration := 0.0; // em segundos
	var _range := 1; // em tiles
	var _damage; // DamageDice class
	var _script := ""; // nome do script que será executado ao usar a habilidade

	function Ability(this, abilityEnum)
		var cfgFile := CFGFile("abilities", "configs/habilidades").SetElementName(abilityEnum);
		
		this._id := abilityEnum;
		this._name := cfgFile.GetPropertyValue("name")[1];
		this._class := cfgFile.GetPropertyValue("classe");
		this._level := cfgFile.GetPropertyValue("nivel");
		this._folk := cfgFile.GetPropertyValue("povo")[1];
		this._description := cfgFile.GetPropertyValue("description")[1];
		this._activation := cfgFile.GetPropertyValue("uso")[1];
		this._activationMoment := cfgFile.GetPropertyValue("activationMoment")[1];
		this._requirement := cfgFile.GetPropertyValue("requisito")[1];

		this._cooldown := cfgFile.GetPropertyValue("cooldown")[1];
		this._costHP := cfgFile.GetPropertyValue("hp")[1];
		this._costSTAM := cfgFile.GetPropertyValue("stam")[1];
		this._costMANA := cfgFile.GetPropertyValue("mana")[1];
		this._effectDuration := cfgFile.GetPropertyValue("effectDuration")[1];
		this._range := cfgFile.GetPropertyValue("range")[1];
		this._damage := cfgFile.GetPropertyValue("damage")[1];
		this._script := cfgFile.GetPropertyValue("script")[1];
	endfunction

	function GetId(this)
		return this._id;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetDescription(this)
		return this._description;
	endfunction

	function GetActivationMoment(this)
		return this._activationMoment;
	endfunction

	function GetCostHP(this)
		return this._costHP;
	endfunction

	function GetCostSTAM(this)
		return this._costSTAM;
	endfunction

	function GetCostMANA(this)
		return this._costMANA;
	endfunction

	function GetCooldown(this)
		return this._cooldown;
	endfunction

	function GetEffectDuration(this)
		return this._effectDuration;
	endfunction

	function GetRange(this)
		return this._range;
	endfunction

	function GetDamage(this)
		return this._damage;
	endfunction

	function GetScript(this)
		return this._script;
	endfunction

	function ExecuteOn(this, targetInfo)
		Start_Script($"{ABILITY_PKG}{this._script}", targetInfo);
	endfunction
endclass

enum class ABILITY_ID
	// Deixado com o INT para facilitar adições futuras e a leitura do código

	/*
	 * Formato: 0000
	 * x000 - x -> Caminho: 0(white): Armas, 1: Ladinos, 2: Sábios
	 * 0x00 - x -> Classe dentro do caminho
	 */

	///////////////////////
	// Caminho das Armas //
	///////////////////////

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
	CONTRA_EMPUNHADURA := 200,
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

	// AKLAT'TUR
	NAZG_KHALUG := 500,
	BURZ := 501,
	GHASHBURZ := 502,
	SHAR_KRITH := 503,
	NAZG_GUL := 504,

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
	APOTEOSE := 2804
endenum

enum class ABILITY_ACTIVATION
	BEFORE_DAMAGE,
	AFTER_DAMAGE,
	BEFORE_HIT,
	ON_MISS,
	ON_HIT,
	ON_CRITICAL,
	ON_KILL,
	ON_USE
endenum

enum class ABILITY_ACTIVATION_MODE
	PASSIVE := "Passiva",
	ACTIVE := "Ativa"
endenum

enum class ABILITY_DURATION
	_DEFAULT // Placeholder
endenum
