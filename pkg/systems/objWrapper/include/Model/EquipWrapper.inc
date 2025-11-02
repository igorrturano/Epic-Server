use uo;

include "IEquipWrapper";
include "ItemWrapper";
include ":IO:CFGFile";
include ":equipsys:utils/consts";

include ":combat:Model/AttributeModifier";

class EquipWrapper(ItemWrapper, IEquipWrapper)
	var _template; // CfgElem
	var _prop;

	var _weight := 0;
	var _maxHP := 0;
	var _material := "";

	var _modifiers := dictionary{}; // Dictionary of AttributeModifier()

	function EquipWrapper(this, byRef item)
		Super(item);

		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			this._template := item.getCfgElem();
			this._prop := item.GetProp("atributos") ?: error{"errortext" := "This item has no attributes Property!"};
			this._graphic := this._template.graphic;
			this._color := this._template.color;
			
			if (this._prop)
				this._name := this._prop.Name;
				this._weight := this._prop.Weight;
				this._maxHP := this._prop.MaxHP;
			else
				this._name := item.name;
				this._weight := item.weight;
				this._maxHP := item.maxHP;
			endif
			
		else // Caso seja apenas um template
			this._template := CFGFile("*", "itemdesc").SetElementName(item).GetElementValue();
			this._objType := error{"errortext" := "This item is just a template!"};
			this._prop := error{"errortext" := "This item is just a template!"};

			this._name := this._template.desc;
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

	function GetItemProp(this)
		return this._prop;
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
		if (this._objType == error) return this._objType; endif
		Super::_SetName(this, name);
		return this;
	endfunction

	function SetGraphic(this, graphic)
		if (this._objType == error) return this._objType; endif
		Super::SetGraphic(this, graphic);
		return this;
	endfunction

	function SetWeight(this, weight)
		if (this._objType == error) return this._objType; endif
		this._weight := weight;
		return this;
	endfunction

	function SetMaxHP(this, maxHP)
		if (this._objType == error) return this._objType; endif
		this._maxHP := maxHP;
		return this;
	endfunction

	function SetMaterial(this, material)
		if (this._objType == error) return this._objType; endif
		this._material := material;
		return this;
	endfunction

	function SetModifiers(this, modifiers) // modifiers: dictionary of AttributeModifier
		if (this._objType == error) return this._objType; endif
		this._modifiers := modifiers;
		return this;
	endfunction

	function AddModifier(this, modifier) // modifier: AttributeModifier
		if (this._objType == error) return this._objType; endif
		this._modifiers[modifier.GetAttribute()] := modifier;
		return this;
	endfunction

	function Update(this)
		if (this._objType == error) return this._objType; endif

		this._prop := struct{};
		this._prop.+Name := this._name;
		this._prop.+Graphic := this._graphic;
		this._prop.+Weight := this._weight;
		this._prop.+MaxHP := this._maxHP;

		this._SetName(this._name);
		this._objType.graphic := this._graphic;
		this._objType.weight := this._weight;
		this._objType.maxHP := this._maxHP;

		this._prop.+Modifiers := dictionary{};
	endfunction

	function Equals(this, otherEquipWrapper)
		if (!(otherEquipWrapper is @EquipWrapper))
			return error {"errortext" := "The param must be an EquipWrapper!"};
		endif

		return this._objType == otherEquipWrapper.GetObjType();
	endfunction

	function ToString(this)
		return Super::ToString(this) + " >>> Equipment: " +
			$"Name: {this._name}, " +
			$"Graphic: {this._graphic} ({Hex(this._graphic)}), " +
			$"Weight: {this._weight}, " +
			$"MaxHP: {this._maxHP}";
	endfunction
endclass
