use cfgfile;

include "ICombatAbility";

include ":diceroll:model/DiceRoll";

class CombatAbility(ICombatAbility)
	// TODO: Pensar se precisa encapsular os modificadores de cooldown, effect, damage, etc.
	// TODO: Pensar também se precisa encapsular executor e target.

	var _name := "";
	var _description := "";
	var _momentum := COMBATABILITY_MOMENTUM::ONHIT;
	var _cooldown := 0; // em segundos
	var _stamCost := 0;
	var _manaCost := 0;
	var _effectDuration := 0; // em segundos
	var _range := 1; // em tiles
	var _damage := DiceRoll("1d1");

	function CombatAbility(this, abilityName)
		var info := this._GetAbilityInfo(abilityName);
		this._name := info.name;
		this._description := info.description;
		this._momentum := info.momentum;
		this._cooldown := info.cooldown;
		this._stamCost := info.stamCost;
		this._manaCost := info.manaCost;
		this._effectDuration := info.effectDuration;
		this._range := info.range;
		this._damage := info.damage;
	endfunction

	function ExecuteOn(this, byRef executor, byRef target)
		// TODO: Implementar a lógica de execução da habilidade aqui
		// executor e target são referências aos personagens envolvidos
		return true;
	endfunction

	// Private methods
	function _GetAbilityInfo(this, tag)
		var cfgFile := ReadConfigFile(":charactercreation:config/habilidades");
		
		var info := struct{
			name := "",
			description := "",
			momentum := this._momentum,
			cooldown := 0,
			stamCost := 0,
			manaCost := 0,
			effectDuration := 0,
			range := 1,
			damage := DiceRoll("1d2")
		};

		if (tag == COMBATABILITY_NAME::MARTIR)
			var elem := FindConfigElem(cfgFile, "Martir");
			info.name := elem.nome;
			info.description := elem.desc;
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := CInt(elem.delay);
			info.stamCost := CInt(elem.stam);
			info.manaCost := CInt(elem.mana);
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::MANTOROTHEN)
			var elem := FindConfigElem(cfgFile, "Manto Rothen");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::RECUPERACAOSALVADORA)
			var elem := FindConfigElem(cfgFile, "Recuperacao Salvadora");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::VINGANCA)
			var elem := FindConfigElem(cfgFile, "Vinganca");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::CHAINOFPAIN)
			var elem := FindConfigElem(cfgFile, "Chain of Pain");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::TATICASARCANAS)
			var elem := FindConfigElem(cfgFile, "Taticas Arcanas");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::BRIGADORDETAVERNA)
			var elem := FindConfigElem(cfgFile, "Brigador de Taverna");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::KANINOSUTAIRU)
			var elem := FindConfigElem(cfgFile, "Kanino Sutairu");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::EXOESQUELETO)
			var elem := FindConfigElem(cfgFile, "Exoesqueleto");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::ESPECIALISTAESCUDOS)
			var elem := FindConfigElem(cfgFile, "Especialista Escudos");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::BACKSTAB)
			var elem := FindConfigElem(cfgFile, "Backstab");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::FURIA)
			var elem := FindConfigElem(cfgFile, "Furia");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::RESILIENCIABARBARA)
			var elem := FindConfigElem(cfgFile, "Resiliencia Barbara");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::SEMPREALERTA)
			var elem := FindConfigElem(cfgFile, "Sempre Alerta");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::CHARGE)
			var elem := FindConfigElem(cfgFile, "Charge");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::VINCULOELEMENTAL)
			var elem := FindConfigElem(cfgFile, "Vinculo Elemental");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::KENNOTATSUJIN)
			var elem := FindConfigElem(cfgFile, "Ken no Tatsujin");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::BARREIRAINDESTRUTIVEL)
			var elem := FindConfigElem(cfgFile, "Barreira Indestrutivel");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::TRESPASSAR)
			var elem := FindConfigElem(cfgFile, "Trespassar");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::ATAQUEFURTIVO)
			var elem := FindConfigElem(cfgFile, "Ataque Furtivo");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::ARMADURAARCANA)
			var elem := FindConfigElem(cfgFile, "Armadura Arcana");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::AINALAITH)
			var elem := FindConfigElem(cfgFile, "Ainalaith");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::CONHECIMENTOSOBREINIMIGO)
			var elem := FindConfigElem(cfgFile, "Conhecimento Sobre Inimigo");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::DEATHDANCE)
			var elem := FindConfigElem(cfgFile, "Death Dance");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::DISPAROMISTICO)
			var elem := FindConfigElem(cfgFile, "Disparo Mistico");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::CONTRAATAQUE)
			var elem := FindConfigElem(cfgFile, "Contra-ataque");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::DESVIARPROJETIL)
			var elem := FindConfigElem(cfgFile, "Desviar Projetil");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::DESVIARDISPAROS)
			var elem := FindConfigElem(cfgFile, "Desviar Disparos");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::MAOLIVRE)
			var elem := FindConfigElem(cfgFile, "Mao Livre");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::FEROCIDADEENSANGUENTADA)
			var elem := FindConfigElem(cfgFile, "Ferocidade Ensanguentada");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::EVASAO)
			var elem := FindConfigElem(cfgFile, "Evasao");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::REBUKE)
			var elem := FindConfigElem(cfgFile, "Rebuke");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::ATAQUEINFALIVEL)
			var elem := FindConfigElem(cfgFile, "Ataque Infalivel");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::MESTREDUELISTA)
			var elem := FindConfigElem(cfgFile, "Mestre Duelista");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::INSISTENCIA)
			var elem := FindConfigElem(cfgFile, "Insistencia");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::ARROWRECOVERY)
			var elem := FindConfigElem(cfgFile, "Arrow Recovery");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		elseif (tag == COMBATABILITY_NAME::DIVINEAUTHORITY)
			var elem := FindConfigElem(cfgFile, "Divine Authority");
			info.name := GetConfigString(elem, "nome");
			info.description := GetConfigString(elem, "desc");
			info.momentum := COMBATABILITY_MOMENTUM::ONHIT;
			info.cooldown := 0;
			info.cost := elem.stam;
			info.effectDuration := 0;
			info.range := 1;
			info.damage := DiceRoll("1d2");
		endif

		return info;
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