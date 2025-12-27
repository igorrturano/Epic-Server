use os;

include "IAbility";

include ":diceroll:model/DiceRoll";
include ":IO:CFGFile";

class CombatAbility(IAbility)
	var _id; // ABILITY_ID enum
	var _name := "";
	var _description := "";
	var _activationMoment := ACTIVATION_MOMENT::ON_USE;
	var _costHP := 0;
	var _costSTAM := 0;
	var _costMANA := 0;
	var _cooldown := 0.0; // em segundos
	var _effectDuration := 0.0; // em segundos
	var _range := 1; // em tiles
	var _damage; // DamageDice class
	var _script := ""; // nome do script que será executado ao usar a habilidade

	function CombatAbility(this, abilityEnum)
		var cfgFile := CFGFile("charactercreation", "config/habilidades").SetElementName(abilityEnum);
		
		this._id := abilityEnum;
		this._name := cfgFile.GetPropertyValue("name");
		this._description := cfgFile.GetPropertyValue("description");
		this._activationMoment := cfgFile.GetPropertyValue("activationMoment");
		this._cooldown := cfgFile.GetPropertyValue("cooldown");
		this._costHP := cfgFile.GetPropertyValue("hp");
		this._costSTAM := cfgFile.GetPropertyValue("stam");
		this._costMANA := cfgFile.GetPropertyValue("mana");
		this._effectDuration := cfgFile.GetPropertyValue("effectDuration");
		this._range := cfgFile.GetPropertyValue("range");
		this._damage := cfgFile.GetPropertyValue("damage");
		this._script := cfgFile.GetPropertyValue("script");
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
		Start_Script($":abilities:include/habilidades/{this._script}", targetInfo);
	endfunction
endclass

enum class ABILITY_ID
	AINALAITH := "Ainalaith",
	ARMADURA_ARCANA := "Armadura Arcana",
	ATAQUE_FURTIVO := "Ataque Furtivo",
	ATAQUE_INFALIVEL := "Ataque Infalível",
	AURA_DE_FE := "Aura de Fe",
	BRIGADOR_DE_TAVERNA := "Brigador de Taverna",
	BUDOKA := "Budōka",
	CHARGE := "Charge",
	CONHECIMENTO_SOBRE_O_INIMIGO := "Conhecimento sobre o Inimigo",
	DANCA_DAS_SOMBRAS := "Danca das Sombras",
	DEATH_DANCE := "Danca da Morte",
	DESTRUIDOR := "Destruidor",
	DESVIAR_DISPAROS := "Desviar Disparos",
	DESVIAR_PROJETIL := "Desviar Projetil",
	DISPARO_MISTICO := "Disparo Místico",
	DOMINIO_VITAL := "Domínio Vital",
	ESPECIALISTA_ESCUDOS := "Especialista Escudos",
	ESQUIVA_INSTINTIVA := "Esquiva Instintiva",
	EVASAO := "Evasão",
	EXOESQUELETO := "Exoesqueleto",
	FURIA := "Fúria",
	FURIA_INDOMAVEL := "Fúria Indomável",
	ILUDIR_O_FRACO := "Iludir o Fraco",
	INSPIRAR_DEFESAS := "Inspirar Defesas",
	INSTINTO_ASSASSINO := "Instinto Assassino",
	KANI_NO_SUTAIRU := "Kani no Sutairu",
	KEN_NO_TATSUJIN := "Ken no Tatsujin",
	LADRAO_DA_SORTE := "Ladrao da Sorte",
	LOBO_SOLITARIO := "Lobo Solitário",
	MAESTRIA_DE_DEVASTACAO := "Maestria de Devastacao",
	MANCHA_DA_CORRUPCAO := "Mancha da Corrupcao",
	MAO_LIVRE := "Mão Livre",
	MOVIMENTACAO_GRACIOSA := "Movimentacao Graciosa",
	MUMIFICACAO := "Mumificacao",
	OPORTUNIDADE_CRITICA := "Oportunidade Critica",
	POSTURA_DEFENSIVA := "Postura Defensiva",
	POSTURA_OFENSIVA := "Postura Ofensiva",
	PRESERVAR_ORGAOS := "Preservar Orgaos",
	REBUKE := "Rebuke",
	RECUPERACAO_SALVADORA := "Recuperação Salvadora",
	RECUPERAR_FLECHAS := "Recuperar Flechas",
	REFLEXO_ESTRATEGICO := "Reflexo Estrategico",
	RESILIENCIA_BARBARA := "Resiliência Bárbara",
	SEMPRE_ALERTA := "Sempre Alerta",
	SENSO_DA_NATUREZA := "Senso da Natureza",
	TABIBITO_NO_MICHI := "Tabibito no Michi",
	TATICAS_ARCANAS := "Taticas Arcanas",
	TIROTHAN := "Tirothan",
	TRESPASSAR := "Trespassar",
	TRESPASSAR_CRITICO := "Trespassar Critico",
	VENENO_PURO := "Veneno Puro",
	VINCULO_ELEMENTAL := "Vínculo Elemental",
	ESPIAO := "Espiao",
	MESTRE_ESPIAO := "Mestre Espiao",
	HALFSWORD := "Contra-Empunhadura",

	// TODO: Conferir grafia
	MARTIR := "Mártir",
	MANTO_ROTHEN := "Manto Rothen",
	VINGANCA := "Vingança",
	KANINO_SUTAIRU := "Kanino Sutairu",
	BACKSTAB := "Backstab",
	BARREIRA_INDESTRUTIVEL := "Barreira Indestrutível",
	CONTRA_ATAQUE := "Contra-Ataque",
	FEROCIDADE_ENSANGUENTADA := "Ferocidade Ensanguentada",
	MESTRE_DUELISTA := "Mestre Duelista",
	ARROW_RECOVERY := "Recuperar Flechas",
	DIVINE_AUTHORITY := "Divine Authority"
endenum

enum class ACTIVATION_MOMENT
	BEFORE_DAMAGE,
	AFTER_DAMAGE,
	ON_MISS,
	ON_HIT,
	ON_CRITICAL,
	ON_KILL,
	ON_USE
endenum
