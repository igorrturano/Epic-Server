include "ICombatAbility";

include ":diceroll:model/DiceRoll";
include ":IO:CFGFile";

class CombatAbility(ICombatAbility)
	var _name := "";
	var _description := "";
	var _moment := COMBATABILITY_MOMENT::ON_HIT;
	var _costHP := 0;
	var _costSTAM := 0;
	var _costMANA := 0;
	var _cooldown := 0.0; // em segundos
	var _effectDuration := 0.0; // em segundos
	var _range := 1; // em tiles
	var _damage; // DamageDice class

	function CombatAbility(this, abilityName)
		var cfgFile := CFGFile("charactercreation", "config/habilidades").SetElementName(abilityName);
		
		this._name := cfgFile.SetKeyName("name").GetKeyValue();
		this._description := cfgFile.SetKeyName("description").GetKeyValue();
		this._moment := cfgFile.SetKeyName("moment").GetKeyValue();
		this._cooldown := cfgFile.SetKeyName("cooldown").GetKeyValue();
		this._costHP := cfgFile.SetKeyName("hp").GetKeyValue();
		this._costSTAM := cfgFile.SetKeyName("stam").GetKeyValue();
		this._costMANA := cfgFile.SetKeyName("mana").GetKeyValue();
		this._effectDuration := cfgFile.SetKeyName("effectDuration").GetKeyValue();
		this._range := cfgFile.SetKeyName("range").GetKeyValue();
		this._damage := cfgFile.SetKeyName("damage").GetKeyValue();
	endfunction

	function ExecuteOn(this, byRef executor, byRef target)
		// TODO: Implementar a lógica de execução da habilidade aqui
		return true;
	endfunction

	// Private methods
endclass

enum class COMBAT_ABILITY
	POSTURA_OFENSIVA := "Postura Ofensiva",
	POSTURA_DEFENSIVA := "Postura Defensiva",
	PRESERVAR_ORGAOS := "Preservar Orgaos",
	AURA_DE_FE := "Aura de Fe",
	ILUDIR_O_FRACO := "Iludir o Fraco",
	ESQUIVA_INSTINTIVA := "Esquiva Instintiva",
	FURIA_INDOMAVEL := "Fúria Indomável",
	LOBO_SOLITARIO := "Lobo Solitário",
	BUDOKA := "Budōka",
	DANCA_DAS_SOMBRAS := "Danca das Sombras",
	MOVIMENTACAO_GRACIOSA := "Movimentacao Graciosa",
	MANCHA_DA_CORRUPCAO := "Mancha da Corrupcao",
	TIROTHAN := "Tirothan",
	INSTINTO_ASSASSINO := "Instinto Assassino",
	SENSO_DA_NATUREZA := "Senso da Natureza",
	VENENO_PURO := "Veneno Puro",
	REFLEXO_ESTRATEGICO := "Reflexo Estrategico",
	MAESTRIA_DE_DEVASTACAO := "Maestria de Devastacao",
	INSPIRAR_DEFESAS := "Inspirar Defesas",
	KEN_NO_TATSUJIN := "Ken no Tatsujin",
	TABIBITO_NO_MICHI := "Tabibito no Michi",
	RECUPERAR_FLECHAS := "Recuperar Flechas",
	TATICAS_ARCANAS := "Taticas Arcanas",
	BRIGADOR_DE_TAVERNA := "Brigador de Taverna",
	KANI_NO_SUTAIRU := "Kani no Sutairu",
	EXOESQUELETO := "Exoesqueleto",
	ESPECIALISTA_ESCUDOS := "Especialista Escudos",
	FURIA := "Furia",



	// TODO: Conferir grafia
	MARTIR := "Mártir",
	MANTO_ROTHEN := "Manto Rothen",
	RECUPERACAO_SALVADORA := "Recuperação Salvadora",
	VINGANCA := "Vingança",
	CHAIN_OF_PAIN := "Chain of Pain",
	KANINO_SUTAIRU := "Kanino Sutairu",
	EXO_ESQUELETO := "Exo Esqueleto",
	BACKSTAB := "Backstab",
	RESILIENCIA_BARBARA := "Resiliência Barbará",
	SEMPRE_ALERTA := "Sempre Alerta",
	CHARGE := "Charge",
	VINCULO_ELEMENTAL := "Vínculo Elemental",
	BARREIRA_INDESTRUTIVEL := "Barreira Indestrutível",
	TRESPASSAR := "Trespassar",
	ATAQUE_FURTIVO := "Ataque Furtivo",
	ARMADURA_ARCANA := "Armadura Arcana",
	AINALAITH := "Ainalaith",
	CONHECIMENTO_SOBRE_INIMIGO := "Conhecimento sobre Inimigo",
	DEATH_DANCE := "Death Dance",
	DISPARO_MISTICO := "Disparo Místico",
	CONTRA_ATAQUE := "Contra-Ataque",
	DESVIAR_PROJETIL := "Desviar Projetil",
	DESVIAR_DISPAROS := "Desviar Disparos",
	MAO_LIVRE := "Mão Livre",
	FEROCIDADE_ENSANGUENTADA := "Ferocidade Ensanguentada",
	EVASAO := "Evasão",
	REBUKE := "Rebuke",
	ATAQUE_INFALIVEL := "Ataque Infalível",
	MESTRE_DUELISTA := "Mestre Duelista",
	INSISTENCIA := "Insistência",
	ARROW_RECOVERY := "Arrow Recovery",
	DIVINE_AUTHORITY := "Divine Authority"
endenum

enum class COMBATABILITY_MOMENT
	BEFORE_DAMAGE,
	AFTER_DAMAGE,
	ON_MISS,
	ON_HIT,
	ON_CRITICAL,
	ON_KILL
endenum
