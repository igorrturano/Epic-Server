use uo;
use cfgfile;

include "equipstats";

class EquipWrapper()
	var item;
	var template;

	function EquipWrapper(this, equipment)
		if (equipment.isA(POLCLASS_EQUIPMENT))
			this.item := equipment;
			this.template := equipment.getCfgElem();
		else
			this.template := FindConfigElem(ReadConfigFile(":*:itemdesc"), equipment);
		endif
	endfunction

	function GetDesc(this)
		return this.item ? this.item.desc : this.template.desc;
	endfunction

	function GetWeight(this)
		return this.item ? this.item.weight : this.template.weight;
	endfunction

	function GetMaxHP(this)
		return this.item ? this.item.maxHP : this.template.maxHP;
	endfunction

	function GetTipoProtecao(this)
		return this.template.TipoProtecao;
	endfunction

	function GetTipoAtaque(this)
		return this.template.TipoAtaque;
	endfunction

	function GetGraphic(this)
		return this.item ? this.item.graphic : this.template.graphic;
	endfunction

	function GetSpeed(this)
		return getSpeedByTier(this.template.EpicSpeed);
	endfunction

	function GetMainDamage(this)
		var damages := SplitWords(this.element.EpicMainDamage);

		return struct{
			Slash := getDamageByTier(damages[1]) ?: 0,
			Piercing := getDamageByTier(damages[2]) ?: 0,
			Blunt := getDamageByTier(damages[3]) ?: 0
		};
	endfunction

	function GetOffDamage(this)
		var damages := SplitWords(this.element.EpicOffDamage);

		return struct{
			Slash := getDamageByTier(damages[1]) ?: 0,
			Piercing := getDamageByTier(damages[2]) ?: 0,
			Blunt := getDamageByTier(damages[3]) ?: 0
		};
	endfunction

	function GetFireDamage(this)
		return getDamageElemental("FIRE", this.item) ?: 0;
	endfunction

	function GetColdDamage(this)
		return getDamageElemental("COLD", this.item) ?: 0;
	endfunction

	function GetPoisonDamage(this)
		return getDamageElemental("POISON", this.item) ?: 0;
	endfunction

	function GetEnergyDamage(this)
		return getDamageElemental("ENERGY", this.item) ?: 0;
	endfunction

	function GetCriticalChance(this)
		return getCriticalChanceByTier(this.template.EpicCriticalChance);
	endfunction

	function GetAttribute(this)
		return this.template.attribute;
	endfunction

	function GetMinRange(this)
		return this.template.MinRange;
	endfunction

	function GetMaxRange(this)
		return this.template.MaxRange;
	endfunction
endclass