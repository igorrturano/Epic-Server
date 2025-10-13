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
		ALCHEMY := 0.0,
		MAGERY := 0.0,
		ARCANA := 0.0,
		SCIENCE := 0.0,
		THEOLOGY := 0.0,

		PARRY := 0.0,
		ONEHANDED := 0.0,
		TWOHANDED := 0.0,
		DUALWIELD := 0.0,
		RANGED := 0.0,
		TACTICS := 0.0,

		SNEAK := 0.0,
		PICKPOCKETING := 0.0,
		AWARENESS := 0.0,
		MECHANICS := 0.0,

		MEDICINE := 0.0,
		SURVIVAL := 0.0,

		METALWORKING := 0.0,
		WOODWORKING := 0.0,
		DOMESTIC_WORK := 0.0,
		GATHERING := 0.0,
		HANDLEANIMAL := 0.0,

		WRESTLING := 0.0
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
				this._skills[skillName] := AP_GetTrueSkill(mobRef, skillName);
			endforeach
		else
			error{"errortext" := "This character does not exist!"};
		endif
	endfunction

	function GetMobRef(this)
		return this._mobRef ?: error{"errortext" := "This character does not exist!"};
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
		return this._hp.current;
	endfunction

	function GetCurrentStamina(this)
		return this._stamina.current;
	endfunction

	function GetCurrentMana(this)
		return this._mana.current;
	endfunction

	function GetCurrentHunger(this)
		return this._hunger.current;
	endfunction

	function GetMaxHP(this)
		return this._hp.max;
	endfunction

	function GetMaxStamina(this)
		return this._stamina.max;
	endfunction

	function GetMaxMana(this)
		return this._mana.max;
	endfunction

	function GetMaxHunger(this)
		return this._hunger.max;
	endfunction

	function GetGold(this)
		return this._gold;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetSkill(this, skillName)
		return AP_GetSkill(this._mobRef, skillName);
	endfunction

	function GetSkillBase(this, skillName)
		return AP_GetTrueSkill(this._mobRef, skillName);
	endfunction

	function GetSkillMod(this, skillName)
		return AP_GetSkillMod(this._mobRef, skillName);
	endfunction

	function IsNPC(this)
		return this._mobRef.isA(POLCLASS_NPC);
	endfunction

	function SetSkillBase(this, skillName, value)
		AP_SetTrueSkill(this._mobRef, skillName, CDbl(value));
		return this;
	endfunction

	function SetSkillMod(this, skillName, value)
		AP_SetSkillMod(this._mobRef, skillName, CDbl(value));
		return this;
	endfunction

	function ApplyItemModifiers(this, byRef itemWrapped)
		if (TypeOf(itemWrapped) != "EquipWrapperArmor" && TypeOf(itemWrapped) != "EquipWrapperWeapon")
			return error {"errortext" := "The object passed must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif


	endfunction

	function RemoveItemModifiers(this, byRef itemWrapped)
		if (TypeOf(itemWrapped) != "EquipWrapperArmor" && TypeOf(itemWrapped) != "EquipWrapperWeapon")
			return error {"errortext" := "The object passed must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif
	endfunction
endclass