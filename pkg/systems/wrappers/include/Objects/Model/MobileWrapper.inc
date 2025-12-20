use uo;

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
include "include/epicNPCUtils";

class MobileWrapper(UObjectWrapper, IMobileWrapper)
	var _backpack; // ContainerWrapper class
	var _race := "";
	var _culture := "";
	var _religion := "";
	var _combatTraits; // CombatTraits class

	function MobileWrapper(this, byRef mobile)
		Super(mobile);

		if (TypeOfInt(mobile) == OT_MOBILEREF)
			if (mobile.isA(POLCLASS_NPC))
				this._cfgFile := CFGFile("epicAI", "npcdesc");
				this._template := this._cfgFile.SetElementName(NPC_ParseTemplateName(mobile.npctemplate).template).GetElementValue();
				this._category := this._template.Category;

				// Variáveis que indicarão que é um NPC ou não encontrou template
				var err := IsNPCException();
				this._race := err;
				this._culture := err;
				this._religion := err;
				this._combatTraits := err;
			else // Se entrar aqui, é player
				this._race := this.GetProperty(MOBILE_PROPNAME::CHARDATA).race;
				this._culture := this.GetProperty(MOBILE_PROPNAME::CHARDATA).culture;
				this._religion := this.GetProperty(MOBILE_PROPNAME::CHARDATA).religion;

				// Variáveis que indicarão que é um player
				var err := IsPlayerException();
				this._template := err;
				this._objType := err;
			endif

			this._name := this._objRef.name;
			this._backpack := ContainerWrapper(this._objRef.backpack);
		else
			this._cfgFile := CFGFile("epicAI", "npcdesc");
			this._template := this._cfgFile.SetElementName(mobile).GetElementValue();

			// Variáveis que indicarão que é apenas um template ou algum erro ocorreu
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Mobile ObjRef or a valid Template!");
				this._objType := err;
				this._name := err;
				this._graphic := err;
				this._color := err;
			else
				err := IsTemplateException();
				this._name := this._template.desc;
				this._graphic := this._template.graphic;
				this._color := this._template.color;
				this._category := this._template.Category;
			endif
			
			this._objRef := err;
			this._serial := err;

			this._backpack := err;
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
	
	function GetBackpack(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._backpack;
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

	function GetLanguageOptions(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this.GetProperty(MOBILE_PROPNAME::LANGUAGE);
	endfunction

	/**
	 * Obtém o gênero do mobile (jogador ou NPC).
	 * 
	 * Este método verifica primeiro se existe uma referência de objeto válida.
	 * Caso não exista, tenta obter o gênero através do template.
	 * Se nenhum dos dois existir, retorna um erro.
	 * 
	 * @param this Instância do MobileWrapper
	 * @return MOBILE_GENDER Retorna uma das seguintes constantes:
	 *         - MOBILE_GENDER::MALE (0) para masculino
	 *         - MOBILE_GENDER::FEMALE (1) para feminino
	 *         - MOBILE_GENDER::UNDEFINED para valores não reconhecidos
	 * @throws error Se não houver nem referência de objeto nem template válido
	 */
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

	/**
	 * Obtém o tier do template associado ao objeto (NPC).
	 * 
	 * @return Integer representando o tier do template, ou um objeto Exception caso o template seja inválido
	 * 
	 * @note Se o template for uma Exception, retorna a própria Exception ao invés do tier
	 */
	function GetTier(this)
		if (this._template is @Exception)
			return this._template;
		endif
		
		return CInt(this._template.Tier);
	endfunction

	function GetCriticalChance(this)
		return CInt(this.GetCriticalChanceBase() + this.GetCriticalChanceMod());
	endfunction
	
	function GetCriticalChanceBase(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return CInt(this.GetProperty("CriticalChance"));
	endfunction

	function GetCriticalChanceMod(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return CInt(this.GetProperty("#CriticalChance"));
	endfunction

	function GetTemplateName(this)
		if (this._template is @Exception)
			return this._template;
		endif

		return this._cfgFile.GetElementName();
	endfunction

	function GetNPCDamage(this)
		if (this._template is @Exception)
			return this._template;
		endif
		
		return this._template.Damage;
	endfunction

	function GetBodyResist(this)
		if (this._template is @Exception)
			return this._template;
		endif
		
		var phys, elem;
		phys := SplitWords(this._template.EpicFisResists);
		elem := SplitWords(this._template.EpicElemResists);

		return ArmorResist(
			GetResistValueByTier(phys[1]), GetResistValueByTier(phys[2]), GetResistValueByTier(phys[3]),
			GetResistValueByTier(elem[1]), GetResistValueByTier(elem[2]), GetResistValueByTier(elem[3]), GetResistValueByTier(elem[4])
		);
	endfunction

	/**
	 * Obtém o equipamento equipado em um slot específico do personagem.
	 *
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
		
		var equipsInSlot := array{};

		// Feito assim pra garantir que a variável pertença ao ENUM, e também caso necessite customizar o retorno de algum slot
		if (equipSlot == WEAPON)
			var weapon := WeaponWrapper(this._objRef.weapon);

			if (this.IsNPC())
				weapon.SetMainDamage(DamageDice(
					Dice(GetDamageByTier(SplitWords(this._template.EpicAttackDamage)[1])),
					Dice(GetDamageByTier(SplitWords(this._template.EpicAttackDamage)[2])),
					Dice(GetDamageByTier(SplitWords(this._template.EpicAttackDamage)[3]))
				));
				weapon.SetDamageMod(Damage(
					GetDamageByTier(SplitWords(this._template.EpicElemDamage)[1]),
					GetDamageByTier(SplitWords(this._template.EpicElemDamage)[2]),
					GetDamageByTier(SplitWords(this._template.EpicElemDamage)[3]),
					GetDamageByTier(SplitWords(this._template.EpicElemDamage)[4])
				));


			endif

			return weapon;
		elseif (equipSlot == SHIELD)
			return ArmorWrapper(this._objRef.shield);
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

	/**
	 * Obtém o equipamento de uma camada específica do personagem e o encapsula no wrapper apropriado.
	 *
	 * @param this Referência ao objeto Mobile
	 * @param layerEnum Enumeração da camada de equipamento a ser consultada
	 * @param itemType Tipo do item (ITEM_TYPE::WEAPON, ITEM_TYPE::ARMOR ou ITEM_TYPE::CONTAINER)
	 *
	 * @return Retorna o equipamento encapsulado no wrapper correspondente ao tipo especificado
	 *         (WeaponWrapper, ArmorWrapper ou ContainerWrapper).
	 *         Retorna Exception se o objeto de referência for inválido, ou InvalidFormatException se
	 *         o itemType não for válido.
	 */
	function GetEquipByLayer(this, layerEnum, itemType)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		var equip := GetEquipmentByLayer(this._objRef, layerEnum);
		if (itemType == ITEM_TYPE::WEAPON)
			equip := WeaponWrapper(equip);
		elseif (itemType == ITEM_TYPE::ARMOR)
			equip := ArmorWrapper(equip);
		elseif (itemType == ITEM_TYPE::CONTAINER)
			equip := ContainerWrapper(equip);
		else
			return InvalidFormatException("The itemType param must be a valid ITEM_TYPE enum value!");
		endif

		return equip;
	endfunction

	function GetSkill(this, skillName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this.GetSkillBase(skillName) + this.GetSkillMod(skillName);
	endfunction

	function GetSkillBase(this, skillName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return AP_GetTrueSkill(this._objRef, skillName);
	endfunction

	function GetSkillMod(this, skillName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return AP_GetSkillMod(this._objRef, skillName);
	endfunction

	function GetAbility(this, abilityName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		// TODO: Implementar retorno de CombatAbility
	endfunction

	function GetCombatTraits(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		var propData := this.GetProperty(MOBILE_PROPNAME::COMBAT_TRAITS);
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
		this.SetProperty(MOBILE_PROPNAME::COMBAT_TRAITS, this._combatTraits.Serialize());

		return this;
	endfunction

	function SetLanguageOptions(this, langOptions)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		return this.SetProperty(MOBILE_PROPNAME::LANGUAGE, langOptions);
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
		if (this._objRef is @Exception)
			return this._objRef;
		endif

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
		if (this._objRef is @Exception)
			return this._objRef;
		endif

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
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return TemHabilidade(this._objRef, abilityName);
	endfunction

	function IsNPC(this)
		return (this._template && !(this._template is @Exception));
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

		// TODO: Isso aqui vai dar errado
		return this.GetEquipByLayer(LAYER_MOUNT) == true;
	endfunction

	function IsConnected(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.connected;
	endfunction

	function IsHidden(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return this._objRef.hidden;
	endfunction

	/*
	 * Calcula a distância entre este objeto e outro objeto.
	 *
	 * @param otherObj  Referência ao objeto de destino (deve ser um POLCLASS_UOBJECT)
	 *
	 * @return Integer  Distância entre os dois objetos
	 * @return Error    Retorna um erro se otherObj não for uma referência de objeto válida
	 *
	 * @example
	 *   var distancia := mobile.DistanceTo(target);
	 */
	function DistanceTo(this, otherObj)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(otherObj is @UObjectWrapper))
			return InvalidFormatException("The param must be an Object Wrapper!");
		endif

		return Distance(this._objRef, otherObj.GetObjRef());
	endfunction

	/**
	 * Reproduz uma animação no objeto móvel.
	 *
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

		return PerformAction(this._objRef, animID, repeatcount, framecount, backward, repeatflag, delay);
	endfunction

	function PlayMusic(this, musicID)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		return PlayMusic(this._objRef, musicID);
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

	/**
	 * Executa um ataque contra um alvo móvel.
	 * 
	 * @param targetWrapper O wrapper do objeto alvo que será atacado (deve ser do tipo MobileWrapper)
	 * 
	 * @return Retorna o resultado do ataque executado pelo objeto de referência, ou uma Exception se:
	 *         - O objeto de referência interno for uma Exception
	 *         - O parâmetro targetWrapper não for um objeto do tipo MobileWrapper (retorna InvalidFormatException)
	 */
	function Attack(this, byRef targetWrapper)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(targetWrapper is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		return this._objRef.attack_once(targetWrapper.GetObjRef());
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
			$"Serial: {this._serial}, " +
			$"ObjType: {this._objType}, " +
			$"Graphic: {this._graphic} ({this._graphic}), " +
			$"Color: {this._color} ({this._color}), " +
			$"Str: {this.GetStr()}, " +
			$"Dex: {this.GetDex()}, " +
			$"Int: {this.GetInt()}";
	endfunction

	// ============================================================================
	// Entropy System Methods
	// ============================================================================

	/**
	 * Obtém o valor atual de entropia do mobile.
	 *
	 * @return Integer Valor de entropia (0 se não definido)
	 * @return Exception Se o objeto de referência for inválido
	 */
	function GetEntropy(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		var entropy := this.GetProperty(MOBILE_PROPNAME::ENTROPY);
		if (!entropy)
			return 0;
		endif
		return CInt(entropy);
	endfunction

	/**
	 * Define o valor de entropia do mobile, respeitando os limites mínimo e máximo.
	 *
	 * @param value Integer Novo valor de entropia
	 * @return MobileWrapper Retorna a própria instância para encadeamento de métodos
	 * @return Exception Se o objeto de referência for inválido
	 */
	function SetEntropy(this, value)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (value > ENTROPY_LIMITS::MAX)
			value := ENTROPY_LIMITS::MAX;
		elseif (value < ENTROPY_LIMITS::MIN)
			value := ENTROPY_LIMITS::MIN;
		endif

		this.SetProperty(MOBILE_PROPNAME::ENTROPY, CInt(value));
		return this;
	endfunction

	/**
	 * Atualiza a entropia do mobile.
	 *
	 * IMPORTANT: This method only updates the individual mobile's entropy.
	 * The caller is responsible for updating global entropy and handling any
	 * game-specific bonuses or effects.
	 *
	 * @param amt Integer Quantidade a ser adicionada ou subtraída da entropia
	 * @return MobileWrapper Retorna a própria instância para encadeamento de métodos
	 * @return Exception Se o objeto de referência for inválido
	 */
	function UpdateEntropy(this, amt)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		var current := this.GetEntropy();
		this.SetEntropy(current + amt);
		return this;
	endfunction
endclass

enum class MOBILE_GENDER
	MALE := "Male",
	FEMALE := "Female",
	UNDEFINED := "Undefined"
endenum

enum class MOBILE_PROPNAME
	CHARDATA := "chardata",
	COMBAT_TRAITS := "combattraits",
	LANGUAGE := "langdata",
	ENTROPY := "entropy"
endenum

enum class ENTROPY_LIMITS
	MAX := 20000,
	MIN := -20000
endenum

enum class MOBILE_CATEGORY
	ANGEL := "Angel",
	ANIMAL := "Animal",
	BEAST := "Beast",
	CONSTRUCT := "Construct",
	DEMONS := "Demons",
	DRAGON := "Dragon",
	GIANTANIMAL := "GiantAnimal",
	GIANTS := "Giants",
	GOBLIN := "Goblin",
	MAGIC := "Magic",
	MONSTROSITY := "Monstrosity",
	MOUNT := "Mount",
	SEA := "Sea",
	SNOW := "Snow",
	UNDEAD := "Undead",
	UNDERDARK := "Underdark"
endenum
