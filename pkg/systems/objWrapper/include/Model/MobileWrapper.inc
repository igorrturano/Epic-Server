use uo;

include "IMobileWrapper";
include "UObjectWrapper";
include "ArmorWrapper";
include "WeaponWrapper";

include ":attributes:skills";
include ":attributes:vitals";
include ":attributes:stats";
include ":combat:armorZones";
include ":combat:Model/CombatTraits";

class MobileWrapper(UObjectWrapper, IMobileWrapper)
	var _race := "";
	var _culture := "";
	var _religion := "";
	var _combatTraits; // CombatTraits class

	function MobileWrapper(this, byRef mobRef)
		Super(mobRef);

		if (mobRef.isA(POLCLASS_MOBILE)) // Caso mobRef seja válido
			this._race := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).race;
			this._culture := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).culture;
			this._religion := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).religion;
		else
			var err := error{errortext := "Param is not a mobRef or does not exist!"};
			this._objRef := err;
			this._name := err;
			this._race := err;
			this._culture := err;
			this._religion := err;
			this._combatTraits := err;
		endif
	endfunction

	function GetAccountName(this)
		return this._objRef.acctname;
	endfunction
	
	function GetRace(this)
		return this._race;
	endfunction

	function GetCulture(this)
		return this._culture;
	endfunction

	function GetReligion(this)
		return this._religion;
	endfunction
	
	function GetStr(this)
		return this.GetStrBase() + this.GetStrMod();
	endfunction

	function GetStrBase(this)
		return AP_GetTrueStat(this._objRef, STRENGTH);
	endfunction

	function GetStrMod(this)
		return AP_GetStatMod(this._objRef, STRENGTH);
	endfunction

	function GetDex(this)
		return this.GetDexBase() + this.GetDexMod();
	endfunction

	function GetDexBase(this)
		return AP_GetTrueStat(this._objRef, DEXTERITY);
	endfunction

	function GetDexMod(this)
		return AP_GetStatMod(this._objRef, DEXTERITY);
	endfunction

	function GetInt(this)
		return this.GetIntBase() + this.GetIntMod();
	endfunction

	function GetIntBase(this)
		return AP_GetTrueStat(this._objRef, INTELLIGENCE);
	endfunction

	function GetIntMod(this)
		return AP_GetStatMod(this._objRef, INTELLIGENCE);
	endfunction

	function GetCurrentHP(this)
		this.Update();
		return AP_GetVital(this._objRef, HITS);
	endfunction

	function GetCurrentStamina(this)
		this.Update();
		return AP_GetVital(this._objRef, STAMINA);
	endfunction

	function GetCurrentMana(this)
		this.Update();
		return AP_GetVital(this._objRef, MANA);
	endfunction

	function GetCurrentHunger(this)
		this.Update();
		return AP_GetVital(this._objRef, HUNGER);
	endfunction

	function GetMaxHP(this)
		this.Update();
		return AP_GetVitalMaximumValue(this._objRef, HITS);
	endfunction

	function GetMaxStamina(this)
		this.Update();
		return AP_GetVitalMaximumValue(this._objRef, STAMINA);
	endfunction

	function GetMaxMana(this)
		this.Update();
		return AP_GetVitalMaximumValue(this._objRef, MANA);
	endfunction

	function GetMaxHunger(this)
		this.Update();
		return AP_GetVitalMaximumValue(this._objRef, HUNGER);
	endfunction

	function GetRegenHP(this)
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, HITS);
	endfunction

	function GetRegenStamina(this)
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, STAMINA);
	endfunction

	function GetRegenMana(this)
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, MANA);
	endfunction

	function GetRegenHunger(this)
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, HUNGER);
	endfunction

	function GetGender(this)
		if (this._objRef.gender == 0)
			return MOBILE_GENDER::MALE;
		elseif (this._objRef.gender == 1)
			return MOBILE_GENDER::FEMALE;
		else
			return MOBILE_GENDER::UNDEFINED;
		endif
	endfunction

	function GetCriticalChance(this)
		return CInt(this.GetCriticalChanceBase() + this.GetCriticalChanceMod());
	endfunction
	
	function GetCriticalChanceBase(this)
		return CInt(this._objRef.GetProp("CriticalChance"));
	endfunction

	function GetCriticalChanceMod(this)
		return CInt(this._objRef.GetProp("#CriticalChance"));
	endfunction

	function GetNPCTemplateName(this)
		return this.IsNPC() ? this._objRef.npctemplate : error{errortext := "This mobile is not a NPC!"};
	endfunction

	function GetNPCTemplate(this)
		return this.IsNPC() ? NPC_GetNPCConfig(this._objRef.npctemplate) : error{errortext := "This mobile is not a NPC or has no template configured!"};
	endfunction

	function GetNPCDamage(this)
		return this.IsNPC() ? NPC_GetNPCConfig(this._objRef.npctemplate).Damage : error{errortext := "This mobile is not a NPC!"};
	endfunction

	function GetEquipBySlot(this, equipSlot) // Os enum estão em EquipWrapper::EQUIP_SLOT
		var equipsInSlot;

		// Feito assim pra garantir que a variável pertença ao ENUM, e também caso necessite customizar o retorno de algum slot
		if (equipSlot == WEAPON)
			return this._objRef.weapon ? WeaponWrapper(this._objRef.weapon) : error{errortext := "Empty slot!"};
		elseif (equipSlot == SHIELD)
			return this._objRef.shield ? ArmorWrapper(this._objRef.shield) : error{errortext := "Empty slot!"};
		elseif (equipSlot == HEAD)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, HEAD);
		elseif (equipSlot == NECK)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, NECK);
		elseif (equipSlot == ARMS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, ARMS);
		elseif (equipSlot == BODY)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, BODY);
		elseif (equipSlot == HANDS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, HANDS);
		elseif (equipSlot == LEGS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, LEGS);
		elseif (equipSlot == FEET)
			equipsInSlot := CS_GetEquipmentInArmorZone(this._objRef, FEET);
		else
			return error{errortext := "Wrong slot specified!"};
		endif

		if (equipsInSlot.size() == 0)
			return error{errortext := "Empty slot!"};
		endif

		foreach equip in (equipsInSlot)
			equip := ArmorWrapper(equip);
		endforeach

		return equipsInSlot;
	endfunction

	function GetEquipByLayer(this, layerEnum)
		return ArmorWrapper(GetEquipmentByLayer(this._objRef, layerEnum));
	endfunction

	function GetSkill(this, skillName)
		return this.GetSkillBase(skillName) + this.GetSkillMod(skillName);
	endfunction

	function GetSkillBase(this, skillName)
		return AP_GetTrueSkill(this._objRef, skillName);
	endfunction

	function GetSkillMod(this, skillName)
		return AP_GetSkillMod(this._objRef, skillName);
	endfunction

	function GetAbility(this, abilityName)
		// TODO: Implementar retorno de CombatAbility
	endfunction

	function GetCombatTraits(this)
		var propData := this._objRef.GetProp(MOBILE_PROPNAME::COMBAT_TRAITS);
		if (!propData)
			propData := CombatTraits().Serialize();
		endif

		this._combatTraits := CombatTraits().Deserialize(propData);

		return this._combatTraits;
	endfunction

	function SetSkillBase(this, skillName, value)
		AP_SetTrueSkill(this._objRef, skillName, CDbl(value));
		return this;
	endfunction

	function SetSkillMod(this, skillName, value)
		AP_SetSkillMod(this._objRef, skillName, CDbl(value));
		return this;
	endfunction

	function SetCombatTraits(this, combatTrait) // combatTrait: CombatTraits
		if (!(combatTrait is @CombatTraits))
			return error{errortext := "The param must be a CombatTraits object!"};
		endif
		
		this._combatTraits := combatTrait;
		this._objRef.SetProp(MOBILE_PROPNAME::COMBAT_TRAITS, this._combatTraits.Serialize());

		return this;
	endfunction

	function ApplyItemModifiers(this, byRef itemWrapped)
		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return error{errortext := "The param must be an ArmorWrapper or WeaponWrapper object!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.ApplyOn(this._objRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function RemoveItemModifiers(this, byRef itemWrapped)
		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return error{errortext := "The param must be an ArmorWrapper or WeaponWrapper object!"};
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.RemoveFrom(this._objRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function HasAbility(this, abilityName)
		return TemHabilidade(this._objRef, abilityName);
	endfunction

	function IsNPC(this)
		return this._objRef.npctemplate != error;
	endfunction

	function IsDead(this)
		return this._objRef.dead == true;
	endfunction

	function IsWarMode(this)
		return this._objRef.warmode == true;
	endfunction

	function IsMounted(this)
		return this.GetEquipByLayer(LAYER_MOUNT) == true;
	endfunction

	function IsConnected(this)
		return this._objRef.connected == true;
	endfunction

	function DistanceTo(this, otherObj) // other: ObjRef
		if (!otherObj.IsA(POLCLASS_UOBJECT))
			return error{errortext := "The param must be an ObjRef!"};
		endif

		return Distance(this._objRef, otherObj);
	endfunction

	function PlayAnimation(this, animID, framecount := 5, repeatcount := 1, backward := ACTION_DIR_FORWARD, repeatflag := ACTION_NOREPEAT, delay := 1)
		PerformAction(this._objRef, animID, repeatcount, framecount, backward, repeatflag, delay);
		return this;
	endfunction

	function Update(this)
		RecalcVitals(this._objRef);
	endfunction
	
	function Dispose(this)
		this._objRef := 0;
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
			return error{errortext := "The param must be a MobileWrapper object!"};
		endif

		return this.GetObjRef() == otherMobileWrapper.GetObjRef() &&
			   this.GetSerial() == otherMobileWrapper.GetSerial();
	endfunction

	function ToString(this)
	endfunction
endclass

enum class MOBILE_GENDER
	MALE := "Male",
	FEMALE := "Female",
	UNDEFINED := "Undefined"
endenum

enum class MOBILE_PROPNAME
	CHARDATA := "chardata",
	COMBAT_TRAITS := "combattraits"
endenum
