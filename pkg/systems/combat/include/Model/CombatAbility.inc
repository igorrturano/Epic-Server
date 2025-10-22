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

enum class COMBATABILITY_NAME
	MARTIR,
	MANTOROTHEN,
	RECUPERACAOSALVADORA,
	VINGANCA,
	CHAINOFPAIN,
	TATICASARCANAS,
	BRIGADORDETAVERNA,
	KANINOSUTAIRU,
	EXOESQUELETO,
	ESPECIALISTAESCUDOS,
	BACKSTAB,
	FURIA,
	RESILIENCIABARBARA,
	SEMPREALERTA,
	CHARGE,
	VINCULOELEMENTAL,
	KENNOTATSUJIN,
	BARREIRAINDESTRUTIVEL,
	TRESPASSAR,
	ATAQUEFURTIVO,
	ARMADURAARCANA,
	AINALAITH,
	CONHECIMENTOSOBREINIMIGO,
	DEATHDANCE,
	DISPAROMISTICO,
	CONTRAATAQUE,
	DESVIARPROJETIL,
	DESVIARDISPAROS,
	MAOLIVRE,
	FEROCIDADEENSANGUENTADA,
	EVASAO,
	REBUKE,
	ATAQUEINFALIVEL,
	MESTREDUELISTA,
	INSISTENCIA,
	ARROWRECOVERY,
	DIVINEAUTHORITY
endenum

enum class COMBATABILITY_MOMENT
	BEFORE_DAMAGE,
	AFTER_DAMAGE,
	ON_MISS,
	ON_HIT,
	ON_CRITICAL,
	ON_KILL
endenum