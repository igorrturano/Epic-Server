use uo;

include "IMobileWrapper";
include ":attributes:skills";

class MobileWrapper(IMobileWrapper)
	var _mobRef; // mobRef

	var _name := "";
	var _str := 0;
	var _dex := 0;
	var _int := 0;

	var _hp := struct{
		current := 0,
		max := 0,
		regen := 0
	};
	var _stamina := struct{
		current := 0,
		max := 0,
		regen := 0
	};
	var _mana := struct{
		current := 0,
		max := 0,
		regen := 0
	};
	var _hunger := struct{
		current := 0,
		max := 0,
		regen := 0
	};

	var _gold := 0;
	var _weight := 0;

	var _skills := dictionary{ // Dictionary of skills (string) and their base value (double)
		ALCHEMY := struct{
			base := 0.0,
			mod := 0.0
		},
		MAGERY := struct{
			base := 0.0,
			mod := 0.0
		},
		ARCANA := struct{
			base := 0.0,
			mod := 0.0
		},
		SCIENCE := struct{
			base := 0.0,
			mod := 0.0
		},
		THEOLOGY := struct{
			base := 0.0,
			mod := 0.0
		},

		PARRY := struct{
			base := 0.0,
			mod := 0.0
		},
		ONEHANDED := struct{
			base := 0.0,
			mod := 0.0
		},
		TWOHANDED := struct{
			base := 0.0,
			mod := 0.0
		},
		DUALWIELD := struct{
			base := 0.0,
			mod := 0.0
		},
		RANGED := struct{
			base := 0.0,
			mod := 0.0
		},
		TACTICS := struct{
			base := 0.0,
			mod := 0.0
		},

		SNEAK := struct{
			base := 0.0,
			mod := 0.0
		},
		PICKPOCKETING := struct{
			base := 0.0,
			mod := 0.0
		},
		AWARENESS := struct{
			base := 0.0,
			mod := 0.0
		},
		MECHANICS := struct{
			base := 0.0,
			mod := 0.0
		},

		MEDICINE := struct{
			base := 0.0,
			mod := 0.0
		},
		SURVIVAL := struct{
			base := 0.0,
			mod := 0.0
		},

		METALWORKING := struct{
			base := 0.0,
			mod := 0.0
		},
		WOODWORKING := struct{
			base := 0.0,
			mod := 0.0
		},
		DOMESTIC_WORK := struct{
			base := 0.0,
			mod := 0.0
		},
		GATHERING := struct{
			base := 0.0,
			mod := 0.0
		},
		HANDLEANIMAL := struct{
			base := 0.0,
			mod := 0.0
		},
		WRESTLING := struct{
			base := 0.0,
			mod := 0.0
		}
	};

	function MobileWrapper(this, byRef mobRef)
		if (mobRef.isA(POLCLASS_MOBILE)) // Caso o mobile exista
			// TODO: Checar os atributos corretos, aqui é só um exemplo
			this._mobRef := mobRef;

			this._name := mobRef.name;
			this._str := mobRef.str;
			this._dex := mobRef.dex;
			this._int := mobRef.int;

			this._hp := MobileWrapper::_hp;
			this._hp.current := AP_GetVital(mobRef, HITS);
			this._hp.max := AP_GetVitalMaximumValue(mobRef, HITS);
			this._hp.regen := AP_GetVitalRegenRate(mobRef, HITS);

			this._stamina := MobileWrapper::_stamina;
			this._stamina.current := AP_GetVital(mobRef, STAMINA);
			this._stamina.max := AP_GetVitalMaximumValue(mobRef, STAMINA);
			this._stamina.regen := AP_GetVitalRegenRate(mobRef, STAMINA);

			this._mana := MobileWrapper::_mana;
			this._mana.current := AP_GetVital(mobRef, MANA);
			this._mana.max := AP_GetVitalMaximumValue(mobRef, MANA);
			this._mana.regen := AP_GetVitalRegenRate(mobRef, MANA);

			this._hunger := MobileWrapper::_hunger;
			this._hunger.current := AP_GetVital(mobRef, HUNGER);
			this._hunger.max := AP_GetVitalMaximumValue(mobRef, HUNGER);
			this._hunger.regen := AP_GetVitalRegenRate(mobRef, HUNGER);

			this._gold := mobRef.gold;
			this._weight := mobRef.weight;

			this._skills := MobileWrapper::_skills;
			foreach skillName in (this._skills.keys())
				this._skills[skillName].base := AP_GetTrueSkill(mobRef, skillName);
				this._skills[skillName].mod := AP_GetSkillMod(mobRef, skillName);
			endforeach
		else
			var err := error{ errortext := "Param is not a mobRef or does not exist!" };
			this._mobRef := err;
			this._name := err;
			this._str := err;
			this._dex := err;
			this._int := err;
			this._hp := err;
			this._stamina := err;
			this._mana := err;
			this._hunger := err;
			this._gold := err;
			this._weight := err;
			this._skills := err;
		endif
	endfunction

	function GetMobRef(this)
		return this._mobRef;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetStr(this)
		return this._str;
	endfunction

	function GetDex(this)
		return this._dex;
	endfunction

	function GetInt(this)
		return this._int;
	endfunction

	function GetCurrentHP(this)
		this._hp.current := AP_GetVital(this._mobRef, HITS);
		return this._hp.current;
	endfunction

	function GetCurrentStamina(this)
		this._stamina.current := AP_GetVital(this._mobRef, STAMINA);
		return this._stamina.current;
	endfunction

	function GetCurrentMana(this)
		this._mana.current := AP_GetVital(this._mobRef, MANA);
		return this._mana.current;
	endfunction

	function GetCurrentHunger(this)
		this._hunger.current := AP_GetVital(this._mobRef, HUNGER);
		return this._hunger.current;
	endfunction

	function GetMaxHP(this)
		this._hp.max := AP_GetVitalMaximumValue(this._mobRef, HITS);
		return this._hp.max;
	endfunction

	function GetMaxStamina(this)
		this._stamina.max := AP_GetVitalMaximumValue(this._mobRef, STAMINA);
		return this._stamina.max;
	endfunction

	function GetMaxMana(this)
		this._mana.max := AP_GetVitalMaximumValue(this._mobRef, MANA);
		return this._mana.max;
	endfunction

	function GetMaxHunger(this)
		this._hunger.max := AP_GetVitalMaximumValue(this._mobRef, HUNGER);
		return this._hunger.max;
	endfunction

	function GetGold(this)
		return this._gold;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetSkill(this, skillName)
		this._skills[skillName] := AP_GetSkill(this._mobRef, skillName);
		return this._skills[skillName];
	endfunction

	function GetSkillBase(this, skillName)
		this._skills[skillName] := AP_GetTrueSkill(this._mobRef, skillName);
		return this._skills[skillName].base;
	endfunction

	function GetSkillMod(this, skillName)
		this._skills[skillName] := AP_GetSkillMod(this._mobRef, skillName);
		return this._skills[skillName].mod;
	endfunction

	function IsNPC(this)
		return this._mobRef.isA(POLCLASS_NPC);
	endfunction

	function SetSkillBase(this, skillName, value)
		this._skills[skillName].base := AP_SetTrueSkill(this._mobRef, skillName, CDbl(value));
		return this;
	endfunction

	function SetSkillMod(this, skillName, value)
		this._skills[skillName].mod := AP_SetSkillMod(this._mobRef, skillName, CDbl(value));
		return this;
	endfunction

	function ApplyItemModifiers(this, byRef itemWrapped)
		if (!itemWrapped.GetTemplate())
			return error {"errortext" := "The param must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.ApplyOn(this._mobRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function RemoveItemModifiers(this, byRef itemWrapped)
		if (!itemWrapped.GetTemplate())
			return error {"errortext" := "The param must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.RemoveFrom(this._mobRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction
endclass