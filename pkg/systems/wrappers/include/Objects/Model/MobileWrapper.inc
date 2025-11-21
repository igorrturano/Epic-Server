use uo;

// Exceptions
include ":wrappers:Objects/Exceptions/_all";
include "system/exception/FormatException/InvalidFormatException";

// Internal
include "IMobileWrapper";
include "UObjectWrapper";
include "ArmorWrapper";
include "WeaponWrapper";

// External
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

	function MobileWrapper(this, byRef mobile)
		Super(mobile);

		if (mobile.isA(POLCLASS_NPC))
			this._cfgFile := CFGFile("epicAI", "npcdesc");
			this._template := this._cfgFile.SetElementName(NPC_GetNPCConfig(mobile.npctemplate).template).GetElementValue();
			this._name := this._objRef.name;

			// Variáveis que indicarão que é um NPC
			var err := IsNPCException();
			this._race := err;
			this._culture := err;
			this._religion := err;
			this._combatTraits := err;
		elseif (mobile.isA(POLCLASS_MOBILE)) // Se entrar aqui, é player
			this._race := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).race;
			this._culture := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).culture;
			this._religion := this._objRef.GetProp(MOBILE_PROPNAME::CHARDATA).religion;

			// Variáveis que indicarão que é um player
			var err := IsPlayerException();
			this._template := err;
		elseif (TypeOfInt(mobile) == OT_STRING) // Caso seja apenas um template
			this._cfgFile := CFGFile("epicAI", "npcdesc");
			this._template := this._cfgFile.SetElementName(mobile).GetElementValue();

			// Variáveis que indicarão que é apenas um template
			var err := IsTemplateException();
			this._objRef := err;
			this._name := err;
			this._race := err;
			this._culture := err;
			this._religion := err;
			this._combatTraits := err;
		else // Caso não seja nenhum valor válido
			var err := InvalidFormatException("The param must be a Mobile ObjRef or NPC template string!");
			this._template := err;
			this._objRef := err;
			this._name := err;
			this._race := err;
			this._culture := err;
			this._religion := err;
			this._combatTraits := err;
		endif
	endfunction

	function GetAccountName(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.acctname;
	endfunction
	
	function GetRace(this)
		if (this._race is @Exception)
			return this._race;
		endif

		return this._race;
	endfunction

	function GetCulture(this)
		if (this._culture is @Exception)
			return this._culture;
		endif

		return this._culture;
	endfunction

	function GetReligion(this)
		if (this._religion is @Exception)
			return this._religion;
		endif

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
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this.Update();
		return AP_GetVital(this._objRef, HITS);
	endfunction

	function GetCurrentStamina(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this.Update();
		return AP_GetVital(this._objRef, STAMINA);
	endfunction

	function GetCurrentMana(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		this.Update();
		return AP_GetVital(this._objRef, MANA);
	endfunction

	function GetCurrentHunger(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

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
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, HITS);
	endfunction

	function GetRegenStamina(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, STAMINA);
	endfunction

	function GetRegenMana(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, MANA);
	endfunction

	function GetRegenHunger(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		this.Update();
		return AP_GetVitalRegenRate(this._objRef, HUNGER);
	endfunction

	function GetGender(this)
		if (this._objRef is @Exception)
			if (this._template is @Exception)
				return error{errortext := "This is not a Player, NPC or valid template!"};
			else
				if (this._template.Gender == 0)
					return MOBILE_GENDER::MALE;
				elseif (this._template.Gender == 1)
					return MOBILE_GENDER::FEMALE;
				else
					return MOBILE_GENDER::UNDEFINED;
				endif
			endif
		endif
		
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
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return CInt(this._objRef.GetProp("CriticalChance"));
	endfunction

	function GetCriticalChanceMod(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return CInt(this._objRef.GetProp("#CriticalChance"));
	endfunction

	function GetNPCTemplateName(this)
		if (this._template is @Exception)
			return this._template;
		endif

		return this._cfgFile.GetElementName();
	endfunction

	function GetNPCTemplate(this)
		if (this._template is @Exception)
			return this._template;
		endif
		
		return this._template;
	endfunction

	function GetNPCDamage(this)
		if (this._template is @Exception)
			return this._template;
		endif
		
		return this._template.Damage ?: error{errortext := "This mobile is not a NPC!"};
	endfunction

	/**
	 * Obtém o equipamento equipado em um slot específico do personagem.
	 *
	 * @param this Referência ao objeto Mobile (personagem)
	 * @param equipSlot ENUM do Slot do equipamento a ser consultado (WEAPON, SHIELD, HEAD, NECK, ARMS, BODY, HANDS, LEGS, FEET)
	 *
	 * @return WeaponWrapper|ArmorWrapper|Array<ArmorWrapper>|error
	 *         - Para WEAPON: retorna WeaponWrapper ou erro se o slot estiver vazio
	 *         - Para SHIELD: retorna ArmorWrapper ou erro se o slot estiver vazio
	 *         - Para outros slots de armadura: retorna array de ArmorWrapper ou erro se o slot estiver vazio
	 *         - Retorna erro se o slot especificado for inválido
	 *
	 * @error "Empty slot!" - Quando não há equipamento no slot especificado
	 * @error "Wrong slot specified!" - Quando o slot informado não é válido
	 */
	function GetEquipBySlot(this, equipSlot)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
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

	/*
	 * Obtém o equipamento equipado em uma camada específica do personagem
	 *
	 * Parâmetros:
	 *   this      - Referência ao objeto mobile
	 *   layerEnum - Enumeração da camada de equipamento a ser consultada
	 *
	 * Retorna:
	 *   ArmorWrapper - Objeto wrapper da armadura equipada na camada especificada
	 *                  ou invalid se não houver equipamento na camada
	 *
	 * Descrição:
	 *   Esta função encapsula a chamada nativa GetEquipmentByLayer e retorna
	 *   o resultado como um ArmorWrapper para facilitar a manipulação do item.
	 */
	function GetEquipByLayer(this, layerEnum)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
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
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		AP_SetTrueSkill(this._objRef, skillName, CDbl(value));
		return this;
	endfunction

	function SetSkillMod(this, skillName, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		AP_SetSkillMod(this._objRef, skillName, CDbl(value));
		return this;
	endfunction

	/**
	 * Define as características de combate do mobile.
	 * 
	 * @param this Referência ao objeto MobileWrapper
	 * @param combatTrait Objeto CombatTraits contendo as características de combate a serem definidas
	 * 
	 * @return this Retorna a própria instância do objeto para encadeamento de métodos
	 * @return error Retorna um erro se o parâmetro não for um objeto CombatTraits
	 * 
	 * @example
	 * var traits := CombatTraits.Create();
	 * mobile.SetCombatTraits(traits);
	 */
	function SetCombatTraits(this, combatTrait)
		if (!(combatTrait is @CombatTraits))
			return InvalidFormatException("The param must be a CombatTraits object!");
		endif
		
		this._combatTraits := combatTrait;
		this._objRef.SetProp(MOBILE_PROPNAME::COMBAT_TRAITS, this._combatTraits.Serialize());

		return this;
	endfunction

	/**
	 * Aplica os modificadores de um item no objeto móvel.
	 *
	 * @param itemWrapped ArmorWrapper|WeaponWrapper - Objeto wrapper de armadura ou arma contendo os modificadores
	 * @return MobileWrapper - Retorna a própria instância para encadeamento de métodos
	 * @throws error - Retorna erro se o parâmetro não for um ArmorWrapper ou WeaponWrapper
	 *
	 * @description
	 * Esta função itera sobre todos os modificadores do item fornecido e os aplica
	 * na referência do objeto móvel (_objRef). Um pequeno atraso de 2ms é adicionado
	 * entre cada aplicação de modificador.
	 */
	function ApplyItemModifiers(this, byRef itemWrapped)
		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return InvalidFormatException("The param must be an ArmorWrapper or WeaponWrapper object!");
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.ApplyOn(this._objRef);
			SleepMS(2);
		endforeach

		return this;
	endfunction

	/*
	 * Remove modificadores de um item equipado
	 *
	 * Remove todos os modificadores de um item (arma ou armadura) do mobile.
	 * Itera por cada modificador do item e os remove da referência do objeto.
	 *
	 * @param itemWrapped ArmorWrapper|WeaponWrapper - O objeto wrapper do item (armadura ou arma)
	 *
	 * @return MobileWrapper - Retorna a própria instância para encadeamento de métodos
	 * @return Error - Retorna erro se o parâmetro não for um ArmorWrapper ou WeaponWrapper
	 */
	function RemoveItemModifiers(this, byRef itemWrapped)
		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return InvalidFormatException("The param must be an ArmorWrapper or WeaponWrapper object!");
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
		return !(this._template is @Exception);
	endfunction

	function IsDead(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this._objRef.dead == true;
	endfunction

	function IsWarMode(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.warmode == true;
	endfunction

	function IsMounted(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this.GetEquipByLayer(LAYER_MOUNT) == true;
	endfunction

	function IsConnected(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.connected;
	endfunction

	/*
	 * Calcula a distância entre este objeto e outro objeto.
	 *
	 * @param this      Referência ao objeto atual
	 * @param otherObj  Referência ao objeto de destino (deve ser um POLCLASS_UOBJECT)
	 *
	 * @return Integer  Distância entre os dois objetos
	 * @return Error    Retorna um erro se otherObj não for uma referência de objeto válida
	 *
	 * @example
	 *   var distancia := mobile.DistanceTo(target);
	 */
	function DistanceTo(this, otherObj)
		if (!otherObj.IsA(POLCLASS_UOBJECT))
			return InvalidFormatException("The param must be an ObjRef!");
		endif

		return Distance(this._objRef, otherObj);
	endfunction

	/**
	 * Reproduz uma animação no objeto móvel.
	 *
	 * @param this          Referência ao objeto wrapper do móvel
	 * @param animID        ID da animação a ser reproduzida
	 * @param framecount    Número de frames da animação (padrão: 5)
	 * @param repeatcount   Número de vezes que a animação será repetida (padrão: 1)
	 * @param backward      Direção da animação - ACTION_DIR_FORWARD ou ACTION_DIR_BACKWARD (padrão: ACTION_DIR_FORWARD)
	 * @param repeatflag    Flag de repetição - ACTION_NOREPEAT ou ACTION_REPEAT (padrão: ACTION_NOREPEAT)
	 * @param delay         Atraso entre repetições em frames (padrão: 1)
	 *
	 * @return Retorna o resultado da execução da ação PerformAction
	 */
	function PlayAnimation(this, animID, framecount := 5, repeatcount := 1, backward := ACTION_DIR_FORWARD, repeatflag := ACTION_NOREPEAT, delay := 1)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return PerformAction(this._objRef, Hex(animID), repeatcount, framecount, backward, repeatflag, delay);
	endfunction

	function PlayMusic(this, musicID)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return PlayMusic(this._objRef, Hex(musicID));
	endfunction

	function Update(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return RecalcVitals(this._objRef);
	endfunction

	function GetTarget(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return Target(this._objRef);
	endfunction

	function ConsumeHP(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return AP_ConsumeVital(this._objRef, HITS, CDbl(value));
	endfunction

	function ConsumeStamina(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return AP_ConsumeVital(this._objRef, STAMINA, CDbl(value));
	endfunction

	function ConsumeMana(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return AP_ConsumeVital(this._objRef, MANA, CDbl(value));
	endfunction

	function RestoreHP(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return HealHits(this._objRef, CDbl(value));
	endfunction

	function RestoreStamina(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return HealStamina(this._objRef, CDbl(value));
	endfunction

	function RestoreMana(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return HealMana(this._objRef, CDbl(value));
	endfunction

	function Dispose(this)
		this._objRef := 0;
		this._name := "";
	endfunction

	function Equals(this, byRef otherMobileWrapper)
		if (!(otherMobileWrapper is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		return this.GetObjRef() == otherMobileWrapper.GetObjRef() &&
			   this.GetSerial() == otherMobileWrapper.GetSerial();
	endfunction

	function ToString(this)
		return $"Mobile {this._name}: " +
			$"Serial: {Hex(this._serial)}, " +
			$"ObjType: {Hex(this._objType)}, " +
			$"Graphic: {this._graphic} ({Hex(this._graphic)}), " +
			$"Color: {this._color} ({Hex(this._color)}) ";
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
