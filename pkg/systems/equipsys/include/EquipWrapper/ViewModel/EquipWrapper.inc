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

	function EquipWrapper(this, byRef item)
		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			this._template := item.getCfgElem();
			this._item := item;
			this._prop := item.getAttrBase();

			this._name := item.name;
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
		if (this._item == error) return this._item; endif
		this._name := name;
	endfunction

	function SetDesc(this, desc)
		if (this._item == error) return this._item; endif
		this._desc := desc;
	endfunction

	function SetGraphic(this, graphic)
		if (this._item == error) return this._item; endif
		this._graphic := graphic;
	endfunction

	function SetWeight(this, weight)
		if (this._item == error) return this._item; endif
		this._weight := weight;
	endfunction

	function SetMaxHP(this, maxHP)
		if (this._item == error) return this._item; endif
		this._maxHP := maxHP;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif

		this._item.name := this._name;
		this._item.desc := this._desc;
		this._item.graphic := this._graphic;
		this._item.weight := this._weight;
		this._item.maxHP := this._maxHP;
	endfunction

	function ToString(this)
		return $"Equipment:
		Name: {this._name},
		Description: {this._desc},
		Graphic: {this._graphic} ({Hex(this._graphic)}),
		Weight: {this._weight},
		MaxHP: {this._maxHP}";
	endfunction
endclass