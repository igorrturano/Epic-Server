use uo;
use cfgfile;

include "IEquipWrapper";

class EquipWrapper(IEquipWrapper)
	var _template;
	var _item;
	var _prop;

	var _name := "";
	var _desc := "";
	var _graphic := 0;
	var _weight := 0;
	var _maxHP := 0;

	function EquipWrapper(this, item)
		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			this._template := item.getCfgElem();
			this._item := item;
			this._prop := item.getAttrBase();

			this._name := this._template.name;
			this._desc := item.desc;
			this._weight := item.weight;
			this._maxHP := item.maxHP;
			this._graphic := item.graphic;
			
		else // Caso seja apenas um template
			this._template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);
			this._item := error{"errortext" := "This item is just a template!"};
			this._prop := error{"errortext" := "This item is just a template!"};

			this._name := this._template.name;
			this._desc := this._template.desc;
			this._weight := this._template.weight;
			this._maxHP := this._template.maxHP;
			this._graphic := this._template.graphic;
		endif
	endfunction

	function GetTemplate(this)
		return this._template;
	endfunction

	function GetItem(this)
		return this._item;
	endfunction

	function GetProp(this)
		return this._prop;
	endfunction

	function GetName(this)
		return this._name;
	endfunction

	function GetDesc(this)
		return this._desc;
	endfunction

	function GetGraphic(this)
		return this._graphic;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetMaxHP(this)
		return this._maxHP;
	endfunction

	function _SetName(this, name)
		this._name := name;
		if (this._item) this._item.name := name; endif
	endfunction

	function SetDesc(this, desc)
		this._desc := desc;
		if (this._item) this._item.desc := desc; endif
	endfunction

	function SetGraphic(this, graphic)
		this._graphic := graphic;
		if (this._item) this._item.graphic := graphic; endif
	endfunction

	function SetWeight(this, weight)
		this._weight := weight;
		if (this._item) this._item.weight := weight; endif
	endfunction

	function SetMaxHP(this, maxHP)
		this._maxHP := maxHP;
		if (this._item) this._item.maxHP := maxHP; endif
	endfunction

	function Update(this)
		this._item.name := this._name;
		this._item.desc := this._desc;
		this._item.graphic := this._graphic;
		this._item.weight := this._weight;
		this._item.maxHP := this._maxHP;
	endfunction

	function ToString(this)
		return $"Equipment:
		Name: {this.GetName()},
		Description: {this.GetDesc()},
		Graphic: {this.GetGraphic()} ({Hex(this.GetGraphic())}),
		Weight: {this.GetWeight()},
		MaxHP: {this.GetMaxHP()}";
	endfunction
endclass