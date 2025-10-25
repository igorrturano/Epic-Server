use uo;

include "IMobileWrapper";

include ":attributes:skills";
include ":attributes:vitals";
include ":attributes:stats";
include ":equipsys:ViewModel/EquipWrapperArmor";
include ":equipsys:ViewModel/EquipWrapperWeapon";
include ":combat:armorZones";

class MobileWrapper(IMobileWrapper)
	var _mobRef; // mobRef

	var _name := "";
	var _str := struct{
		base := 0,
		mod := 0
	};
	var _dex := struct{
		base := 0,
		mod := 0
	};
	var _int := struct{
		base := 0,
		mod := 0
	};

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

	function MobileWrapper(this, byRef mobRef)
		if (mobRef.isA(POLCLASS_MOBILE)) // Caso o mobile exista
			// TODO: Checar os atributos corretos, aqui é só um exemplo
			this._mobRef := mobRef;

			this._name := mobRef.name;
			this._str := MobileWrapper::_str;
			this._str.base := this.GetStrBase();
			this._str.mod := this.GetStrMod();
			this._dex := MobileWrapper::_dex;
			this._dex.base := this.GetDexBase();
			this._dex.mod := this.GetDexMod();
			this._int := MobileWrapper::_int;
			this._int.base := this.GetIntBase();
			this._int.mod := this.GetIntMod();

			this._hp := MobileWrapper::_hp;
			this._hp.current := this.GetCurrentHP();
			this._hp.max := this.GetMaxHP();
			this._hp.regen := this.GetRegenHP();
			this._stamina := MobileWrapper::_stamina;
			this._stamina.current := this.GetCurrentStamina();
			this._stamina.max := this.GetMaxStamina();
			this._stamina.regen := this.GetRegenStamina();
			this._mana := MobileWrapper::_mana;
			this._mana.current := this.GetCurrentMana();
			this._mana.max := this.GetMaxMana();
			this._mana.regen := this.GetRegenMana();
			this._hunger := MobileWrapper::_hunger;
			this._hunger.current := this.GetCurrentHunger();
			this._hunger.max := this.GetMaxHunger();
			this._hunger.regen := this.GetRegenHunger();

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

	function GetSerial(this)
		return this._mobRef.serial;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetPosition(this)
		return struct{
			x := this._mobRef.x,
			y := this._mobRef.y,
			z := this._mobRef.z,
			realm := this._mobRef.realm
		};
	endfunction

	function GetStr(this)
		return this.GetStrBase() + this.GetStrMod();
	endfunction

	function GetStrBase(this)
		this._str.base := AP_GetTrueStat(this._mobRef, STRENGTH);
		return this._str.base;
	endfunction

	function GetStrMod(this)
		this._str.mod := AP_GetStatMod(this._mobRef, STRENGTH);
		return this._str.mod;
	endfunction

	function GetDex(this)
		return this.GetDexBase() + this.GetDexMod();
	endfunction

	function GetDexBase(this)
		this._dex.base := AP_GetTrueStat(this._mobRef, DEXTERITY);
		return this._dex.base;
	endfunction

	function GetDexMod(this)
		this._dex.mod := AP_GetStatMod(this._mobRef, DEXTERITY);
		return this._dex.mod;
	endfunction

	function GetInt(this)
		return this.GetIntBase() + this.GetIntMod();
	endfunction

	function GetIntBase(this)
		this._int.base := AP_GetTrueStat(this._mobRef, INTELLIGENCE);
		return this._int.base;
	endfunction

	function GetIntMod(this)
		this._int.mod := AP_GetStatMod(this._mobRef, INTELLIGENCE);
		return this._int.mod;
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

	function GetRegenHP(this)
		this._hp.regen := AP_GetVitalRegenRate(this._mobRef, HITS);
		return this._hp.regen;
	endfunction

	function GetRegenStamina(this)
		this._stamina.regen := AP_GetVitalRegenRate(this._mobRef, STAMINA);
		return this._stamina.regen;
	endfunction

	function GetRegenMana(this)
		this._mana.regen := AP_GetVitalRegenRate(this._mobRef, MANA);
		return this._mana.regen;
	endfunction

	function GetRegenHunger(this)
		this._hunger.regen := AP_GetVitalRegenRate(this._mobRef, HUNGER);
		return this._hunger.regen;
	endfunction

	function GetGold(this)
		return this._gold;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetGender(this)
		if (this._mobRef.gender == 0)
			return MOBILE_GENDER::MALE;
		elseif (this._mobRef.gender == 1)
			return MOBILE_GENDER::FEMALE;
		else
			return MOBILE_GENDER::UNDEFINED;
		endif
	endfunction

	function GetNPCTemplate(this)
		if (!this.IsNPC())
			return error{ "errortext" := "This mobile is not an NPC!" };
		else
			return this._mobRef.npctemplate;
		endif
	endfunction

	function GetEquipBySlot(this, equipSlot) // Os enum estão em EquipWrapper::EQUIP_SLOT
		var equipsInSlot;

		// Feito assim pra garantir que a variável pertença ao ENUM, e também caso necessite customizar o retorno de algum slot
		if (equipSlot == WEAPON)
			return EquipWrapperWeapon(this._mobRef.weapon);
		elseif (equipSlot == SHIELD)
			return EquipWrapperArmor(this._mobRef.shield);
		elseif (equipSlot == HEAD)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, HEAD);
		elseif (equipSlot == NECK)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, NECK);
		elseif (equipSlot == ARMS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, ARMS);
		elseif (equipSlot == BODY)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, BODY);
		elseif (equipSlot == HANDS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, HANDS);
		elseif (equipSlot == LEGS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, LEGS);
		elseif (equipSlot == FEET)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._mobRef, FEET);
		else
			return error {"errortext" := "Wrong slot specified!"};
		endif

		foreach equip in (equipsInSlot)
			equip := EquipWrapperArmor(equip);
		endforeach

		return equipsInSlot;
	endfunction

	function GetEquipByLayer(this, layerEnum) // Os enum estão em EquipWrapper::EQUIP_LAYER
		return EquipWrapperArmor(GetEquipmentByLayer(this._mobRef, layerEnum));
	endfunction

	function GetSkill(this, skillName)
		return this.GetSkillBase(skillName) + this.GetSkillMod(skillName);
	endfunction

	function GetSkillBase(this, skillName)
		this._skills[skillName].base := AP_GetTrueSkill(this._mobRef, skillName);
		return this._skills[skillName].base;
	endfunction

	function GetSkillMod(this, skillName)
		this._skills[skillName].mod := AP_GetSkillMod(this._mobRef, skillName);
		return this._skills[skillName].mod;
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
		if (!(itemWrapped is @EquipWrapperArmor) && !(itemWrapped is @EquipWrapperWeapon))
			return error {"errortext" := "The param must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.ApplyOn(this._mobRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function RemoveItemModifiers(this, byRef itemWrapped)
		if (!(itemWrapped is @EquipWrapperArmor) && !(itemWrapped is @EquipWrapperWeapon))
			return error {"errortext" := "The param must be an EquipWrapperArmor or EquipWrapperWeapon!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.RemoveFrom(this._mobRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function IsNPC(this)
		return this._mobRef.npctemplate != false;
	endfunction

	function IsDead(this)
		return this._mobRef.dead == true;
	endfunction

	function IsWarMode(this)
		return this._mobRef.warmode == true;
	endfunction

	function PlaySound(this, soundId, isPrivate := false, soundOrigin := false)
		if (isPrivate)
			PlaySoundEffectPrivate(this._mobRef, soundId, soundOrigin ?: this._mobRef);
		else
			PlaySoundEffect(this._mobRef, soundId);
		endif

		return this;
	endfunction

	function Dispose(this)
		this._mobRef := 0;
		this._name := "";
		this._str := 0;
		this._dex := 0;
		this._int := 0;
		this._hp := 0;
		this._stamina := 0;
		this._mana := 0;
		this._hunger := 0;
		this._gold := 0;
		this._weight := 0;
		this._skills := 0;
	endfunction

	function Equals(this, byRef otherMobileWrapper)
		if (!(otherMobileWrapper is @MobileWrapper))
			return error{ "errortext" := "The param must be a MobileWrapper!" };
		endif

		return this.GetMobRef() == otherMobileWrapper.GetMobRef() &&
			   this.GetSerial() == otherMobileWrapper.GetSerial();
	endfunction
endclass

enum class MOBILE_GENDER
	MALE := "Male",
	FEMALE := "Female",
	UNDEFINED := "Undefined"
endenum