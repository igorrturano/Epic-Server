use uo;

include "IEquipWrapper";
include ":IO:CFGFile";

include ":combat:Model/AttributeModifier";

const PROPNAME := "atributos";

class EquipWrapper(IEquipWrapper)
	var _template; // CfgElem
	var _itemRef; // ItemRef
	var _prop;

	var _name := "";
	var _objType := 0x0;
	var _graphic := 0;
	var _color := 0;
	var _weight := 0;
	var _maxHP := 0;
	var _material := "";

	var _modifiers := dictionary{}; // Dictionary of AttributeModifier()

	function EquipWrapper(this, byRef item)
		if (item.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			this._template := item.getCfgElem();
			this._itemRef := item;
			this._prop := item.GetProp(PROPNAME) ?: error{"errortext" := "This item has no attributes Property!"};
			this._objType := item.objtype;
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
			this._itemRef := error{"errortext" := "This item is just a template!"};
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

	function GetItemRef(this)
		return this._itemRef;
	endfunction

	function GetItemProp(this)
		return this._prop;
	endfunction

	function GetName(this)
		return this._name;
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
		if (this._itemRef == error) return this._itemRef; endif
		this._name := name;
		return this;
	endfunction

	function SetGraphic(this, graphic)
		if (this._itemRef == error) return this._itemRef; endif
		this._graphic := graphic;
		return this;
	endfunction

	function SetWeight(this, weight)
		if (this._itemRef == error) return this._itemRef; endif
		this._weight := weight;
		return this;
	endfunction

	function SetMaxHP(this, maxHP)
		if (this._itemRef == error) return this._itemRef; endif
		this._maxHP := maxHP;
		return this;
	endfunction

	function SetMaterial(this, material)
		if (this._itemRef == error) return this._itemRef; endif
		this._material := material;
		return this;
	endfunction

	function AddModifier(this, modifier) // modifier is of type AttributeModifier
		if (this._itemRef == error) return this._itemRef; endif
		this._modifiers[modifier.GetAttribute()] := modifier;
		return this;
	endfunction

	function Update(this)
		if (this._itemRef == error) return this._itemRef; endif

		this._prop := struct{};
		this._prop.+Name := this._name;
		this._prop.+Graphic := this._graphic;
		this._prop.+Weight := this._weight;
		this._prop.+MaxHP := this._maxHP;

		this._SetName(this._name);
		this._itemRef.graphic := this._graphic;
		this._itemRef.weight := this._weight;
		this._itemRef.maxHP := this._maxHP;

		this._prop.+Modifiers := dictionary{};
	endfunction

	function Equals(this, otherEquipWrapper)
		// if (!(otherEquipWrapper is @EquipWrapper))
		// 	return error {"errortext" := "The param must be an EquipWrapper!"};
		// endif

		return this._itemRef == otherEquipWrapper.GetItemRef();
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

enum class EQUIP_SLOT
	HEAD,
	NECK,
	BACK,
	CHEST,
	HANDS,
	LEGS,
	FEET,
	WEAPON,
	SHIELD,
	MAINHAND,
	OFFHAND,
	ACCESSORY_1,
	ACCESSORY_2
endenum

enum class EQUIP_LAYER
	NONE,
	HAND1,
	HAND2,
	SHOES,
	PANTS,
	SHIRT,
	HELM,
	GLOVES,
	RING,
	UNUSED9,
	NECK,
	HAIR,
	WAIST,
	CHEST,
	WRIST,
	PACK2,
	BEARD,
	TUNIC,
	EARS,
	ARMS,
	CAPE,
	PACK,
	ROBE,
	SKIRT,
	LEGS,
	MOUNT
endenum