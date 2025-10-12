use uo;

include "IMobileWrapper";
include ":attributes:skills";

class MobileWrapper(IMobileWrapper)
	var _mobRef; // MobRef

	var _name := "";
	var _str := 0;
	var _dex := 0;
	var _int := 0;

	var _currentHP := 0;
	var _currentStamina := 0;
	var _currentMana := 0;

	var _maxHP := 0;
	var _maxStamina := 0;
	var _maxMana := 0;

	var _gold := 0;
	var _weight := 0;

	var _skills := dictionary{}; // Dictionary of skills (string) and their base value (double)

	function MobileWrapper(this, byRef mobRef)
		if (mobRef.isA(POLCLASS_MOBILE)) // Caso o mobile exista
			this._mobRef := mobRef;

			this._name := mobRef.name;
			this._str := mobRef.str;
			this._dex := mobRef.dex;
			this._int := mobRef.int;

			this._currentHP := mobRef.currentHP;
			this._currentStamina := mobRef.currentStamina;
			this._currentMana := mobRef.currentMana;

			this._maxHP := mobRef.maxHP;
			this._maxStamina := mobRef.maxStamina;
			this._maxMana := mobRef.maxMana;

			this._gold := mobRef.gold;
			this._weight := mobRef.weight;
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
		return this._currentHP;
	endfunction

	function GetCurrentStamina(this)
		return this._currentStamina;
	endfunction

	function GetCurrentMana(this)
		return this._currentMana;
	endfunction

	function GetMaxHP(this)
		return this._maxHP;
	endfunction

	function GetMaxStamina(this)
		return this._maxStamina;
	endfunction

	function GetMaxMana(this)
		return this._maxMana;
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