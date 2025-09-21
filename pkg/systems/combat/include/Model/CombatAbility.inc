include "ICombatAbility";

include ":diceroll:model/DiceRoll";

class CombatAbility(ICombatAbility)
	var _name := "";
	var _description := "";
	var _momentum := COMBATABILITY_MOMENTUM::ONHIT;
	var _cooldown := 0; // em segundos
	var _effectDuration := 0; // em segundos
	var _range := 1; // em tiles
	var _damage := DiceRoll("1d1");

	function CombatAbility(this, abilityName)
	endfunction

	function _GetAbilityInfo(this, tag)
		var info := struct{
			name := "",
			description := "",
			momentum := this._momentum,
			cooldown := 0,
			effectDuration := 0,
			range := 1,
			damage := DiceRoll("1d2")
		};
	endfunction

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

enum class COMBATABILITY_MOMENTUM
	BEFOREDAMAGE,
	AFTERDAMAGE,
	ONMISS,
	ONHIT,
	ONCRITICAL,
	ONKILL
endenum