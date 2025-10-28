use cfgfile;

include "ICombatAbility";

include ":diceroll:model/DiceRoll";
include ":IO:CFGFile";

class CombatAbility(ICombatAbility)
	var _name := "";
	var _description := "";
	var _moment := COMBATABILITY_MOMENT::ONHIT;
	var _cooldown := 0; // em segundos
	var _cost := struct{
		hp := 0,
		stam := 0,
		mana := 0
	};
	var _effectDuration := 0; // em segundos
	var _range := 1; // em tiles
	var _damage := DiceRoll("1d1");

	function CombatAbility(this, abilityName)
		var cfgFile := CFGFile("charactercreation", "config/habilidades").SetElementName(abilityName);
		
		this._name := cfgFile.SetKeyName("name").GetKeyValue();
		this._description := cfgFile.SetKeyName("description").GetKeyValue();
		this._moment := cfgFile.SetKeyName("moment").GetKeyValue();
		this._cooldown := cfgFile.SetKeyName("cooldown").GetKeyValue();
		this._cost := CombatAbility::_cost;
		this._cost.hp := cfgFile.SetKeyName("hp").GetKeyValue();
		this._cost.stam := cfgFile.SetKeyName("stam").GetKeyValue();
		this._cost.mana := cfgFile.SetKeyName("mana").GetKeyValue();
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



	// TODO: Conferir grafia
	MARTIR := "Mártir",
	MANTO_ROTHEN := "Manto Rothen",
	RECUPERACAO_SALVADORA := "Recuperação Salvadora",
	VINGANCA := "Vingança",
	CHAIN_OF_PAIN := "Chain of Pain",
	TATICAS_ARCANAS := "Táticas Arcanas",
	BRIGADOR_DE_TAVERNA := "Brigador de Taverna",
	KANINO_SUTAIRU := "Kanino Sutairu",
	EXO_ESQUELETO := "Exo Esqueleto",
	ESPECIALISTA_ESCUDOS := "Especialista Escudos",
	BACKSTAB := "Backstab",
	FURIA := "Fúria",
	RESILIENCIA_BARBARA := "Resiliência Barbará",
	SEMPRE_ALERTA := "Sempre Alerta",
	CHARGE := "Charge",
	VINCULO_ELEMENTAL := "Vínculo Elemental",
	KEN_NO_TATSUJIN := "Ken no Tatsujin",
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