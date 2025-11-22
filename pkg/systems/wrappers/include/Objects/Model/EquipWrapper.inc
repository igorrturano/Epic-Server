use uo;

include "system/exception/PropException/InvalidPropException";

include "IEquipWrapper";
include "ItemWrapper";
include ":IO:CFGFile";
include ":equipsys:utils/consts";

include ":combat:Model/AttributeModifier";

CONST PROPNAME_ITEMINFO := "itemInfo";

class EquipWrapper(ItemWrapper, IEquipWrapper)
	var _prop;
	var _layer := 0x0;
	var _weight := 0;
	var _maxHP := 0;
	var _modifiers := dictionary{}; // Dictionary of AttributeModifier()

	function EquipWrapper(this, byRef item)
		Super(item);

		if (this._objRef.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			var err := InvalidTemplateException();
			this._prop := this._objRef.GetProp(PROPNAME_ITEMINFO) ?: InvalidPropException("This item has no Property!");
			this._layer := this._objRef.layer;
			this._graphic := this._objRef.graphic;
			this._color := this._objRef.color;
			
			this._name := this._prop.Name ?: this._objRef.name;
			this._weight := this._prop.Weight ?: this._objRef.weight;
			this._maxHP := this._prop.MaxHP ?: this._objRef.maxHP;
			this._modifiers := dictionary{};
		else // Caso seja um template ou tipo errado
			var err;
			if (!this._template)
				err := InvalidFormatException("The param must be a Equipment ObjRef or a valid Template!");
				this._weight := err;
				this._maxHP := err;
				this._graphic := err;
			else
				err := IsTemplateException();
				this._weight := this._template.weight;
				this._maxHP := this._template.maxHP;
				this._graphic := this._template.graphic;
			endif
			
			this._prop := err;
			this._layer := err;
			this._modifiers := err;
		endif
	endfunction

	function GetItemProp(this)
		return this._prop;
	endfunction

	function GetLayer(this)
		return this._layer;
	endfunction

	function GetWeight(this)
		return this._weight;
	endfunction

	function GetMaxHP(this)
		return this._maxHP;
	endfunction

	function GetModifier(this, modifierName)
		if (this.GetModifiers() is @Exception)
			return this.GetModifiers();
		endif

		if (this._modifiers.size() == 0) return error{"errortext" := "The item have no Modifiers!"}; endif

		return this._modifiers[modifierName] ?: error{"errortext" := "This modifier does not exist!"};
	endfunction
	
	function GetModifiers(this)
		return this._modifiers;
	endfunction

	/**
	 * Define um modificador para o objeto.
	 * 
	 * @param modifier O modificador a ser definido (deve ser um objeto AttributeModifier)
	 * 
	 * @return this Retorna a própria instância para encadeamento de métodos, ou
	 *              Exception se _objRef for uma exceção, ou
	 *              InvalidFormatException se o parâmetro não for um AttributeModifier
	 */
	function SetModifier(this, modifier)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

		if (!(modifier is @AttributeModifier))
			return InvalidFormatException("The param must be an AttributeModifier object!");
		endif

		this._modifiers[modifier.GetAttribute()] := modifier;

		return this;
	endfunction

	function Update(this)
		if (this._objRef is @Exception)
			return this._objRef;
		endif

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
			$"Layer: {this._layer} ({Hex(this._layer)}), " +
			$"Graphic: {this._graphic} ({Hex(this._graphic)}), " +
			$"Color: {this._color} ({this._color}), " +
			$"Weight: {this._weight}, " +
			$"MaxHP: {this._maxHP}";
	endfunction
endclass
