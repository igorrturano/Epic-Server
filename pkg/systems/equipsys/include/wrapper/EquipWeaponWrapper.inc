include "EquipWrapper";

class EquipWeaponWrapper(EquipWrapper)
	var _attribute; // String
	var _tipoAtaque; // String
	var _speed; // Int
	var _mainDamage; // Struct
	var _offDamage; // Struct
	var _criticalChance; // Int
	var _minRange; // Int
	var _maxRange; // Int
	var _penalties; // Struct

	function EquipWeaponWrapper(this, item)
		var template, attribute, tipoAtaque, speed, mainDamage, offDamage, criticalChance, minRange, maxRange;
		Super(item);
		
		if (item.isA(POLCLASS_WEAPON))
			template := item.getCfgElem();
			this._attribute := template.Attribute;
			this._tipoAtaque := item.getType();
			this._speed := template.EpicSpeed;
			this._mainDamage := struct {
				SlashDamage := item.getAttrBase().SlashMainDamage,
				PiercingDamage := item.getAttrBase().PiercingMainDamage,
				BluntDamage := item.getAttrBase().BluntMainDamage
			};
			this._offDamage := struct {
				SlashDamage := item.getAttrBase().SlashOffDamage,
				PiercingDamage := item.getAttrBase().PiercingOffDamage,
				BluntDamage := item.getAttrBase().BluntOffDamage
			};
			this._criticalChance := template.EpicCriticalChance;
			this._minRange := template.MinRange ?: 1;
			this._maxRange := template.MaxRange ?: 1;

		else
			template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);
			this._attribute := template.Attribute;
			this._tipoAtaque := template.TipoAtaque;
			this._speed := template.EpicSpeed;
			this._mainDamage := struct {
				SlashDamage := getDamageByTier(SplitWords(template.EpicMainDamage)[1]) ?: 0,
				PiercingDamage := getDamageByTier(SplitWords(template.EpicMainDamage)[2]) ?: 0,
				BluntDamage := getDamageByTier(SplitWords(template.EpicMainDamage)[3]) ?: 0
			};
			this._offDamage := struct {
				SlashDamage := getDamageByTier(SplitWords(template.EpicOffDamage)[1]) ?: 0,
				PiercingDamage := getDamageByTier(SplitWords(template.EpicOffDamage)[2]) ?: 0,
				BluntDamage := getDamageByTier(SplitWords(template.EpicOffDamage)[3]) ?: 0
			};
			this._criticalChance := template.EpicCriticalChance;
			this._minRange := template.MinRange ?: 1;
			this._maxRange := template.MaxRange ?: 1;
		endif
	endfunction

	function getAttribute(this)
		return this._attribute;
	endfunction

	function getTipoAtaque(this)
		return this._tipoAtaque;
	endfunction

	function GetSpeed(this)
		return getSpeedByTier(this._speed);
	endfunction

	function GetMainDamage(this)
		return this._mainDamage;
	endfunction

	function GetOffDamage(this)
		return this._offDamage;
	endfunction

	function GetCriticalChance(this)
		return getCriticalChanceByTier(this._criticalChance);
	endfunction

	function GetMinRange(this)
		return this._minRange;
	endfunction

	function GetMaxRange(this)
		return this._maxRange;
	endfunction


	function ToString(this)
		return $"Weapon {EquipWrapper::ToString(this)},
		Attribute: {this.getAttribute()}, Attack Type: {this.getTipoAtaque()},
		MainDamage: {this.GetMainDamage().SlashDamage}/{this.GetMainDamage().PiercingDamage}/{this.GetMainDamage().BluntDamage},
		OffDamage: {this.GetOffDamage().SlashDamage}/{this.GetOffDamage().PiercingDamage}/{this.GetOffDamage().BluntDamage},
		Speed: {this.GetSpeed()}, Critical Chance: {this.GetCriticalChance()}%,
		Range: {this.GetMinRange()}-{this.GetMaxRange()}";
	endfunction
endclass