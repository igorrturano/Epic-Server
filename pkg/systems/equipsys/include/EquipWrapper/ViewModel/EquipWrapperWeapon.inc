use math;

include "EquipWrapper";
include "IEquipWrapperWeapon";
include ":equipsys:Controller/WeaponController";

include ":combat:Model/WeaponDamage";

class EquipWrapperWeapon(EquipWrapper, IEquipWrapperWeapon)
	var _attribute := "";
	var _tipoAtaque := "";
	var _speedTier := "";
	var _speed := 0;
	var _mainDamageTier := WeaponDamage();
	var _offDamageTier := WeaponDamage();
	var _mainDamage := WeaponDamage();
	var _offDamage := WeaponDamage();
	var _damageMod := WeaponDamage();
	var _criticalChanceTier := "";
	var _criticalChance := 0;
	var _criticalChanceMod := 0;
	var _strRequired := 0;
	var _minRange := 0;
	var _maxRange := 0;
	var _penalties := struct{};

	function EquipWrapperWeapon(this, byRef item)
		Super(item);
		
		if (item.isA(POLCLASS_WEAPON)) // Caso o itemRef exista
			this._attribute := this._template.Attribute;
			this._tipoAtaque := this._template.TipoAtaque ?: error{"errortext" := "The item have no Attack Type!"};
			this._speedTier := this._template.EpicSpeed;
			this._speed := this._prop.AttackSpeed;
			this._mainDamageTier := WeaponDamage(
				SplitWords(this._template.EpicMainDamage)[1],
				SplitWords(this._template.EpicMainDamage)[2],
				SplitWords(this._template.EpicMainDamage)[3]
			);
			this._offDamageTier := WeaponDamage(
				SplitWords(this._template.EpicOffDamage)[1],
				SplitWords(this._template.EpicOffDamage)[2],
				SplitWords(this._template.EpicOffDamage)[3]
			);
			this._mainDamage := WeaponDamage(
				this._prop.SlashMainDamage,
				this._prop.PiercingMainDamage,
				this._prop.BluntMainDamage
			);
			this._offDamage := WeaponDamage(
				this._prop.SlashOffDamage,
				this._prop.PiercingOffDamage,
				this._prop.BluntOffDamage
			);
			this._criticalChanceTier := this._template.EpicCriticalChance;
			this._criticalChance := this._prop.CriticalChance;
			this._criticalChanceMod := this._prop.CriticalChanceMod;
			this._weight := this._GetWeight();
			this._strRequired := this._GetSTRRequired();
			this._minRange := this._template.MinRange ?: 1;
			this._maxRange := this._template.MaxRange ?: 1;
		else // Caso seja apenas um template
			this._attribute := this._template.Attribute;
			this._tipoAtaque := this._template.TipoAtaque;
			this._speedTier := this._template.EpicSpeed;
			this._speed := getSpeedByTier(this._speedTier);
			this._mainDamageTier := WeaponDamage(
				SplitWords(this._template.EpicMainDamage)[1],
				SplitWords(this._template.EpicMainDamage)[2],
				SplitWords(this._template.EpicMainDamage)[3]
			);
			this._offDamageTier := WeaponDamage(
				SplitWords(this._template.EpicOffDamage)[1],
				SplitWords(this._template.EpicOffDamage)[2],
				SplitWords(this._template.EpicOffDamage)[3]
			);
			this._mainDamage := WeaponDamage(
				getDamageByTier(SplitWords(this._template.EpicMainDamage)[1]) ?: 0,
				getDamageByTier(SplitWords(this._template.EpicMainDamage)[2]) ?: 0,
				getDamageByTier(SplitWords(this._template.EpicMainDamage)[3]) ?: 0
			);
			this._offDamage := WeaponDamage(
				getDamageByTier(SplitWords(this._template.EpicOffDamage)[1]) ?: 0,
				getDamageByTier(SplitWords(this._template.EpicOffDamage)[2]) ?: 0,
				getDamageByTier(SplitWords(this._template.EpicOffDamage)[3]) ?: 0
			);
			this._criticalChanceTier := this._template.EpicCriticalChance;
			this._criticalChance := getCriticalChanceByTier(this._criticalChanceTier);
			this._minRange := this._template.MinRange ?: 1;
			this._maxRange := this._template.MaxRange ?: 1;
		endif
	endfunction

	function GetAttribute(this)
		return this._attribute;
	endfunction

	function GetTipoAtaque(this)
		return this._tipoAtaque;
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
	
	function GetMainDamage(this)
		return this._mainDamage;
	endfunction

	function GetOffDamage(this)
		return this._offDamage;
	endfunction

	function GetCriticalChanceTier(this)
		return this._criticalChanceTier;
	endfunction

	function GetCriticalChance(this)
		return this._criticalChance;
	endfunction

	function GetMinRange(this)
		return this._minRange;
	endfunction

	function GetMaxRange(this)
		return this._maxRange;
	endfunction

	function SetSpeed(this, value)
		if (this._item == error) return this._item; endif
		this._speed := value;
	endfunction

	function SetMainDamage(this, slash, piercing, blunt)
		if (this._item == error) return this._item; endif
		this._mainDamage.SetSlashDamage(slash);
		this._mainDamage.SetPiercingDamage(piercing);
		this._mainDamage.SetBluntDamage(blunt);
	endfunction

	function SetOffDamage(this, slash, piercing, blunt)
		if (this._item == error) return this._item; endif
		this._offDamage.SetSlashDamage(slash);
		this._offDamage.SetPiercingDamage(piercing);
		this._offDamage.SetBluntDamage(blunt);
	endfunction

	function SetCriticalChance(this, value)
		if (this._item == error) return this._item; endif
		this._criticalChance := value;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif
		EquipWrapper::Update(this);

		this._prop.+Attribute := this._attribute;
		this._prop.+TipoAtaque := this._tipoAtaque;
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

		this._item.speed_mod := CInt(this._speed) - this._item.speed;
		
		this._item.SetProp(PROPNAME, this._prop);
	endfunction

	function ToString(this)
		return $"Weapon {EquipWrapper::ToString(this)},
		Attribute: {this._attribute}, Attack Type: {this._tipoAtaque},
		MainDamage (S/P/B): {this._mainDamage.GetSlashDamage()} ({this._mainDamageTier.GetSlashDamage()})/{this._mainDamage.GetPiercingDamage()} ({this._mainDamageTier.GetPiercingDamage()})/{this._mainDamage.GetBluntDamage()} ({this._mainDamageTier.GetBluntDamage()}),
		OffDamage (S/P/B): {this._offDamage.GetSlashDamage()} ({this._offDamageTier.GetSlashDamage()})/{this._offDamage.GetPiercingDamage()} ({this._offDamageTier.GetPiercingDamage()})/{this._offDamage.GetBluntDamage()} ({this._offDamageTier.GetBluntDamage()}),
		Speed: {this._speed} ({this._speedTier}), Critical Chance: {this._criticalChance}% ({this._criticalChanceTier}),
		Range: {this._minRange}-{this._maxRange}";
	endfunction

	// Private functions
	function _GetWeight(this)
		var maxMainDamage := Max(array{this._mainDamage.GetSlashDamage(), this._mainDamage.GetPiercingDamage(), this._mainDamage.GetBluntDamage()});
		var maxOffDamage := Max(array{this._offDamage.GetSlashDamage(), this._offDamage.GetPiercingDamage(), this._offDamage.GetBluntDamage()});

		this._weight := CInt(maxMainDamage + maxOffDamage) ?: 0;
	endfunction

	function _GetSTRRequired(this)
		this._strRequired := CInt(this._weight * 7.5) ?: 0;
	endfunction
endclass