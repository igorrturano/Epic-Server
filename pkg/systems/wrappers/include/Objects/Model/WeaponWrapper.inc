use math;

include "IWeaponWrapper";
include "EquipWrapper";
include ":equipsys:Controller/WeaponController";

include ":combat:Model/DamageTier";
include ":combat:Model/DamageDice";
include ":combat:Model/Damage";
include ":combat:Model/Ammo";
include ":diceroll:model/Dice";

class WeaponWrapper(EquipWrapper, IWeaponWrapper)
	var _attribute := "";
	var _attackType := "";
	var _speedTier := "";
	var _speed := 0;
	var _speedModTier := "";
	var _speedMod := 0;
	var _mainDamageTier; // DamageDice class
	var _offDamageTier; // DamageDice class
	var _mainDamage; // DamageDice class
	var _offDamage; // DamageDice class
	var _damageMod; // Damage class
	var _criticalChanceTier := "";
	var _criticalChance := 0;
	var _criticalChanceMod := 0;
	var _strRequired := 0;
	var _ammo; // Ammo class
	var _minRange := 1;
	var _maxRange := 1;
	var _isThrowable := false;
	var _epicMainDamage := "";

	function WeaponWrapper(this, byRef item)
		Super(item);
		
		if (this._template)
			this._attribute := this._template.Attribute;
			this._attackType := this._template.TipoAtaque;
			this._category := this._template.Category;
			this._speedTier := this._template.EpicSpeed;
			this._criticalChanceTier := this._template.EpicCriticalChance;
			this._criticalChanceMod := 0;
			this._epicMainDamage := this._template.EpicMainDamage;
			this._mainDamageTier := !this._template.EpicMainDamage ? DamageTier() : DamageTier(
				SplitWords(this._template.EpicMainDamage)[1],
				SplitWords(this._template.EpicMainDamage)[2],
				SplitWords(this._template.EpicMainDamage)[3]
			);
			this._offDamageTier := !this._template.EpicOffDamage ? DamageTier() : DamageTier(
				SplitWords(this._template.EpicOffDamage)[1],
				SplitWords(this._template.EpicOffDamage)[2],
				SplitWords(this._template.EpicOffDamage)[3]
			);
			this._mainDamage := DamageDice(
				Dice(GetDamageByTier(this._mainDamageTier.GetSlashDamage())),
				Dice(GetDamageByTier(this._mainDamageTier.GetPiercingDamage())),
				Dice(GetDamageByTier(this._mainDamageTier.GetBluntDamage()))
			);
			this._offDamage := DamageDice(
				Dice(GetDamageByTier(this._offDamageTier.GetSlashDamage())),
				Dice(GetDamageByTier(this._offDamageTier.GetPiercingDamage())),
				Dice(GetDamageByTier(this._offDamageTier.GetBluntDamage()))
			);
			this._damageMod := Damage();
			this._speed := getSpeedByTier(this._speedTier);
			this._criticalChance := getCriticalChanceByTier(this._criticalChanceTier);
			this._weight := this._GetWeight();
			this._strRequired := this._GetSTRRequired();
			this._ammo := this._template.ProjectileType ? Ammo(this._template.ProjectileType) : uninit;
			this._minRange := this._template.MinRange ?: 1;
			this._maxRange := this._template.MaxRange ?: 1;
			this._isThrowable := Boolean(this._template.Throwable) ?: false;

			if (this._prop) // Alguns atributos só estão nas props
				this._criticalChanceMod := this._prop.CriticalChanceMod;
				this._damageMod.SetSlashRawDamage(CInt(this._prop.SlashDamageMod));
				this._damageMod.SetPiercingRawDamage(CInt(this._prop.PiercingDamageMod));
				this._damageMod.SetBluntRawDamage(CInt(this._prop.BluntDamageMod));
				this._damageMod.SetFireRawDamage(CInt(this._prop.FireDamage));
				this._damageMod.SetColdRawDamage(CInt(this._prop.ColdDamage));
				this._damageMod.SetPoisonRawDamage(CInt(this._prop.PoisonDamage));
				this._damageMod.SetEnergyRawDamage(CInt(this._prop.EnergyDamage));
			endif
		else
			var err := InvalidTemplateException("The param must be a Weapon ObjRef or a valid Template!");
			this._attribute := err;
			this._attackType := err;
			this._category := err;
			this._speedTier := err;
			this._mainDamageTier := err;
			this._offDamageTier := err;
			this._mainDamage := err;
			this._offDamage := err;
			this._damageMod := err;
			this._criticalChanceTier := err;
			this._criticalChance := err;
			this._criticalChanceMod := err;
			this._speed := err;
			this._strRequired := err;
			this._epicMainDamage := err;
		endif
	endfunction

	function GetAttribute(this)
		return this._attribute;
	endfunction

	function GetAttackType(this)
		return this._attackType;
	endfunction

	function GetAnimation(this)
		return this._animation;
	endfunction

	function GetSpeedTier(this)
		return this._speedTier;
	endfunction

	function GetSpeed(this)
		return this._speed;
	endfunction

	function GetMainDamageTier(this)
		return this._mainDamageTier;
	endfunction

	function GetOffDamageTier(this)
		return this._offDamageTier;
	endfunction

	function GetEpicMainDamage(this)
		return this._epicMainDamage;
	endfunction
	
	/**
	 * Retorna o dano principal da arma.
	 *
	 * @return DamageDice do dano principal
	 */
	function GetMainDamage(this)
		return this._mainDamage;
	endfunction

	/**
	 * Retorna o DamageDice da mão secundária.
	 *
	 * @return DamageDice da mão secundária
	 */
	function GetOffDamage(this)
		return this._offDamage;
	endfunction

	/*
	 * Retorna o modificador de dano da arma.
	 *
	 * @return objeto Damage contendo o modificador de dano
	 */
	function GetDamageMod(this)
		return this._damageMod;
	endfunction

	function GetSTRRequired(this)
		return this._strRequired;
	endfunction

	function GetCriticalChanceTier(this)
		return this._criticalChanceTier;
	endfunction

	function GetCriticalChance(this)
		return CInt(this.GetCriticalChanceBase() + this.GetCriticalChanceMod());
	endfunction

	function GetCriticalChanceBase(this)
		return CInt(this._criticalChance);
	endfunction

	function GetCriticalChanceMod(this)
		return CInt(this._criticalChanceMod);
	endfunction

	/*
	 * Retorna a munição atrelada à arma.
	 *
	 * @return Ammo - Objeto da classe Ammo representando a munição da arma
	 */
	function GetAmmo(this)
		return this._ammo;
	endfunction

	function GetMinRange(this)
		return this._minRange;
	endfunction

	function GetMaxRange(this)
		return this._maxRange;
	endfunction

	function SetSpeed(this, value)
		if (this._objRef is @Exception) return this._objRef; endif
		this._speed := CInt(value);

		return this;
	endfunction

	/**
	 * Define os valores de dano principal da arma
	 *
	 * @param {WeaponWrapper} this - Instância do objeto WeaponWrapper
	 * @param {DamageDice} slash - Objeto DamageDice representando o dano de corte
	 * @param {DamageDice} piercing - Objeto DamageDice representando o dano perfurante
	 * @param {DamageDice} blunt - Objeto DamageDice representando o dano contundente
	 *
	 * @return {WeaponWrapper|Error} Retorna a própria instância para encadeamento ou erro se:
	 *                                - O item interno for inválido
	 *                                - Algum parâmetro não for uma instância de DamageDice
	 */
	function SetMainDamage(this, slash, piercing, blunt)
		if (this._objRef is @Exception) return this._objRef; endif
		if (!(slash is @DamageDice) || !(piercing is @DamageDice) || !(blunt is @DamageDice))
			return error{errortext := "Parameters must be a DamageDice class!"};
		endif
		this._mainDamage.SetSlashDamage(slash);
		this._mainDamage.SetPiercingDamage(piercing);
		this._mainDamage.SetBluntDamage(blunt);

		return this;
	endfunction

	/**
	 * Define os valores de dano secundário da arma
	 *
	 * @param {WeaponWrapper} this - Instância do objeto WeaponWrapper
	 * @param {DamageDice} slash - Objeto DamageDice representando o dano de corte
	 * @param {DamageDice} piercing - Objeto DamageDice representando o dano perfurante
	 * @param {DamageDice} blunt - Objeto DamageDice representando o dano contundente
	 *
	 * @return {WeaponWrapper|Error} Retorna a própria instância para encadeamento ou erro se:
	 *                                - O item interno for inválido
	 *                                - Algum parâmetro não for uma instância de DamageDice
	 */
	function SetOffDamage(this, slash, piercing, blunt)
		if (this._objRef is @Exception) return this._objRef; endif
		if (!(slash is @DamageDice) || !(piercing is @DamageDice) || !(blunt is @DamageDice))
			return error{errortext := "Parameters must be a DamageDice class!"};
		endif
		this._offDamage.SetSlashDamage(slash);
		this._offDamage.SetPiercingDamage(piercing);
		this._offDamage.SetBluntDamage(blunt);

		return this;
	endfunction

	/**
	 * Define o modificador de dano para a arma.
	 *
	 * @param {WeaponWrapper} this - Instância do wrapper da arma
	 * @param {Damage} damageMod - Objeto da classe Damage que será usado como modificador de dano
	 *
	 * @return {WeaponWrapper|Error} Retorna a própria instância (this) para encadeamento de métodos,
	 *                                ou erro se o item não existir ou se o parâmetro não for da classe Damage
	 *
	 * @error Retorna erro se this._item for error
	 * @error Retorna erro com mensagem "Parameter must be a Damage class!" se damageMod não for da classe Damage
	 */
	function SetDamageMod(this, damageMod)
		if (this._objRef is @Exception) return this._objRef; endif
		if (!(damageMod is @Damage))
			return error{errortext := "Parameter must be a Damage class!"};
		endif
		this._damageMod := damageMod;

		return this;
	endfunction

	function SetCriticalChanceBase(this, value)
		if (this._objRef is @Exception) return this._objRef; endif
		this._criticalChance := CInt(value);

		return this;
	endfunction

	/**
	 * Define a velocidade da arma
	 * 
	 * @param value Valor da velocidade a ser definida (será convertido para inteiro)
	 * @return Retorna a própria instância do objeto para permitir encadeamento de métodos, 
	 *         ou uma exceção caso _objRef seja uma Exception
	 */
	function SetCriticalChanceMod(this, value)
		if (this._objRef is @Exception) return this._objRef; endif
		this._criticalChanceMod := CInt(value);

		return this;
	endfunction

	/**
	 * Define a munição para a arma.
	 * 
	 * @param ammo Objeto da classe Ammo a ser definido como munição da arma
	 * 
	 * @return this Retorna a própria instância para permitir encadeamento de métodos
	 * @return Exception Retorna exceção se a referência do objeto for inválida
	 * @return InvalidFormatException Retorna exceção se o parâmetro não for da classe Ammo
	 */
	function SetAmmo(this, ammo)
		if (this._objRef is @Exception)
			return this._objRef;
		endif
		
		if (!(ammo is @Ammo))
			return InvalidFormatException("Parameter must be an Ammo class!");
		endif
		
		this._ammo := ammo;
		
		return this;
	endfunction

	/**
	 * Atualiza as propriedades da arma no item.
	 * 
	 * Esta função sincroniza todos os atributos da arma wrapper com as propriedades
	 * do item subjacente, incluindo tipos de dano, atributos de combate e requisitos.
	 * 
	 * @param this O objeto WeaponWrapper a ser atualizado
	 * @return Retorna o item atualizado ou um erro caso o item seja inválido
	 * 
	 * @note A função primeiro verifica se o item é válido antes de prosseguir
	 * @note Chama a função Update da classe pai (EquipWrapper) antes de processar
	 * @note Atualiza os seguintes atributos:
	 *       - Attribute: Atributo principal da arma
	 *       - AttackType: Tipo de ataque
	 *       - Danos principais: Slash, Piercing e Blunt
	 *       - Danos secundários: Slash, Piercing e Blunt
	 *       - CriticalChance: Chance de crítico base
	 *       - CriticalChanceMod: Modificador de chance de crítico
	 *       - STRRequired: Força requerida para equipar
	 *       - AttackSpeed: Velocidade de ataque
	 *       - MinRange/MaxRange: Alcance mínimo e máximo
	 * @note Calcula e define o modificador de velocidade (speed_mod) do item
	 * @note Persiste todas as propriedades através do método SetProp
	 */
	function Update(this)
		if (this._objRef is @Exception) return this._objRef; endif
		EquipWrapper::Update(this);

		this._prop.+Attribute := this._attribute;
		this._prop.+AttackType := this._attackType;
		this._prop.+SlashMainDamage := this._mainDamage.GetSlashDamage();
		this._prop.+PiercingMainDamage := this._mainDamage.GetPiercingDamage();
		this._prop.+BluntMainDamage := this._mainDamage.GetBluntDamage();
		this._prop.+SlashOffDamage := this._offDamage.GetSlashDamage();
		this._prop.+PiercingOffDamage := this._offDamage.GetPiercingDamage();
		this._prop.+BluntOffDamage := this._offDamage.GetBluntDamage();
		this._prop.+CriticalChance := this._criticalChance;
		this._prop.+CriticalChanceMod := this._criticalChanceMod;
		this._prop.+STRRequired := this._strRequired;
		this._prop.+AttackSpeed := this._speed;
		this._prop.+MinRange := this._minRange;
		this._prop.+MaxRange := this._maxRange;

		this._objRef.speed_mod := CInt(this._speed) - CInt(this._template.Speed);
		
		this._objRef.SetProp(PROPNAME_ITEMINFO, this._prop);
	endfunction

	function ToString(this)
		return Super::ToString(this) + "Weapon: " +
			$"\nAttribute: {this._attribute}, Attack Type: {this._attackType}" +
			$"\nMainDamage (S/P/B): {this._mainDamage.GetSlashDamage()} ({this._mainDamageTier.GetSlashDamage()})/{this._mainDamage.GetPiercingDamage()} ({this._mainDamageTier.GetPiercingDamage()})/{this._mainDamage.GetBluntDamage()} ({this._mainDamageTier.GetBluntDamage()})" +
			$"\nOffDamage (S/P/B): {this._offDamage.GetSlashDamage()} ({this._offDamageTier.GetSlashDamage()})/{this._offDamage.GetPiercingDamage()} ({this._offDamageTier.GetPiercingDamage()})/{this._offDamage.GetBluntDamage()} ({this._offDamageTier.GetBluntDamage()})" +
			$"\nSpeed: {this._speed} ({this._speedTier}), Critical Chance: {this._criticalChance}% ({this._criticalChanceTier})" +
			$"\nRange: {this._minRange}-{this._maxRange}";
	endfunction

	function IsMontante(this)
		return this._objType in array{0x26CF, 0xF9EA, 0xF9F7};
	endfunction

	function IsRanged(this)
		return this._maxRange > 1;
	endfunction

	function IsThrowable(this)
		return this._isThrowable;
	endfunction

	// Private functions
	function _GetWeight(this)
		var maxMainDamage := Max(array{
			CInt(SplitWords(this._mainDamage.GetSlashDamage(), "d")[2]),
			CInt(SplitWords(this._mainDamage.GetPiercingDamage(), "d")[2]),
			CInt(SplitWords(this._mainDamage.GetBluntDamage(), "d")[2])
		});
		var maxOffDamage := Max(array{
			CInt(SplitWords(this._offDamage.GetSlashDamage(), "d")[2]),
			CInt(SplitWords(this._offDamage.GetPiercingDamage(), "d")[2]),
			CInt(SplitWords(this._offDamage.GetBluntDamage(), "d")[2])
		});

		return CInt(maxMainDamage) + CInt(maxOffDamage);
	endfunction

	function _GetSTRRequired(this)
		return CInt(this._weight * 7.5);
	endfunction
endclass

enum class WEAPON_CATEGORY
	UNARMED := "Unarmed",

	DAGGER := "Dagger",
	SWORD := "Sword",
	GREATSWORD := "GreatSword",
	AXE := "Axe",
	SPEAR := "Spear",
	MACE := "Mace",
	STAFF := "Staff",
	THROWING := "Throwing",
	POLEARM := "Polearm",

	BOW := "Bow",
	CROSSBOW := "Crossbow",
	MAGICBOW := "MagicBow"
endenum
