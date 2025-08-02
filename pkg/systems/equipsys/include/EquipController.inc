use uo;
use cfgfile;

class EquipController()
	var itemRef;
	var template;

	function EquipController(this, equipment)
		if (equipment.isA(POLCLASS_EQUIPMENT))
			this.itemRef := equipment;
		else
			this.template := FindConfigElem(ReadConfigFile(":*:itemdesc"), equipment);
		endif
	endfunction

	function GetDesc(this)
		return this.itemRef ? this.itemRef.desc : this.template.desc;
	endfunction

	function GetWeight(this)
		return this.itemRef ? this.itemRef.weight : this.template.weight;
	endfunction

	function GetMaxHP(this)
		return this.itemRef ? this.itemRef.maxHP : this.template.maxHP;
	endfunction

	function GetTipoProtecao(this)
		return this.itemRef ? this.itemRef.TipoProtecao : this.template.TipoProtecao;
	endfunction

	function GetGraphic(this)
		return this.itemRef ? this.itemRef.graphic : this.template.graphic;
	endfunction

	function GetSpeed(this)
		return this.itemRef ? this.itemRef.speed : this.template.speed;
	endfunction

	function GetMainDamage(this)
		return this.itemRef ? this.itemRef.MainDamage : this.template.MainDamage;
	endfunction

	function GetOffDamage(this)
		return this.itemRef ? this.itemRef.OffDamage : this.template.OffDamage;
	endfunction

	function GetCriticalChance(this)
		return this.itemRef ? this.itemRef.CriticalChance : this.template.CriticalChance;
	endfunction

	function GetAttribute(this)
		return this.itemRef ? this.itemRef.attribute : this.template.attribute;
	endfunction

	function GetTipoAtaque(this)
		return this.itemRef ? this.itemRef.TipoAtaque : this.template.TipoAtaque;
	endfunction

	function GetMinRange(this)
		return this.itemRef ? this.itemRef.MinRange : this.template.MinRange;
	endfunction

	function GetMaxRange(this)
		return this.itemRef ? this.itemRef.MaxRange : this.template.MaxRange;
	endfunction
endclass