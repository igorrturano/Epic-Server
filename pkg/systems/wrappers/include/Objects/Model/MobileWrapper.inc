use uo;
use os;

// Internal
include "IMobileWrapper";
include "UObjectWrapper";
include "ArmorWrapper";
include "WeaponWrapper";

// External
include ":attributes:attributes";
include ":attributes:skills";
include ":merchants:cambio";
include ":attributes:vitals";
include ":attributes:stats";
include ":combat:armorZones";
include ":combat:Model/CombatTraits";
include "include/epicNPCUtils";
include "include/NPCUtil";
include "include/utils";
include "include/facings";
include ":achievements:utils/datafile";
include ":serialization:StructSerializer";
include ":serialization:DictionarySerializer";
include ":abilities:controllers/AbilityMonitor";
include ":hotbar:include/cooldown_packet";
include ":mastery:models/Mastery";

class MobileWrapper(UObjectWrapper, IMobileWrapper)
	var BackpackSerial := 0x0; 
	var Race := "";
	var Culture := "";
	var Religion := "";
	var CombatTraitsVar; // CombatTraits class
	var Abilities; // Dictionary<ABILITY_ID, Integer>
	var ActiveAbilities; // Dictionary<ACTIVATION_MOMENT, Dictionary<ABILITY_ID, INTEGER>> // O valor dentro do 2º é quantas cargas ativas existem dessa habilidade
	var _cachedWeapon;       // WeaponWrapper cacheado pra evitar recriação a cada GetEquipBySlot(WEAPON)
	var _cachedWeaponSerial; // Serial da arma que gerou o cache
	var Environment;      // NPC-only: spawn environment (ex-Category for NPCs)

	function MobileWrapper(this, byRef mobile)
		Super(mobile);

		if (TypeOfInt(mobile) == OT_MOBILEREF || TypeOfInt(mobile) == OT_OFFLINEMOBILEREF)
			if (mobile.isA(POLCLASS_NPC))
				this.CfgFile := CFGFile("epicAI", "npcdesc");
				this.Template := this.GetCfgFile().SetElementName(NPC_ParseTemplateName(mobile.npctemplate).template).GetElement();
				this.Environment := this.GetTemplate().Environment;

				// Variáveis que indicarão que é um NPC ou não encontrou template
				var err := IsNPCException();
				this.Race := err;
				this.Culture := err;
				this.Religion := err;
				this.CombatTraitsVar := err;
			else // Se entrar aqui, é player
				this.Race := this.GetProperty(MOBILE_PROPNAME::CHARDATA).raca;
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
				this.Environment := this.GetTemplate().Environment;
			endif

			this.ObjRef := err;
			this.Serial := err;

			this.BackpackSerial := err;
			this.Race := err;
			this.Culture := err;
			this.Religion := err;
			this.CombatTraitsVar := err;
			this.Abilities := err;
			this.ActiveAbilities := err;
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

	function GetEnvironment(this)
		return this.Environment;
	endfunction

	function GetCulture(this)
		if (this.Culture is @Exception)
			return this.Culture;
		endif

		return this.Culture;
	endfunction

	function GetReligion(this)
		if (this.Religion is @Exception)
			// NPC path — lazy-read from "npc_religion" ObjProperty set by makeReligion at init.
			// Non-cleric / non-arqueiro-arcano NPCs have no religion; property is absent and
			// the IsNPCException is returned (preserves legacy behavior for those).
			if (this.IsNPC())
				var stored := GetObjProperty(this.GetObjRef(), "npc_religion");
				if (stored)
					this.Religion := stored;
					return stored;
				endif
			endif
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

	function GetVicioStr(this)
		return AP_GetVital(this.GetObjRef(), ADDICTION_STR);
	endfunction
	function GetVicioDex(this)
		return AP_GetVital(this.GetObjRef(), ADDICTION_DEX);
	endfunction
	function GetVicioInt(this)
		return AP_GetVital(this.GetObjRef(), ADDICTION_INT);
	endfunction

	function GetVisibleVicioStr(this)
		return VICIO_MAX_POINTS - cint(this.GetVicioStr());
	endfunction
	function GetVisibleVicioDex(this)
		return VICIO_MAX_POINTS - cint(this.GetVicioDex());
	endfunction
	function GetVisibleVicioInt(this)
		return VICIO_MAX_POINTS - cint(this.GetVicioInt());
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

	function SetCriticalChanceMod(this, value)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.SetProperty("#CriticalChance", CInt(value));
		return this;
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
	 * Returns the spell resist bonuses active on this mobile.
	 * Spell resist is a virtual armor layer applied by magic (barkskin, protection, etc.)
	 * and is read by both DamageController and sendStats.
	 *
	 * @return ArmorResist with spell resist values (all zeros if no active spell resist)
	 */
	function GetSpellResist(this)
		var sr := this.GetProperty("#SpellResist");
		if (!sr || sr is @Exception)
			return ArmorResist();
		endif
		return ArmorResist(
			CInt(sr.slash), CInt(sr.piercing), CInt(sr.blunt),
			CInt(sr.fire), CInt(sr.cold), CInt(sr.energy), CInt(sr.poison)
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
			var currentWeapon := this.GetObjRef().weapon;
			var currentSerial := currentWeapon ? currentWeapon.serial : 0;

			if (this._cachedWeapon && this._cachedWeaponSerial == currentSerial)
				return this._cachedWeapon;
			endif

			var weapon := WeaponWrapper(currentWeapon);

			if (GetObjProperty(this._objRef, "metamorfose") != error)
				var dicedmgProp := GetObjProperty(this._objRef, "dicedmg");
				if (dicedmgProp != error)
					weapon.SetMainDamage(
						Dice(dicedmgProp[1]),
						Dice(dicedmgProp[2]),
						Dice(dicedmgProp[3])
					);
				endif
			elseif (this.IsNPC())
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

			this._cachedWeapon := weapon;
			this._cachedWeaponSerial := currentSerial;
			return weapon;
		elseif (equipSlot == SHIELD)
			if (!this.GetObjRef().shield) return uninit; endif
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

		if (TACTICS in criteria)
			value += CInt(this.GetSkill(TACTICS) / divisor);
		elseif (THEOLOGY in criteria)
			value += CInt(this.GetSkill(THEOLOGY) / divisor);
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

	// ================= Abilities ================= //
	/**
	 * Obtém as habilidades do mobile.
	 *
	 * Este método recupera as habilidades armazenadas no mobile através da propriedade
	 * ABILITIES. Se o objeto de referência for uma exceção, retorna a própria exceção.
	 *
	 * @param this Referência ao objeto wrapper do mobile
	 * @return Array As habilidades do mobile ou uma Exception caso ocorra erro
	 */
	function GetAbilities(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetProperty(MOBILE_PROPNAME::ABILITIES) ?: dictionary{};
	endfunction

	function TriggerEquipPassives(this, byRef equip)
		var abilities := this.GetAbilities();
		if (abilities)
			foreach level in (abilities.keys())
				run_script_to_completion(":abilities:AbilityController", array{"passive", CInt(abilities[level]), equip, ABILITY_ACTIVATION::ON_EQUIP});
			endforeach
		endif
		equip.SetupOnEquip();
	endfunction

	function TriggerUnequipPassives(this, byRef equip)
		var abilities := this.GetAbilities();
		if (abilities)
			foreach level in (abilities.keys())
				run_script_to_completion(":abilities:AbilityController", array{"passive", CInt(abilities[level]), equip, ABILITY_ACTIVATION::ON_UNEQUIP});
			endforeach
		endif
		equip.CleanupOnUnequip();
	endfunction

	/**
	 * Adiciona uma habilidade ao objeto móvel.
	 *
	 * Este método adiciona uma nova habilidade ao dicionário de habilidades do objeto,
	 * associando-a ao nível em que foi adquirida.
	 *
	 * @param ability A habilidade a ser adicionada ao objeto
	 * @param levelBought O nível no qual a habilidade foi adquirida
	 * @return this Retorna a própria instância do wrapper para encadeamento de métodos,
	 *              ou uma Exception se a referência do objeto for inválida ou a habilidade for inválida
	 * @throws InvalidAbilityException Se a habilidade fornecida não for válida (não possui script)
	 */
	function AddAbility(this, ability, levelBought)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!ability.GetName())
			return InvalidAbilityException("This is not a valid ability!");
		endif

		var abilities := this.GetAbilities();

		abilities[ability.GetName()] := levelBought;
		this.SetProperty(MOBILE_PROPNAME::ABILITIES, abilities);

		return this;
	endfunction

	/**
	 * Remove uma habilidade do personagem móvel.
	 *
	 * @param {MobileWrapper} this - Referência ao objeto wrapper do personagem móvel
	 * @param {Ability} ability - Objeto de habilidade a ser removido
	 * 
	 * @return {MobileWrapper|Exception} Retorna o próprio objeto wrapper em caso de sucesso,
	 *                                    ou uma exceção em caso de erro:
	 *                                    - Exception se _objRef for inválido
	 *                                    - InvalidAbilityException se a habilidade não for válida
	 *                                    - InvalidPropException se o móvel não possuir habilidades
	 *                                    - InvalidAbilityException se o móvel não possuir a habilidade especificada
	 *
	 * @throws {InvalidAbilityException} Quando a habilidade não é válida ou não está presente no móvel
	 * @throws {InvalidPropException} Quando o móvel não possui a propriedade de habilidades
	 */
	function RemoveAbility(this, ability)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!ability.GetName())
			return InvalidAbilityException("This is not a valid ability!");
		endif

		var abilities := this.GetAbilities();
		if (abilities.size() == 0)
			return InvalidPropException("This mobile has no abilities!");
		endif

		if (!this.HasAbility(ability))
			return InvalidAbilityException("This mobile does not have the specified ability!");
		endif

		abilities.remove(ability.GetName());
		this.SetProperty(MOBILE_PROPNAME::ABILITIES, abilities);

		return this;
	endfunction

	/*
	 * Verifica se o mobile possui uma habilidade específica.
	 *
	 * @param this Referência ao objeto MobileWrapper atual
	 * @param ability Objeto da habilidade a ser verificada
	 * @return Retorna o objeto de exceção se _objRef for uma Exception,
	 *         caso contrário retorna true se o mobile possui a habilidade,
	 *         ou false se não possui
	 */
	function HasAbility(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		// myhabs stores ability IDs (integers) keyed by level
		var abilities := this.GetAbilities();
		var targetId := CInt(abilityId);

		foreach level in (abilities.keys())
			if (CInt(abilities[level]) == targetId)
				return true;
			endif
		endforeach

		return false;
	endfunction

	/**
	 * Obtém as habilidades ativas do mobile.
	 * 
	 * @param this Referência ao objeto MobileWrapper
	 * @return Dictionary com as habilidades ativas ou uma Exception caso a referência do objeto seja inválida
	 */
	function GetActiveAbilities(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetProperty(MOBILE_PROPNAME::ACTIVE_ABILITIES) ?: dictionary{};
	endfunction

	/**
	 * Obtém o monitor de uma habilidade ativa específica do mobile.
	 * 
	 * @param {MobileWrapper} this - Referência ao objeto MobileWrapper
	 * @param {Ability} ability - Objeto de habilidade para consultar o monitor
	 * 
	 * @return {Struct} Retorna a estrutura deserializada do monitor da habilidade ativa
	 * @return {Exception} Retorna exceção se a referência do objeto for inválida
	 * @return {InvalidAbilityException} Retorna exceção se o mobile não possuir a habilidade ativa especificada
	 * 
	 * @throws {InvalidAbilityException} Quando a habilidade não existe no momento de ativação especificado
	 */
	function GetAbilityMonitor(this, ability)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var activeAbilities := this.GetActiveAbilities();
		if (!activeAbilities[ability.GetActivationMoment()].exists(ability.GetId()))
			return InvalidAbilityException("This ability is not active on mobile!");
		endif

		var monitor := StructSerializer().Deserialize(
			activeAbilities[ability.GetActivationMoment()][ability.GetId()],
			AbilityMonitor(ability)
		);

		// Stale monitor check - clean up if process died (server restart, etc.)
		if (!GetProcess(monitor.GetPid()))
			activeAbilities[ability.GetActivationMoment()].erase(ability.GetId());
			this.SetProperty(MOBILE_PROPNAME::ACTIVE_ABILITIES, activeAbilities);
			return InvalidAbilityException("This ability is not active on mobile!");
		endif

		return monitor;
	endfunction

	/**
	 * Ativa uma habilidade para o objeto móvel.
	 * 
	 * @param {MobileWrapper} this - Referência ao objeto móvel
	 * @param {Ability} ability - Habilidade a ser ativada
	 * 
	 * @return {MobileWrapper|Exception} Retorna a própria instância do objeto em caso de sucesso,
	 *                                   ou uma Exception se o objeto for inválido ou a habilidade não for válida
	 * 
	 * @throws {InvalidAbilityException} Lançada quando a habilidade não possui um script válido
	 */
	function ActivateAbility(this, ability, abilityMonitor)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!ability.GetName())
			return InvalidAbilityException("This ability is not registered!");
		endif

		if (!ability.GetActivationMoment())
			return InvalidAbilityException("This ability has no activation moment defined to monitor!");
		endif

		var activeAbilities := this.GetActiveAbilities();

		if (!activeAbilities.exists(ability.GetActivationMoment()))
			activeAbilities[ability.GetActivationMoment()] := dictionary{};
		endif

		activeAbilities[ability.GetActivationMoment()][ability.GetId()] := StructSerializer().Serialize(abilityMonitor);
		this.SetProperty(MOBILE_PROPNAME::ACTIVE_ABILITIES, activeAbilities);

		return this;
	endfunction

	/*
	 * Desativa uma habilidade ativa do mobile.
	 *
	 * @param this       Referência ao objeto MobileWrapper atual
	 * @param ability    A habilidade a ser desativada
	 *
	 * @return Retorna o próprio objeto MobileWrapper para encadeamento de métodos,
	 *         ou uma exceção caso o objeto seja inválido ou a habilidade não seja válida
	 *
	 * @throws Exception          Se a referência do objeto for uma exceção
	 * @throws InvalidAbilityException Se a habilidade não possuir um script válido
	 */
	function DeactivateAbility(this, ability)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!ability.GetName())
			return InvalidAbilityException("This is not a valid ability!");
		endif

		ability.OnDeactivate(this);

		var activeAbilities := this.GetActiveAbilities();
		activeAbilities[ability.GetActivationMoment()].erase(ability.GetId());
		this.SetProperty(MOBILE_PROPNAME::ACTIVE_ABILITIES, activeAbilities);

		return this;
	endfunction

	// function HasActiveAbility(this, ability)
	// 	if (this._objRef is @Exception)
	// 		return this._objRef;
	// 	endif

	// 	return this.GetActiveAbilities()[ability.GetActivationMoment()].exists(ability.GetName());
	// endfunction

	// ================ Pool Abilities ================= //

	function AddPoolAbility(this, ability)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!ability.GetName())
			return InvalidAbilityException("This is not a valid ability!");
		endif

		// Store in myhabs like normal
		var abilities := this.GetAbilities();
		abilities[ability.GetName()] := ability.GetId();
		this.SetProperty(MOBILE_PROPNAME::ABILITIES, abilities);

		// Increment pool count
		var poolCounts := this.GetProperty(MOBILE_PROPNAME::POOL_ABILITIES) ?: dictionary{};
		var abilityId := CInt(ability.GetId());
		var currentCount := CInt(poolCounts[abilityId]) ?: 0;
		poolCounts[abilityId] := currentCount + 1;
		this.SetProperty(MOBILE_PROPNAME::POOL_ABILITIES, poolCounts);

		return this;
	endfunction

	function GetPoolAbilityCount(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return 0;
		endif

		var poolCounts := this.GetProperty(MOBILE_PROPNAME::POOL_ABILITIES) ?: dictionary{};
		return CInt(poolCounts[CInt(abilityId)]) ?: 0;
	endfunction

	// ================ Achievements ================= //
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

	// ================ Mastery ================= //
	function GetMasteries(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		var masteries := this.GetProperty(MOBILE_PROPNAME::MASTERY) ?: dictionary{};

		foreach masteryRef in (masteries.keys())
			masteries[masteryRef] := DictionarySerializer().Deserialize(masteries[masteryRef], Mastery(masteryRef));
		endforeach

		return masteries;
	endfunction
	
	function SetMasteries(this, masteriesList)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		foreach masteryRef in (masteriesList.keys())
			masteriesList[masteryRef] := DictionarySerializer().Serialize(masteriesList[masteryRef]);
		endforeach

		this.SetProperty(MOBILE_PROPNAME::MASTERY, masteriesList);

		return this;
	endfunction

	function IsNPC(this)
		return (this.Template && !(this.Template is @Exception));
	endfunction

	function IsDead(this)
		var objRef := this.GetObjRef();
		if (objRef is @Exception)
			return 1;
		endif
		// !objRef catches destroyed NPCs, .dead catches dead players
		return !objRef || objRef.dead;
	endfunction

	function IsWarMode(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return this.GetObjRef().warmode == 1;
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

	/*
	 * Lista mobiles próximos a este mobile usando ListMobilesNearLocationEx.
	 *
	 * @param range    Raio em tiles
	 * @param flags    Flags do ListMobilesNearLocationEx (default LISTEX_FLAG_NORMAL)
	 * @param ignoreZ  Se 1, ignora a coordenada Z na busca
	 * @return array<MobileRef>  Inclui o próprio mobile; cabe ao chamador filtrar
	 * @return Exception         Se o objeto de referência for inválido
	 */
	function GetNearbyMobiles(this, range, flags := LISTEX_FLAG_NORMAL, ignoreZ := 0)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var ref := this.GetObjRef();
		var z := ref.z;
		if (ignoreZ)
			z := LIST_IGNORE_Z;
		endif

		return ListMobilesNearLocationEx(ref.x, ref.y, z, range, flags, ref.realm);
	endfunction

	/*
	 * Lista mobiles hostis a este mobile via ListHostiles.
	 *
	 * @param range  Raio em tiles
	 * @param flags  Flags do ListHostiles (ex: LH_FLAG_LOS). Default 0.
	 * @return array<MobileRef>
	 * @return Exception  Se o objeto de referência for inválido
	 */
	function GetNearbyHostiles(this, range, flags := 0)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		return ListHostiles(this.GetObjRef(), range, flags);
	endfunction

	/*
	 * Aliado se: mesma party, mesma guild, ou other é pet/summon deste mobile.
	 *
	 * @param other  MobileWrapper
	 * @return 1 se aliado, 0 caso contrário
	 * @return Exception  Se ref inválido ou param não-wrapper
	 */
	function IsAllyOf(this, other)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(other is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		var me := this.GetObjRef();
		var them := other.GetObjRef();

		if (me.party && them.party && me.party == them.party)
			return 1;
		endif
		if (me.guild && them.guild && me.guild == them.guild)
			return 1;
		endif
		if (other.GetProperty("owner") == me.serial)
			return 1;
		endif

		return 0;
	endfunction

	/*
	 * Inimigos próximos: GetNearbyMobiles filtrado por !IsAllyOf, exclui self.
	 *
	 * @param range  Raio em tiles
	 * @return array<MobileRef>
	 */
	function GetNearbyEnemies(this, range)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var enemies := array{};
		var meRef := this.GetObjRef();
		var nearby := this.GetNearbyMobiles(range);

		foreach mob in nearby
			if (mob == meRef)
				continue;
			endif
			if (!this.IsAllyOf(MobileWrapper(mob)))
				enemies.append(mob);
			endif
		endforeach

		return enemies;
	endfunction

	/*
	 * Aliados próximos: GetNearbyMobiles filtrado por IsAllyOf, inclui self.
	 *
	 * @param range  Raio em tiles
	 * @return array<MobileRef>
	 */
	function GetNearbyAllies(this, range)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var allies := array{};
		var meRef := this.GetObjRef();
		var nearby := this.GetNearbyMobiles(range);

		foreach mob in nearby
			if (mob == meRef)
				allies.append(mob);
				continue;
			endif
			if (this.IsAllyOf(MobileWrapper(mob)))
				allies.append(mob);
			endif
		endforeach

		return allies;
	endfunction

	/*
	 * NPC: HP < max/2. Player: HP <= GetHP() (STR/4 + hitsmod threshold).
	 *
	 * @return 1 se em fúria (HP baixo), 0 caso contrário
	 */
	function IsRaging(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var current := this.GetCurrentHP();
		if (this.IsNPC())
			return current < CInt(this.GetMaxHP() / 2);
		endif

		return current <= GetHP(this.GetObjRef());
	endfunction

	/*
	 * Verifica se mobile tem um estandarte religioso equipado em LAYER_HAND1 ou LAYER_HAND2.
	 *
	 * @return 1 se equipado, 0 caso contrário
	 */
	function HasReligiousBannerEquipped(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var banners := { 0xF091, 0xF092, 0xF090, 0xECEB, 0xECEA, 0xECE9 };
		var hand1 := this.GetEquipByLayer(LAYER_HAND1);
		var hand2 := this.GetEquipByLayer(LAYER_HAND2);

		return (hand1 && hand1.objtype in banners) || (hand2 && hand2.objtype in banners);
	endfunction

	/*
	 * Teleporta este mobile para a coordenada atrás do alvo, encarando o alvo.
	 * Bloqueia (sleeps) durante o movimento em linha. Fallback para tile aleatório
	 * em raio 1 do alvo se a posição direta não for caminhável.
	 *
	 * @param target  MobileWrapper
	 * @return 1 em sucesso
	 * @return Exception  Se ref inválido ou param não-wrapper
	 */
	function TeleportBehind(this, target)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(target is @MobileWrapper))
			return InvalidFormatException("The param must be a MobileWrapper object!");
		endif

		var me := this.GetObjRef();
		var targ := target.GetObjRef();

		var coords := GetCoordsInLine(me.x, me.y, targ.x, targ.y);
		coords.Erase(coords.size());
		foreach coord in coords
			MoveObjectToLocation(me, coord.x, coord.y, targ.z, me.realm, MOVEOBJECT_FORCELOCATION);
			sleepms(50);
		endforeach

		var behind := InverseFacing(targ.facing);
		behind := GetModByFacing(behind);
		if (CanWalk("L", me.x, me.y, me.z, targ.x + behind[1], targ.y + behind[2]))
			MoveObjectToLocation(me, targ.x + behind[1], targ.y + behind[2], targ.z, me.realm, MOVEOBJECT_FORCELOCATION);
		else
			var radius := GetPointsInRadius(targ.x, targ.y, 1);
			var random := radius.randomentry();
			while (!MoveObjectToLocation(me, random.x, random.y, targ.z, me.realm))
				random := radius.randomentry();
				sleep(2);
			endwhile
		endif

		me.SetFacing(GetFacing(me.x, me.y, targ.x, targ.y));
		return 1;
	endfunction

	/*
	 * Verifica se mobile está dentro de uma zona de fumaça shinobi ativa
	 * (#smoke_state ObjProperty). Limpa automaticamente o estado se expirado
	 * ou inválido.
	 *
	 * @return 1 se dentro de zona ativa, 0 caso contrário
	 */
	function IsInSmokeZone(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var meRef := this.GetObjRef();
		var smoke := this.GetProperty("#smoke_state");
		if (!smoke || smoke == error)
			return 0;
		endif

		var expires := smoke.expires;
		if (!expires || ReadGameClock() > expires)
			this.ClearSmokeProtection();
			return 0;
		endif

		var sx := smoke.x;
		var sy := smoke.y;
		var sr := smoke.radius;
		if (sx == error || sy == error || sr == error)
			this.ClearSmokeProtection();
			return 0;
		endif

		var dx := CInt(meRef.x) - CInt(sx);
		var dy := CInt(meRef.y) - CInt(sy);
		var radius := CInt(sr);

		if ((dx * dx + dy * dy) > (radius * radius))
			this.ClearSmokeProtection();
			return 0;
		endif

		return 1;
	endfunction

	/*
	 * Limpa proteção da zona de fumaça shinobi: erase #smoke_state, concealed := 0,
	 * revoga "hiddenattack".
	 */
	function ClearSmokeProtection(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var meRef := this.GetObjRef();
		EraseObjProperty(meRef, "#smoke_state");
		meRef.concealed := 0;
		RevokePrivilege(meRef, "hiddenattack");
	endfunction

	// ============================================================================
	// Ability Cooldown Methods
	// Cooldown stored as absolute end-time (POLCore().systime + duration).
	// ObjProperty key: "AbilityCooldown_{abilityId}".
	// ============================================================================

	function IsAbilityOnCooldown(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var cooldownEnd := this.GetProperty($"AbilityCooldown_{abilityId}");
		return cooldownEnd && POLCore().systime < cooldownEnd;
	endfunction

	/*
	 * Segundos restantes no cooldown da habilidade. 0 se sem cooldown ou expirado.
	 */
	function GetAbilityCooldownRemaining(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var cooldownEnd := this.GetProperty($"AbilityCooldown_{abilityId}");
		if (!cooldownEnd)
			return 0;
		endif
		var remaining := cooldownEnd - POLCore().systime;
		if (remaining > 0)
			return remaining;
		endif
		return 0;
	endfunction

	function SetAbilityCooldown(this, abilityId, seconds)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		this.SetProperty($"AbilityCooldown_{abilityId}", POLCore().systime + seconds);
		return this;
	endfunction

	function ClearAbilityCooldown(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		EraseObjProperty(this.GetObjRef(), $"AbilityCooldown_{abilityId}");
		return this;
	endfunction

	/*
	 * Reverte ativação de habilidade: limpa cooldown, envia packet zerado,
	 * restaura HP/STAM/MANA consumidos. Usado quando CanActivate falha após
	 * já ter cobrado custos.
	 */
	function RollbackAbilityActivation(this, abilityId)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var ability := Ability(abilityId);
		this.ClearAbilityCooldown(abilityId);
		SendCooldownPacket(this.GetObjRef(), ability.GetName(), 0, ability.GetIcon(), COOLDOWN_TYPE_ABILITY);

		if (ability.GetCostSTAM() > 0)
			this.RestoreStamina(ability.GetCostSTAM());
		endif
		if (ability.GetCostMANA() > 0)
			this.RestoreMana(ability.GetCostMANA());
		endif
		if (ability.GetCostHP() > 0)
			this.RestoreHP(ability.GetCostHP());
		endif
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
	ENTROPY := "entropy",
	ABILITIES := "myhabs",
	ACTIVE_ABILITIES := "activehabs",
	POOL_ABILITIES := "myhabs_pool",
	MASTERY := "masteries"
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
