use uo;

// Internal
include "IMobileWrapper";
include "UObjectWrapper";
include "ArmorWrapper";
include "WeaponWrapper";

// External
include ":attributes:skills";
include ":merchants:cambio";
include ":attributes:vitals";
include ":attributes:stats";
include ":combat:armorZones";
include ":combat:Model/CombatTraits";
include "include/epicNPCUtils";
include ":achievements:utils/datafile";

class MobileWrapper(UObjectWrapper, IMobileWrapper)
	var BackpackSerial := 0x0; 
	var Race := "";
	var Culture := "";
	var Religion := "";
	var CombatTraitsVar; // CombatTraits class

	function MobileWrapper(this, byRef mobile)
		Super(mobile);

		if (TypeOfInt(mobile) == OT_MOBILEREF || TypeOfInt(mobile) == OT_OFFLINEMOBILEREF)
			if (mobile.isA(POLCLASS_NPC))
				this.CfgFile := CFGFile("epicAI", "npcdesc");
				this.Template := this.GetCfgFile().SetElementName(NPC_ParseTemplateName(mobile.npctemplate).template).GetElement();
				this.Category := this.GetTemplate().Category;

				// Variáveis que indicarão que é um NPC ou não encontrou template
				var err := IsNPCException();
				this.Race := err;
				this.Culture := err;
				this.Religion := err;
				this.CombatTraitsVar := err;
			else // Se entrar aqui, é player
				this.Race := this.GetProperty(MOBILE_PROPNAME::CHARDATA).race;
				this.Culture := this.GetProperty(MOBILE_PROPNAME::CHARDATA).culture;
				this.Religion := this.GetProperty(MOBILE_PROPNAME::CHARDATA).religion;

				// Variáveis que indicarão que é um player
				var err := IsPlayerException();
				this.Template := err;
				this.ObjType := err;
			endif

			this.Name := this.GetObjRef().name;
			this.BackpackSerial := this.GetObjRef().backpack.serial;
		else
			this.CfgFile := CFGFile("epicAI", "npcdesc");
			this.Template := this.GetCfgFile().SetElementName(mobile).GetElement();

			// Variáveis que indicarão que é apenas um template ou algum erro ocorreu
			var err;
			if (!this.Template)
				err := InvalidFormatException("The param must be a Mobile ObjRef or a valid Template!");
				this.ObjType := err;
				this.Name := err;
				this.Graphic := err;
				this.Color := err;
			else
				err := IsTemplateException();
				this.Name := this.GetTemplate().desc;
				this.Graphic := this.GetTemplate().graphic;
				this.Color := this.GetTemplate().color;
				this.Category := this.GetTemplate().Category;
			endif
			
			this.ObjRef := err;
			this.Serial := err;

			this.BackpackSerial := err;
			this.Race := err;
			this.Culture := err;
			this.Religion := err;
			this.CombatTraitsVar := err;
		endif
	endfunction

	function GetAccountName(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().acctname;
	endfunction
	
	function GetBackpack(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ContainerWrapper(this.BackpackSerial);
	endfunction
	
	function GetRace(this)
		if (this.Race is @Exception)
			return this.Race;
		endif

		return this.Race;
	endfunction

	function GetCulture(this)
		if (this.Culture is @Exception)
			return this.Culture;
		endif

		return this.Culture;
	endfunction

	function GetReligion(this)
		if (this.Religion is @Exception)
			return this.Religion;
		endif

		return this.Religion;
	endfunction
	
	function GetStr(this)
		return this.GetStrBase() + this.GetStrMod();
	endfunction

	function GetStrBase(this)
		return AP_GetTrueStat(this.GetObjRef(), STRENGTH);
	endfunction

	function GetStrMod(this)
		return AP_GetStatMod(this.GetObjRef(), STRENGTH);
	endfunction

	function GetDex(this)
		return this.GetDexBase() + this.GetDexMod();
	endfunction

	function GetDexBase(this)
		return AP_GetTrueStat(this.GetObjRef(), DEXTERITY);
	endfunction

	function GetDexMod(this)
		return AP_GetStatMod(this.GetObjRef(), DEXTERITY);
	endfunction

	function GetInt(this)
		return this.GetIntBase() + this.GetIntMod();
	endfunction

	function GetIntBase(this)
		return AP_GetTrueStat(this.GetObjRef(), INTELLIGENCE);
	endfunction

	function GetIntMod(this)
		return AP_GetStatMod(this.GetObjRef(), INTELLIGENCE);
	endfunction

	function GetFortitude(this)
		return AP_GetStat(this.GetObjRef(), FORTITUDE);
	endfunction

	function GetFortitudeBase(this)
		return AP_GetTrueStat(this.GetObjRef(), FORTITUDE);
	endfunction

	function GetFortitudeMod(this)
		return AP_GetStatMod(this.GetObjRef(), FORTITUDE);
	endfunction

	function GetReflexes(this)
		return AP_GetStat(this.GetObjRef(), REFLEXES);
	endfunction

	function GetReflexesBase(this)
		return AP_GetTrueStat(this.GetObjRef(), REFLEXES);
	endfunction

	function GetReflexesMod(this)
		return AP_GetStatMod(this.GetObjRef(), REFLEXES);
	endfunction

	function GetWill(this)
		return AP_GetStat(this.GetObjRef(), WILL);
	endfunction

	function GetWillBase(this)
		return AP_GetTrueStat(this.GetObjRef(), WILL);
	endfunction

	function GetWillMod(this)
		return AP_GetStatMod(this.GetObjRef(), WILL);
	endfunction

	function GetCurrentHP(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetVital(this.GetObjRef(), HITS);
	endfunction

	function GetCurrentDp(this)
		return AP_GetVital(this.GetObjRef(), DEATH_POINTS);
	endfunction

	function GetCurrentStamina(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetVital(this.GetObjRef(), STAMINA);
	endfunction

	function GetCurrentMana(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetVital(this.GetObjRef(), MANA);
	endfunction

	function GetCurrentHunger(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetVital(this.GetObjRef(), HUNGER);
	endfunction

	function GetMaxHP(this)
		return AP_GetVitalMaximumValue(this.GetObjRef(), HITS);
	endfunction

	function GetMaxStamina(this)
		return AP_GetVitalMaximumValue(this.GetObjRef(), STAMINA);
	endfunction

	function GetMaxMana(this)
		return AP_GetVitalMaximumValue(this.GetObjRef(), MANA);
	endfunction

	function GetMaxHunger(this)
		return AP_GetVitalMaximumValue(this.GetObjRef(), HUNGER);
	endfunction

	function GetMaxDp(this)
		return AP_GetVitalMaximumValue(this.GetObjRef(), DEATH_POINTS);
	endfunction

	function GetRegenHP(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return AP_GetVitalRegenRate(this.GetObjRef(), HITS);
	endfunction

	function GetRegenStamina(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return AP_GetVitalRegenRate(this.GetObjRef(), STAMINA);
	endfunction

	function GetRegenMana(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return AP_GetVitalRegenRate(this.GetObjRef(), MANA);
	endfunction

	function GetRegenHunger(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return AP_GetVitalRegenRate(this.GetObjRef(), HUNGER);
	endfunction

	function GetLanguageOptions(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
		if (this.GetObjRef() is @Exception)
			if (this.GetTemplate() is @Exception)
				return error{errortext := "This is not a Player, NPC or valid template!"};
			else
				if (this.GetTemplate().Gender == 0)
					return MOBILE_GENDER::MALE;
				elseif (this.GetTemplate().Gender == 1)
					return MOBILE_GENDER::FEMALE;
				else
					return MOBILE_GENDER::UNDEFINED;
				endif
			endif
		endif
		
		if (this.GetObjRef().gender == 0)
			return MOBILE_GENDER::MALE;
		elseif (this.GetObjRef().gender == 1)
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
		if (this.GetTemplate() is @Exception)
			return this.GetTemplate();
		endif
		
		return CInt(this.GetTemplate().Tier);
	endfunction

	function GetCriticalChance(this)
		return CInt(this.GetCriticalChanceBase() + this.GetCriticalChanceMod());
	endfunction
	
	function GetCriticalChanceBase(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return CInt(this.GetProperty("CriticalChance"));
	endfunction

	function GetCriticalChanceMod(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return CInt(this.GetProperty("#CriticalChance"));
	endfunction

	function GetTemplateName(this)
		if (this.GetTemplate() is @Exception)
			return this.GetTemplate();
		endif

		return this.GetCfgFile().GetElementName();
	endfunction

	function GetBodyResist(this)
		if (this.GetTemplate() is @Exception)
			return this.GetTemplate();
		endif
		
		var phys, elem;
		phys := SplitWords(this.GetTemplate().EpicFisResists);
		elem := SplitWords(this.GetTemplate().EpicElemResists);

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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var equipsInSlot := array{};

		// Feito assim pra garantir que a variável pertença ao ENUM, e também caso necessite customizar o retorno de algum slot
		if (equipSlot == WEAPON)
			var weapon := WeaponWrapper(this.GetObjRef().weapon);

			if (GetObjProperty(this._objRef, "metamorfose") != error)
				var dicedmgProp := GetObjProperty(this._objRef, "dicedmg");
				if (dicedmgProp != error)
					// Use dicedmg from metamorfose transformation
					weapon.SetMainDamage(
						Dice(dicedmgProp[1]), 
						Dice(dicedmgProp[2]), 
						Dice(dicedmgProp[3])
					);
				endif
			elseif (this.IsNPC())
				// Use template damage for normal NPCs
				weapon.SetMainDamage(
					Dice(GetDamageByTier(SplitWords(this.GetTemplate().EpicAttackDamage)[1])),
					Dice(GetDamageByTier(SplitWords(this.GetTemplate().EpicAttackDamage)[2])),
					Dice(GetDamageByTier(SplitWords(this.GetTemplate().EpicAttackDamage)[3]))
				);
				weapon.SetDamageMod(Damage(
					GetDamageByTier(SplitWords(this.GetTemplate().EpicElemDamage)[1]),
					GetDamageByTier(SplitWords(this.GetTemplate().EpicElemDamage)[2]),
					GetDamageByTier(SplitWords(this.GetTemplate().EpicElemDamage)[3]),
					GetDamageByTier(SplitWords(this.GetTemplate().EpicElemDamage)[4])
				));
			endif
			// If neither (normal player with equipped weapon), weapon damage is already set from item

			return weapon;
		elseif (equipSlot == SHIELD)
			return ArmorWrapper(this.GetObjRef().shield);
		elseif (equipSlot == HEAD)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), HEAD);
		elseif (equipSlot == NECK)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), NECK);
		elseif (equipSlot == ARMS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), ARMS);
		elseif (equipSlot == BODY)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), BODY);
		elseif (equipSlot == HANDS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), HANDS);
		elseif (equipSlot == LEGS)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), LEGS);
		elseif (equipSlot == FEET)
			equipsInSlot := CS_GetEquipmentInArmorZone(this.GetObjRef(), FEET);
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
	 *
	 * @return Retorna o equipamento encapsulado no wrapper correspondente ao tipo especificado
	 *         (WeaponWrapper, ArmorWrapper ou ContainerWrapper).
	 *         Retorna Exception se o objeto de referência for inválido, ou InvalidFormatException
	 */
	function GetEquipByLayer(this, layerEnum)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		var equip := GetEquipmentByLayer(this.GetObjRef(), layerEnum);
		if (equip.IsA(POLCLASS_WEAPON))
			equip := WeaponWrapper(equip);
		elseif (equip.IsA(POLCLASS_ARMOR))
			equip := ArmorWrapper(equip);
		elseif (equip.IsA(POLCLASS_CONTAINER))
			equip := ContainerWrapper(equip);
		endif

		// TODO: Se retornou o item, tem que ver se ele precisa de Wrapper pra adicionar aqui em cima.
		return equip;
	endfunction

	function GetSkill(this, skillName)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetSkillBase(skillName) + this.GetSkillMod(skillName);
	endfunction

	function GetSkillBase(this, skillName)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetTrueSkill(this.GetObjRef(), skillName);
	endfunction

	function GetSkillMod(this, skillName)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_GetSkillMod(this.GetObjRef(), skillName);
	endfunction

	function GetAbility(this, abilityName)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		// TODO: Implementar retorno de CombatAbility
	endfunction

	function GetCombatTraits(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var propData := this.GetProperty(MOBILE_PROPNAME::COMBAT_TRAITS);
		if (!propData)
			propData := CombatTraits().Serialize();
		endif

		this.CombatTraitsVar := CombatTraits().Deserialize(propData);

		return this.CombatTraitsVar;
	endfunction

	/*
	 * Calcula o dano mínimo principal baseado em critérios especificados.
	 *
	 * Esta função calcula um valor de dano mínimo considerando diferentes
	 * atributos do mobile, como força, destreza e habilidade de táticas.
	 *
	 * @param this - Referência ao objeto MobileWrapper atual
	 * @param criteria - Lista variável de critérios para cálculo (STRENGTH, DEXTERITY, TACTICS)
	 *
	 * @returns Integer - O valor calculado do dano mínimo baseado nos critérios fornecidos
	 *
	 * Comportamento:
	 * - Se STRENGTH estiver nos critérios:
	 *   - Se DEXTERITY também estiver presente, usa a média entre força e destreza
	 *   - Caso contrário, usa apenas o valor de força
	 * - Se TACTICS estiver nos critérios, adiciona o valor da habilidade de táticas dividido por 20
	 *
	 * Nota: O divisor padrão utilizado é 20 para normalizar os valores dos atributos
	 */
	function GetMainDamageModifierBy(this, criteria...)
		var divisor := 20;
		var value := 0;

		if (STRENGTH in criteria && DEXTERITY in criteria)
			// Both stats - use average
			var statValue := CInt((this.GetStr() + this.GetDex()) / 2);
			value += CInt(statValue / divisor);
		elseif (STRENGTH in criteria)
			// Only strength
			value += CInt(this.GetStr() / divisor);
		elseif (DEXTERITY in criteria)
			// Only dexterity
			value += CInt(this.GetDex() / divisor);
		endif

		if (TACTICS in criteria )
			value += CInt(this.GetSkill(TACTICS) / divisor);
		endif

		return CInt(value);
	endfunction

	function GetOffDamageModifierBy(this, criteria...)
		var offHandDivisor := 2;
		
		return CInt(this.GetMainDamageModifierBy(criteria...) / offHandDivisor);
	endfunction

	function SetSkillBase(this, skillName, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		AP_SetTrueSkill(this.GetObjRef(), skillName, CDbl(value));
		return this;
	endfunction

	function SetSkillMod(this, skillName, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		AP_SetSkillMod(this.GetObjRef(), skillName, CDbl(value));
		return this;
	endfunction

	function GetAchievement(this, who, type, key, name)
		return LoadAchievement(type, who, key, name);
	endfunction

	function GetListAchievement(this, who)
		return LoadListAchievement(who);
	endfunction

	function SetAchievement(this, who, type, elem, achievement)
		return SaveAchievement(type, elem, achievement);
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
		
		this.CombatTraitsVar := combatTrait;
		this.SetProperty(MOBILE_PROPNAME::COMBAT_TRAITS, this.CombatTraitsVar.Serialize());

		return this;
	endfunction

	function SetLanguageOptions(this, langOptions)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return InvalidFormatException("The param must be an ArmorWrapper or WeaponWrapper object!");
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.ApplyOn(this.GetObjRef());
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(itemWrapped is @ArmorWrapper) && !(itemWrapped is @WeaponWrapper))
			return InvalidFormatException("The param must be an ArmorWrapper or WeaponWrapper object!");
		endif

		foreach mod in (itemWrapped.GetModifiers())
			mod.RemoveFrom(this.GetObjRef());
			SleepMS(2);
		endforeach

		return this;
	endfunction

	function HasAbility(this, abilityName)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return TemHabilidade(this.GetObjRef(), abilityName);
	endfunction

	function IsNPC(this)
		return (this.Template && !(this.Template is @Exception));
	endfunction

	function IsDead(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		return this.GetObjRef().dead == true;
	endfunction

	function IsWarMode(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().warmode == true;
	endfunction

	function IsMounted(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		// TODO: Isso aqui vai dar errado
		return this.GetEquipByLayer(LAYER_MOUNT) != error;
	endfunction

	function IsConnected(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().connected;
	endfunction

	function IsHidden(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().hidden;
	endfunction

	function GetGold(this)
		return ContaCapital(this._objRef).valorTotal;
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(otherObj is @UObjectWrapper))
			return InvalidFormatException("The param must be an Object Wrapper!");
		endif

		return Distance(this.GetObjRef(), otherObj.GetObjRef());
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return PerformAction(this.GetObjRef(), animID, repeatcount, framecount, backward, repeatflag, delay);
	endfunction

	function PlayMusic(this, musicID)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return PlayMusic(this.GetObjRef(), musicID);
	endfunction

	function Update(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return RecalcVitals(this.GetObjRef());
	endfunction

	function GetTarget(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return Target(this.GetObjRef());
	endfunction

	function ConsumeHP(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_ConsumeVital(this.GetObjRef(), HITS, CDbl(value));
	endfunction

	function ConsumeStamina(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_ConsumeVital(this.GetObjRef(), STAMINA, CDbl(value));
	endfunction

	function ConsumeMana(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return AP_ConsumeVital(this.GetObjRef(), MANA, CDbl(value));
	endfunction

	function RestoreHP(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return HealHits(this.GetObjRef(), CDbl(value));
	endfunction

	function RestoreStamina(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return HealStamina(this.GetObjRef(), CDbl(value));
	endfunction

	function RestoreMana(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return HealMana(this.GetObjRef(), CDbl(value));
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(targetWrapper is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		return this.GetObjRef().attack_once(targetWrapper.GetObjRef());
	endfunction

	function Equals(this, byRef otherMobileWrapper)
		if (!(otherMobileWrapper is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		return this.GetObjRef() == otherMobileWrapper.GetObjRef() &&
			   this.GetSerial() == otherMobileWrapper.GetSerial();
	endfunction

	function ToString(this)
		return $"Mobile {this.GetName()}: " +
			$"Serial: {this.GetSerial()}, " +
			$"ObjType: {this.GetObjType()}, " +
			$"Graphic: {this.GetGraphic()} ({this.GetGraphic()}), " +
			$"Color: {this.GetColor()} ({this.GetColor()}), " +
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
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
