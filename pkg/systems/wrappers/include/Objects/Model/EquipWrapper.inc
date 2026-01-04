use uo;

include "system/exception/PropException/InvalidPropException";

include "IEquipWrapper";
include "ItemWrapper";
include ":IO:CFGFile";
include ":equipsys:utils/consts";

include ":combat:Model/AttributeModifier";
include ":equipsys:Controller/EquipmentController";

CONST PROPNAME_ITEMINFO := "itemInfo";

class EquipWrapper(ItemWrapper, IEquipWrapper)
	var Prop;
	var Layer := 0x0;
	var MaxHP := 0;
	var STRRequired := 0;
	var Material := "";
	var Modifiers := dictionary{}; // Dictionary of AttributeModifier()

	function EquipWrapper(this, byRef item)
		Super(item);

		if (this.ObjRef.isA(POLCLASS_EQUIPMENT)) // Caso o itemRef exista
			var err := InvalidTemplateException();
			this.Prop := this.GetProperty(PROPNAME_ITEMINFO) ?: InvalidPropException("This item has no Property!");
			this.Layer := this.ObjRef.layer;
			this.Graphic := this.ObjRef.graphic;
			this.Color := this.ObjRef.color;
			
			this.Name := this.Prop.Name ?: this.ObjRef.name;
			this.Weight := this.Prop.Weight ?: this.ObjRef.weight;
			this.Modifiers := dictionary{};
		else // Caso seja um template ou tipo errado
			var err;
			if (!this.Template)
				err := InvalidFormatException("The param must be a Equipment ObjRef or a valid Template!");
				this.Weight := err;
				this.Graphic := err;
			else
				err := IsTemplateException();
				this.Weight := this.Template.weight;
				this.Graphic := this.Template.graphic;
			endif
			
			this.Prop := err;
			this.Layer := err;
			this.Modifiers := err;
		endif
	endfunction

	function GetItemProp(this)
		return this.Prop;
	endfunction

	function SetItemProp(this, prop)
		this.Prop := prop;
		return this;
	endfunction

	function GetLayer(this)
		return this.Layer;
	endfunction

	function GetSTRRequired(this)
		return this.STRRequired;
	endfunction

	function GetMaterial(this)
		return this.Material;
	endfunction

	function GetMaxHP(this)
		return this.MaxHP;
	endfunction

	function GetModifier(this, modifierName)
		if (this.GetModifiers() is @Exception)
			return this.GetModifiers();
		endif

		if (this.GetModifiers().size() == 0) return error{"errortext" := "The item have no Modifiers!"}; endif

		return this.GetModifiers()[modifierName] ?: error{"errortext" := "This modifier does not exist!"};
	endfunction
	
	function GetModifiers(this)
		return this.Modifiers;
	endfunction

	function SetSTRRequired(this, strRequired)
		this.STRRequired := CInt(strRequired);

		return this;
	endfunction

	function SetMaterial(this, material)
		this.Material := material;

		return this;
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
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		if (!(modifier is @AttributeModifier))
			return InvalidFormatException("The param must be an AttributeModifier object!");
		endif

		this.GetModifiers()[modifier.GetAttribute()] := modifier;

		return this;
	endfunction

	function Update(this)
		if (this.GetObjRef() is @Exception)
			return this.GetObjRef();
		endif

		var prop := struct{
			Name := this.GetName(),
			Graphic := this.GetGraphic(),
			Weight := this.GetWeight(),
			MaxHP := this.GetMaxHP()
		};
		this.SetItemProp(prop);

		this.SetName(this.GetName());
		this.GetObjRef().graphic := this.GetGraphic();
		this.GetObjRef().weight := this.GetWeight();
		this.GetObjRef().maxHP := this.GetMaxHP();
		this.GetObjRef().max_weight_mod := this.GetWeight() - this.GetObjRef().weight;

		this.Prop.+Modifiers := dictionary{};
	endfunction

	function Equals(this, otherEquipWrapper)
		if (!(otherEquipWrapper is @EquipWrapper))
			return error {"errortext" := "The param must be an EquipWrapper!"};
		endif

		return this.GetObjType() == otherEquipWrapper.GetObjType();
	endfunction

	function ToString(this)
		return Super::ToString(this) + " >>> Equipment: " +
			$"Name: {this.GetName()}, " +
			$"Layer: {this.GetLayer()} ({Hex(this.GetLayer())}), " +
			$"Graphic: {this.GetGraphic()} ({Hex(this.GetGraphic())}), " +
			$"Color: {this.GetColor()} ({this.GetColor()}), " +
			$"Weight: {this.GetWeight()}, " +
			$"MaxHP: {this.GetMaxHP()}";
	endfunction
endclass
