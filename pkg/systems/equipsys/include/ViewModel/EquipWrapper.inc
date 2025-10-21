use uo;

include "IEquipWrapper";
include ":IO:CFGFile";

include ":combat:Model/AttributeModifier";

const PROPNAME := "atributos";

class EquipWrapper(IEquipWrapper)
	var _template; // CfgElem
	var _item; // ItemRef
	var _prop;

	var _name := "";
	var _graphic := 0;
	var _weight := 0;
	var _maxHP := 0;
	var _material := "";

	var _modifiers := dictionary{}; // Dictionary of AttributeModifier()

	function EquipWrapper(this, byRef item)
		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			this._template := item.getCfgElem();
			this._item := item;
			this._prop := item.GetProp(PROPNAME) ?: error{"errortext" := "This item has no attributes Property!"};
			
			if (this._prop)
				this._name := this._prop.Name;
				this._weight := this._prop.Weight;
				this._maxHP := this._prop.MaxHP;
				this._graphic := this._prop.Graphic;
			else
				this._name := item.name;
				this._weight := item.weight;
				this._maxHP := item.maxHP;
				this._graphic := item.graphic;
			endif

			this._material := this._template.Material ?: error{"errortext" := "The item have no Material!"};
			
		else // Caso seja apenas um template
			this._template := CFGFile("*", "itemdesc").SetElementName(item).GetElementValue();
			this._item := error{"errortext" := "This item is just a template!"};
			this._prop := error{"errortext" := "This item is just a template!"};

			this._name := this._template.name;
			this._desc := this._template.desc;
			this._weight := this._template.weight;
			this._maxHP := this._template.maxHP;
			this._graphic := this._template.graphic;

			this._material := this._template.Material ?: error{"errortext" := "This item is just a template and have no Material!"};
		endif

		this._modifiers := dictionary{};
	endfunction

	function GetTemplate(this)
		return this._template;
	endfunction

	function GetItem(this)
		return this._item ?: error{"errortext" := "This item is just a template!"};
	endfunction

	function GetProp(this)
		return this._prop ?: error{"errortext" := "This item has no attributes Property or is just a template!"};
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

	function GetMaterial(this)
		return this._material ?: error{"errortext" := "The item have no Material!"};
	endfunction

	function GetModifier(this, modifierName)
		if ( this._modifiers.size() == 0 ) return error{"errortext" := "The item have no Modifiers!"}; endif
		return this._modifiers[modifierName] ?: error{"errortext" := "This modifier does not exist!"};
	endfunction
	
	function GetModifiers(this)
		return this._modifiers ?: error{"errortext" := "The item have no Modifiers!"};
	endfunction

	function _SetName(this, name)
		if (this._item == error) return this._item; endif
		this._name := name;
		return this;
	endfunction

	function SetDesc(this, desc)
		if (this._item == error) return this._item; endif
		this._desc := desc;
		return this;
	endfunction

	function SetGraphic(this, graphic)
		if (this._item == error) return this._item; endif
		this._graphic := graphic;
		return this;
	endfunction

	function SetWeight(this, weight)
		if (this._item == error) return this._item; endif
		this._weight := weight;
		return this;
	endfunction

	function SetMaxHP(this, maxHP)
		if (this._item == error) return this._item; endif
		this._maxHP := maxHP;
		return this;
	endfunction

	function SetMaterial(this, material)
		if (this._item == error) return this._item; endif
		this._material := material;
		return this;
	endfunction

	function AddModifier(this, modifier) // modifier is of type AttributeModifier
		if (this._item == error) return this._item; endif
		this._modifiers[modifier.GetAttribute()] := modifier;
		return this;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif

		this._prop := struct{};
		this._prop.+Name := this._name;
		this._prop.+Graphic := this._graphic;
		this._prop.+Weight := this._weight;
		this._prop.+MaxHP := this._maxHP;

		this._SetName(this._name);
		this._item.graphic := this._graphic;
		this._item.weight := this._weight;
		this._item.maxHP := this._maxHP;

		this._prop.+Modifiers := dictionary{};
	endfunction

	function ToString(this)
		return $"Equipment:
		Name: {this._name},
		Graphic: {this._graphic} ({Hex(this._graphic)}),
		Weight: {this._weight},
		MaxHP: {this._maxHP}";
	endfunction

	uninit function _GetWeight(this);
	uninit function _GetSTRRequired(this);
endclass

enum class EQUIPSLOT
	HEAD,
	NECK,
	BACK,
	CHEST,
	HANDS,
	LEGS,
	FEET,
	WEAPON,
	SHIELD,
	ACCESSORY_1,
	ACCESSORY_2
endenum