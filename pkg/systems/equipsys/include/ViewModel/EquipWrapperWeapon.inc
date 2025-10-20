use math;

include "EquipWrapper";
include "IEquipWrapperWeapon";
include ":equipsys:Controller/WeaponController";

include ":combat:Model/DamageDice";
include ":diceroll:model/Dice";

class EquipWrapperWeapon(EquipWrapper, IEquipWrapperWeapon)
	var _attribute := "";
	var _attackType := "";
	var _category := "";
	var _speedTier := ""; // TODO: Transformar pra struct
	var _speed := 0; // TODO: Transformar pra struct
	var _speedModTier := ""; // TODO: Transformar pra struct
	var _speedMod := 0; // TODO: Transformar pra struct
	var _mainDamageTier := DamageDice(); // TODO: Transformar pra struct
	var _offDamageTier := DamageDice(); // TODO: Transformar pra struct
	var _mainDamage := DamageDice(); // TODO: Transformar pra struct
	var _offDamage := DamageDice(); // TODO: Transformar pra struct
	var _damageMod := DamageDice(); // TODO: Transformar pra struct
	var _criticalChanceTier := ""; // TODO: Transformar pra struct
	var _criticalChance := 0; // TODO: Transformar pra struct
	var _criticalChanceMod := 0;
	var _strRequired := 0;
	var _minRange := 1; // TODO: Transformar pra struct
	var _maxRange := 1; // TODO: Transformar pra struct

	function EquipWrapperWeapon(this, byRef item)
		Super(item);
		
		this._attribute := this._template.Attribute;
		this._attackType := this._template.TipoAtaque;
		this._category := this._template.Category;
		this._speedTier := this._template.EpicSpeed;
		this._minRange := this._template.MinRange ?: 1;
		this._maxRange := this._template.MaxRange ?: 1;
		this._criticalChanceTier := this._template.EpicCriticalChance;
		this._mainDamageTier := !this._template.EpicMainDamage ? DamageDice() : DamageDice(
			Dice(SplitWords(this._template.EpicMainDamage)[1]),
			Dice(SplitWords(this._template.EpicMainDamage)[2]),
			Dice(SplitWords(this._template.EpicMainDamage)[3])
		);
		this._offDamageTier := !this._template.EpicOffDamage ? DamageDice() : DamageDice(
			Dice(SplitWords(this._template.EpicOffDamage)[1]),
			Dice(SplitWords(this._template.EpicOffDamage)[2]),
			Dice(SplitWords(this._template.EpicOffDamage)[3])
		);
		this._mainDamage := DamageDice(
			Dice(getDamageByTier(this._mainDamageTier.GetSlashDamage())),
			Dice(getDamageByTier(this._mainDamageTier.GetPiercingDamage())),
			Dice(getDamageByTier(this._mainDamageTier.GetBluntDamage()))
		);
		this._offDamage := DamageDice(
			Dice(getDamageByTier(this._offDamageTier.GetSlashDamage())),
			Dice(getDamageByTier(this._offDamageTier.GetPiercingDamage())),
			Dice(getDamageByTier(this._offDamageTier.GetBluntDamage()))
		);
		this._speed := getSpeedByTier(this._speedTier);
		this._criticalChance := getCriticalChanceByTier(this._criticalChanceTier);
		this._weight := this._GetWeight();
		this._strRequired := this._GetSTRRequired();

		if (this._prop) // Caso já tenha prop, não sobrepor alguns valores que devem ser mantidos
			this._attackType := this._prop.AttackType;
			this._criticalChanceMod := this._prop.CriticalChanceMod;
		else
			this._criticalChanceMod := 0;
		endif
	endfunction

	function GetAttribute(this)
		return this._attribute;
	endfunction

	function GetAttackType(this)
		return this._attackType;
	endfunction

	function GetCategory(this)
		return this._category;
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

	function IsMontante(this)
		return this._item.objtype in array{0x26CF, 0xF9EA, 0xF9F7};
	endfunction

	function SetSpeed(this, value)
		if (this._item == error) return this._item; endif
		this._speed := value;

		return this;
	endfunction

	function SetMainDamage(this, slash, piercing, blunt)
		if (this._item == error) return this._item; endif
		this._mainDamage.SetSlashDamage(slash);
		this._mainDamage.SetPiercingDamage(piercing);
		this._mainDamage.SetBluntDamage(blunt);

		return this;
	endfunction

	function SetOffDamage(this, slash, piercing, blunt)
		if (this._item == error) return this._item; endif
		this._offDamage.SetSlashDamage(slash);
		this._offDamage.SetPiercingDamage(piercing);
		this._offDamage.SetBluntDamage(blunt);

		return this;
	endfunction

	function SetCriticalChance(this, value)
		if (this._item == error) return this._item; endif
		this._criticalChance := value;

		return this;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif
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

		this._item.speed_mod := CInt(this._speed) - this._item.speed;
		
		this._item.SetProp(PROPNAME, this._prop);
	endfunction

	function ToString(this)
		return "Weapon " + {EquipWrapper::ToString(this)} +
		$"\nAttribute: {this._attribute}, Attack Type: {this._attackType}" +
		$"\nMainDamage (S/P/B): {this._mainDamage.GetSlashDamage()} ({this._mainDamageTier.GetSlashDamage()})/{this._mainDamage.GetPiercingDamage()} ({this._mainDamageTier.GetPiercingDamage()})/{this._mainDamage.GetBluntDamage()} ({this._mainDamageTier.GetBluntDamage()})" +
		$"\nOffDamage (S/P/B): {this._offDamage.GetSlashDamage()} ({this._offDamageTier.GetSlashDamage()})/{this._offDamage.GetPiercingDamage()} ({this._offDamageTier.GetPiercingDamage()})/{this._offDamage.GetBluntDamage()} ({this._offDamageTier.GetBluntDamage()})" +
		$"\nSpeed: {this._speed} ({this._speedTier}), Critical Chance: {this._criticalChance}% ({this._criticalChanceTier})" +
		$"\nRange: {this._minRange}-{this._maxRange}";
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