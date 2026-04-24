use math;

include "IWeaponWrapper";
include "EquipWrapper";
include ":equipsys:Controller/WeaponController";

include ":combat:Model/DamageTier";
include ":combat:Model/DamageDice";
include ":combat:Model/Damage";
include ":combat:Model/DamageModCollection";
include ":combat:Model/DamageModBase";
include ":combat:Model/DamageModFabricacao";
include ":combat:Model/Ammo";
include ":diceroll:model/Dice";

class WeaponWrapper(EquipWrapper, IWeaponWrapper)
	var Attribute := "";
	var AttackType := "";
	var SpeedTier := "";
	var Speed := 0;
	var SpeedModTier := "";
	var SpeedMod := 0;
	var MainDamageTier; // DamageTier class
	var OffDamageTier; // DamageTier class
	var MainDamage; // DamageDice class
	var OffDamage; // DamageDice class
	var DamageMod; // Damage class
	var CriticalChanceTier := "";
	var CriticalChance := 0;
	var CriticalChanceMod := 0;
	var AccuracyMod := 0;
	var StrRequired := 0;
	var Ammo; // Ammo class
	var MinRange := 0;
	var MaxRange := 1;
	var IsThrowableVar := false;
	var EpicMainDamage := "";

	function WeaponWrapper(this, byRef item)
		Super(item);
		
		if (this.GetTemplate())
			this.Attribute := this.GetTemplate().Attribute;
			this.AttackType := this.GetTemplate().TipoAtaque;
			this.Category := this.GetTemplate().Category;
			this.SpeedTier := this.GetTemplate().EpicSpeed;
			this.CriticalChanceTier := this.GetTemplate().EpicCriticalChance;
			//this.CriticalChanceMod := 0;
			this.EpicMainDamage := this.GetTemplate().EpicMainDamage;
			this.MainDamageTier := !this.GetTemplate().EpicMainDamage ? DamageTier() : DamageTier(
				SplitWords(this.GetTemplate().EpicMainDamage)[1],
				SplitWords(this.GetTemplate().EpicMainDamage)[2],
				SplitWords(this.GetTemplate().EpicMainDamage)[3]
			);
			this.OffDamageTier := !this.GetTemplate().EpicOffDamage ? DamageTier() : DamageTier(
				SplitWords(this.GetTemplate().EpicOffDamage)[1],
				SplitWords(this.GetTemplate().EpicOffDamage)[2],
				SplitWords(this.GetTemplate().EpicOffDamage)[3]
			);
			this.MainDamage := DamageDice(
				Dice(GetDamageByTier(this.MainDamageTier.GetSlashDamage())),
				Dice(GetDamageByTier(this.MainDamageTier.GetPiercingDamage())),
				Dice(GetDamageByTier(this.MainDamageTier.GetBluntDamage()))
			);
			this.OffDamage := DamageDice(
				Dice(GetDamageByTier(this.OffDamageTier.GetSlashDamage())),
				Dice(GetDamageByTier(this.OffDamageTier.GetPiercingDamage())),
				Dice(GetDamageByTier(this.OffDamageTier.GetBluntDamage()))
			);
			this.DamageMods := DamageModCollection();
			this.SetDamageMod(DamageModFabricacao(this.GetObjRef() ? this.GetObjRef() : this.GetTemplate()));

			this.Speed := GetSpeedByTier(this.SpeedTier);
			this.CriticalChance := GetCriticalChanceByTier(this.CriticalChanceTier);
			this.MaxHP := GetDurabilityByType(this.GetAttackType());
			this.WeightVar := this._CalculateWeight();
			this.StrRequired := this._CalculateSTRRequired();
			this.Ammo := this.Template.ProjectileType ? Ammo(this.Template.ProjectileType) : uninit;
			this.MinRange := this.Template.MinRange ?: 0;
			this.MaxRange := this.Template.MaxRange ?: 1;
			this.IsThrowableVar := Boolean(this.Template.Throwable) ?: false;
			this.CriticalChanceMod := PegaValorFabricacao(this.GetObjRef(),CARACTERISTICA_FABRICACAO_CRITICO);
			this.AccuracyMod := PegaValorFabricacao(this.GetObjRef(),CARACTERISTICA_FABRICACAO_PRECISAO);
			/* TODO ver com o Evil sobre essa forma de guardar o CriticalChanceMod , me pareceu que ficaria dificil executar os calculos.
			if (this.Prop) // Alguns atributos só estão nas props
				this.CriticalChanceMod := this.Prop.CriticalChanceMod;
			endif
			*/
		else
			var err := InvalidTemplateException("The param must be a Weapon ObjRef or a valid Template!");
			this.Attribute := err;
			this.AttackType := err;
			this.Category := err;
			this.SpeedTier := err;
			this.MainDamageTier := err;
			this.OffDamageTier := err;
			this.MainDamage := err;
			this.OffDamage := err;
			this.DamageMod := err;
			this.CriticalChanceTier := err;
			this.CriticalChance := err;
			this.CriticalChanceMod := err;
			this.AccuracyMod := err;
			this.Speed := err;
			this.StrRequired := err;
			this.EpicMainDamage := err;
		endif
	endfunction

	function GetAttribute(this)
		return this.Attribute;
	endfunction

	function GetAttackType(this)
		return this.AttackType;
	endfunction

	function GetAnimation(this)
		return this.Animation;
	endfunction

	function GetSpeedTier(this)
		return this.SpeedTier;
	endfunction

	function GetSpeed(this)
		return this.Speed;
	endfunction

	function GetMainDamageTier(this)
		return this.MainDamageTier;
	endfunction

	function GetOffDamageTier(this)
		return this.OffDamageTier;
	endfunction

	function GetEpicMainDamage(this)
		return this.EpicMainDamage;
	endfunction
	
	/**
	 * Retorna o dano principal da arma.
	 *
	 * @return DamageDice do dano principal
	 */
	function GetMainDamage(this)
		return this.MainDamage;
	endfunction

	/**
	 * Retorna o DamageDice da mão secundária.
	 *
	 * @return DamageDice da mão secundária
	 */
	function GetOffDamage(this)
		return this.OffDamage;
	endfunction

	/*
	 * Retorna o modificador de dano da arma.
	 *
	 * @return objeto Damage contendo o modificador de dano
	 */
	function GetDamageMod(this)
		return this.DamageMods.GetTotal();
	endfunction

	function GetAccuracyMod(this)
		return this.AccuracyMod;
	endfunction

	function GetSTRRequired(this)
		return this.StrRequired;
	endfunction

	function GetCriticalChanceTier(this)
		return this.CriticalChanceTier;
	endfunction

	function GetCriticalChance(this)
		return CInt(this.GetCriticalChanceBase() + this.GetCriticalChanceMod());
	endfunction

	function GetCriticalChanceBase(this)
		return CInt(this.CriticalChance);
	endfunction

	function GetCriticalChanceMod(this)
		return CInt(this.CriticalChanceMod);
	endfunction

	/*
	 * Retorna a munição atrelada à arma.
	 *
	 * @return Ammo - Objeto da classe Ammo representando a munição da arma
	 */
	function GetAmmo(this)
		return this.Ammo;
	endfunction

	function GetMinRange(this)
		return this.MinRange;
	endfunction

	function GetMaxRange(this)
		return this.MaxRange;
	endfunction

	function SetSpeed(this, value)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		this.Speed := CInt(value);

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
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		if (!(slash is @DamageDice) || !(piercing is @DamageDice) || !(blunt is @DamageDice))
			return error{errortext := "Parameters must be a DamageDice class!"};
		endif
		this.MainDamage.SetSlashDamage(slash);
		this.MainDamage.SetPiercingDamage(piercing);
		this.MainDamage.SetBluntDamage(blunt);

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
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		if (!(slash is @DamageDice) || !(piercing is @DamageDice) || !(blunt is @DamageDice))
			return error{errortext := "Parameters must be a DamageDice class!"};
		endif
		this.OffDamage.SetSlashDamage(slash);
		this.OffDamage.SetPiercingDamage(piercing);
		this.OffDamage.SetBluntDamage(blunt);

		return this;
	endfunction

	/**
	 * Define o modificador de dano para a arma.
	 *
	 * @param {WeaponWrapper} this - Instância do wrapper da arma
	 * @param {DamageMod} damageMod - Objeto da classe Damage que será usado como modificador de dano
	 *
	 * @return {WeaponWrapper|Error} Retorna a própria instância (this) para encadeamento de métodos,
	 *                                ou erro se o item não existir ou se o parâmetro não for da classe Damage
	 *
	 * @error Retorna erro se this.GetObjRef() for error
	 * 
	 */
	function SetDamageMod(this,byref damageMod)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		var ret := this.DamageMods.InsertMod(damageMod);

		if(ret is @Exception) return this.ret; endif

		return this;
	endfunction

	function SetAccuracyMod(this, accuracyMod)
		this.AccuracyMod := accuracyMod;

		return this;
	endfunction

	function SetCriticalChanceBase(this, value)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		this.CriticalChance := CInt(value);

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
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		this.CriticalChanceMod := CInt(value);

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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif
		
		if (!(ammo is @Ammo))
			return InvalidFormatException("Parameter must be an Ammo class!");
		endif
		
		this.Ammo := ammo;
		
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
	 * @note Persiste todas as propriedades através do método SetProperty
	 */
	function Update(this)
		if (this.GetObjRef() is @Exception) return this.GetObjRef(); endif
		EquipWrapper::Update(this);

		var prop := this.GetEquipProp();
		prop.+Attribute := this.GetAttribute();
		prop.+AttackType := this.GetAttackType();
		prop.+SlashMainDamage := this.GetMainDamage().GetSlashDamage();
		prop.+PiercingMainDamage := this.GetMainDamage().GetPiercingDamage();
		prop.+BluntMainDamage := this.GetMainDamage().GetBluntDamage();
		prop.+SlashOffDamage := this.GetOffDamage().GetSlashDamage();
		prop.+PiercingOffDamage := this.GetOffDamage().GetPiercingDamage();
		prop.+BluntOffDamage := this.GetOffDamage().GetBluntDamage();
		prop.+CriticalChance := this.GetCriticalChance();
		//prop.+CriticalChanceMod := this.GetCriticalChanceMod();
		prop.+STRRequired := this.GetStrRequired();
		prop.+AttackSpeed := this.GetSpeed();
		prop.+MinRange := this.GetMinRange();
		prop.+MaxRange := this.GetMaxRange();
		this.SetEquipProp(prop);

		// Aqui seta as propriedades no ObjRef
		this.GetObjRef().speed_mod := CInt(this.GetSpeed()) - CInt(this.GetTemplate().Speed);
	endfunction

	function ToString(this)
		return Super::ToString(this) + "Weapon: " +
			$"\nAttribute: {this.GetAttribute()}, Attack Type: {this.GetAttackType()}" +
			$"\nMainDamage (S/P/B): {this.GetMainDamage().GetSlashDamage()} ({this.GetMainDamageTier().GetSlashDamage()})/{this.GetMainDamage().GetPiercingDamage()} ({this.GetMainDamageTier().GetPiercingDamage()})/{this.GetMainDamage().GetBluntDamage()} ({this.GetMainDamageTier().GetBluntDamage()})" +
			$"\nOffDamage (S/P/B): {this.GetOffDamage().GetSlashDamage()} ({this.GetOffDamageTier().GetSlashDamage()})/{this.GetOffDamage().GetPiercingDamage()} ({this.GetOffDamageTier().GetPiercingDamage()})/{this.GetOffDamage().GetBluntDamage()} ({this.GetOffDamageTier().GetBluntDamage()})" +
			$"\nSpeed: {this.GetSpeed()} ({this.GetSpeedTier()}), Critical Chance: {this.GetCriticalChance()}% ({this.GetCriticalChanceTier()})" +
			$"\nRange: {this.GetMinRange()}-{this.GetMaxRange()}";
	endfunction

	function IsThrowable(this)
		return this.IsThrowableVar;
	endfunction

	function isFireWeapon(this)
		return this.GetCategory() == WEAPON_CATEGORY::PISTOL ||
		       this.GetCategory() == WEAPON_CATEGORY::MUSKET ||
		       this.GetCategory() == WEAPON_CATEGORY::CARABINE ||
		       this.GetCategory() == WEAPON_CATEGORY::HANDCANNON;
	endfunction

	// Private functions
	function _CalculateWeight(this)
		var weight := 1;
		
		var maxMainDamage := this.GetMainDamage().GetMaximum();
		var maxOffDamage := this.GetOffDamage().GetMaximum();

		weight := CInt(maxMainDamage) + CInt(maxOffDamage);
		weight *= GetWeightMultiplierByMaterial(this.GetMaterial());

		return weight;
	endfunction

	function _CalculateSTRRequired(this)
		return CInt(this.GetWeight() * 7.5);
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
	WHIP := "Whip",

	BOW := "Bow",
	CROSSBOW := "Crossbow",
	MAGICBOW := "MagicBow",

	PISTOL := "Pistol",
	MUSKET := "Musket",
	CARABINE := "Carabine",
	HANDCANNON := "HandCannon",

	SPIRITUALBLADE := "SpiritualBlade"
endenum
