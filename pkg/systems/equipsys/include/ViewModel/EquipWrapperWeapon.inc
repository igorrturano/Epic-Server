include "EquipWrapper";
include "IEquipWrapperWeapon";
include ":equipsys:Controller/WeaponController";

class EquipWrapperWeapon(EquipWrapper, IEquipWrapperWeapon)
	var _attribute := "";
	var _tipoAtaque := "";
	var _speedTier := "";
	var _speed := 0;
	var _mainDamageTier := struct{};
	var _offDamageTier := struct{};
	var _mainDamage := struct{};
	var _offDamage := struct{};
	var _criticalChanceTier := "";
	var _criticalChance := 0;
	var _minRange := 0;
	var _maxRange := 0;
	var _penalties := struct{};

	function EquipWrapperWeapon(this, byRef item)
		Super(item);
		
		if (item.isA(POLCLASS_WEAPON)) // Caso o itemRef exista
			this._attribute := this._template.Attribute;
			this._tipoAtaque := item.getType();
			this._speedTier := this._template.EpicSpeed;
			this._speed := this._prop.AttackSpeed;
			this._mainDamage := struct {
				SlashDamage := this._prop.SlashMainDamage,
				PiercingDamage := this._prop.PiercingMainDamage,
				BluntDamage := this._prop.BluntMainDamage
			};
			this._offDamage := struct {
				SlashDamage := this._prop.SlashOffDamage,
				PiercingDamage := this._prop.PiercingOffDamage,
				BluntDamage := this._prop.BluntOffDamage
			};
			this._criticalChanceTier := this._template.EpicCriticalChance;
			this._criticalChance := this._prop.CriticalChance;
			this._minRange := this._template.MinRange ?: 1;
			this._maxRange := this._template.MaxRange ?: 1;
		else // Caso seja apenas um template
			this._attribute := this._template.Attribute;
			this._tipoAtaque := this._template.TipoAtaque;
			this._speedTier := this._template.EpicSpeed;
			this._speed := getSpeedByTier(this._speedTier);
			this._mainDamage := struct {
				SlashDamage := getDamageByTier(SplitWords(this._template.EpicMainDamage)[1]) ?: 0,
				PiercingDamage := getDamageByTier(SplitWords(this._template.EpicMainDamage)[2]) ?: 0,
				BluntDamage := getDamageByTier(SplitWords(this._template.EpicMainDamage)[3]) ?: 0
			};
			this._offDamage := struct {
				SlashDamage := getDamageByTier(SplitWords(this._template.EpicOffDamage)[1]) ?: 0,
				PiercingDamage := getDamageByTier(SplitWords(this._template.EpicOffDamage)[2]) ?: 0,
				BluntDamage := getDamageByTier(SplitWords(this._template.EpicOffDamage)[3]) ?: 0
			};
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

	function Update(this)
		if (this._item == error) return this._item; endif
		EquipWrapper::Update(this);

		this._item.speed_mod := CInt(this._speed) - this._item.speed;
		
		this._item.setAttrBase(this._prop);
	endfunction

	function ToString(this)
		return $"Weapon {EquipWrapper::ToString(this)},
		Attribute: {this.GetAttribute()}, Attack Type: {this.GetTipoAtaque()},
		MainDamage: {this.GetMainDamage().SlashDamage}/{this.GetMainDamage().PiercingDamage}/{this.GetMainDamage().BluntDamage},
		OffDamage: {this.GetOffDamage().SlashDamage}/{this.GetOffDamage().PiercingDamage}/{this.GetOffDamage().BluntDamage},
		Speed: {this.GetSpeed()}, Critical Chance: {this.GetCriticalChance()}%,
		Range: {this.GetMinRange()}-{this.GetMaxRange()}";
	endfunction
endclass