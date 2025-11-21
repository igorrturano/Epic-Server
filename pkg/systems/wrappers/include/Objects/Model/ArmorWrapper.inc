include "EquipWrapper";
include "IArmorWrapper";
include ":equipsys:Controller/ArmorController";

include ":combat:Model/ArmorResist";
include ":combat:Model/ShieldAttributes";

class ArmorWrapper(EquipWrapper, IArmorWrapper)
	var _coverage := array{};
	var _tipoProtecao := "";

	var _resists; // ArmorResist class
	var _resistsMod; // ArmorResist class
	var _shieldAttributes; // ShieldAttributes class
	var _shieldAttributesMod; // ShieldAttributes class

	function ArmorWrapper(this, byRef item)
		Super(item);

		if (!this._template)
			var err := InvalidTemplateException("The param must be an Armor ObjRef or a valid Template!");
			this._coverage := err;
			this._tipoProtecao := err;
			this._resists := err;
			this._resistsMod := err;
			this._shieldAttributes := err;
			this._shieldAttributesMod := err;
		else
			this._coverage := GetConfigStringArray(this._template, "Coverage");
			this._tipoProtecao := this._template.TipoProtecao ?: error{ errortext := "The item have no Protection Type!" };
			this._resists := this._GetArmorResist();
			this._resistsMod := ArmorResist();
			this._weight := this._GetWeight();
			this._strRequired := this._GetSTRRequired();
			this._InitModifiers();

			if (this._prop)
				this._resistsMod.SetSlashResist(this._prop.SlashResistMod);
				this._resistsMod.SetPiercingResist(this._prop.PiercingResistMod);
				this._resistsMod.SetBluntResist(this._prop.BluntResistMod);
				this._resistsMod.SetFireResist(this._prop.FireResistMod);
				this._resistsMod.SetColdResist(this._prop.ColdResistMod);
				this._resistsMod.SetPoisonResist(this._prop.PoisonResistMod);
				this._resistsMod.SetEnergyResist(this._prop.EnergyResistMod);
			endif
		endif
	endfunction
	
	function GetTipoProtecao(this)
		return this._tipoProtecao;
	endfunction

	function GetCoverage(this)
		return this._coverage;
	endfunction

	function GetResists(this)
		var base := this.GetResistsBase();
		var mod := this.GetResistsMod();

		if (base is @Exception)
			return base;
		endif

		return ArmorResist(
			CInt(base.GetSlashResist()) + CInt(mod.GetSlashResist()),
			CInt(base.GetPiercingResist()) + CInt(mod.GetPiercingResist()),
			CInt(base.GetBluntResist()) + CInt(mod.GetBluntResist()),
			CInt(base.GetFireResist()) + CInt(mod.GetFireResist()),
			CInt(base.GetColdResist()) + CInt(mod.GetColdResist()),
			CInt(base.GetEnergyResist()) + CInt(mod.GetEnergyResist()),
			CInt(base.GetPoisonResist()) + CInt(mod.GetPoisonResist())
		);
	endfunction

	function GetResistsBase(this)
		return this._resists;
	endfunction

	function GetResistsMod(this)
		return this._resistsMod;
	endfunction

	function GetShieldAttributes(this)
		var base := this.GetShieldAttributesBase();
		var mod := this.GetShieldAttributesMod();

		if (base is @Exception)
			return base;
		endif

		return ShieldAttributes(
			CInt(base.GetParryBonus()) + CInt(mod.GetParryBonus()),
			CInt(base.GetParryDelay()) + CInt(mod.GetParryDelay())
		);
	endfunction
	
	function GetShieldAttributesBase(this)
		return this._shieldAttributes;
	endfunction

	function GetShieldAttributesMod(this)
		return this._shieldAttributesMod;
	endfunction

	function SetResistsBase(this, resists)
		if (this._objRef is @Exception) return this._objRef; endif
		this._resists := resists;

		return this;
	endfunction

	function SetResistsMod(this, resists)
		if (this._objRef is @Exception) return this._objRef; endif
		this._resistsMod := resists;

		return this;
	endfunction

	function SetShieldAttributesBase(this, shieldAttrs)
		if (this._objRef is @Exception) return this._objRef; endif
		if (!(shieldAttrs is @ShieldAttributes)) return error{ errortext := "Param must be a ShieldAttributes!" }; endif

		this._shieldAttributes := shieldAttrs;

		return this;
	endfunction

	function SetShieldAttributesMod(this, shieldAttrs)
		if (this._objRef is @Exception) return this._objRef; endif
		if (!(shieldAttrs is @ShieldAttributes)) return error{ errortext := "Param must be a ShieldAttributes!" }; endif

		this._shieldAttributesMod := shieldAttrs;

		return this;
	endfunction

	function Update(this)
		if (this._objRef is @Exception) return this._objRef; endif
		EquipWrapper::Update(this);

		this._prop.+SlashResist := this._resists.GetSlashResist();
		this._prop.+PiercingResist := this._resists.GetPiercingResist();
		this._prop.+BluntResist := this._resists.GetBluntResist();
		this._prop.+FireResist := this._resists.GetFireResist();
		this._prop.+ColdResist := this._resists.GetColdResist();
		this._prop.+EnergyResist := this._resists.GetEnergyResist();
		this._prop.+PoisonResist := this._resists.GetPoisonResist();
		this._InitModifiers();
		foreach modifier in (this._modifiers)
			this._prop.Modifiers[modifier.GetAttribute()] := modifier.GetValue();
		endforeach

		this._item.SetProp(PROPNAME_ITEMINFO, this._prop);
	endfunction

	function ToString(this)
		var str := Super::ToString(this) + "Armor:" +
			$"\nCoverage: {this._coverage}, Protection Type: {this._tipoProtecao}" +
			$"\nSlashResist: {this._resists.GetSlashResist()} (+{this._resistsMod.GetSlashResist()})" +
			$"\nPiercingResist: {this._resists.GetPiercingResist()} (+{this._resistsMod.GetPiercingResist()})" +
			$"\nBluntResist: {this._resists.GetBluntResist()} (+{this._resistsMod.GetBluntResist()})" +
			
			"\n\nModifiers:";

		foreach modifier in (this._modifiers.values())
			str += $"\n{modifier.ToString()}, ";
		endforeach

		return str;
	endfunction

	// Private functions
	function _GetWeight(this)
		var weight := 0;
		var prot := SplitWords(this._tipoProtecao);
		
		if (Upper(prot[1]) == "ESCUDO") // Se for escudo, peso é o DEX Penalty
			weight := -(this._modifiers[DEXTERITY].GetValue()) ?: 0;
		else // Se não for, é a soma do maior valor de resistência + resistência por slot
			weight := Max(array{this._resists.GetSlashResist(), this._resists.GetPiercingResist(), this._resists.GetBluntResist()});
			weight += CInt(getResistPhysicalBySlot(this._coverage));
		endif

		return weight;
	endfunction

	function _GetSTRRequired(this)
		return CInt(this._weight * 7.5);
	endfunction

	function _GetArmorResist(this)
		var templateResists := getResistPhysicalByProtection(this._template.tipoProtecao);
		var slotResists := this._tipoProtecao ? getResistPhysicalBySlot(this._coverage) : ArmorResist();

		return ArmorResist(
			templateResists.GetSlashResist() + slotResists.GetSlashResist(),
			templateResists.GetPiercingResist() + slotResists.GetPiercingResist(),
			templateResists.GetBluntResist() + slotResists.GetBluntResist(),
			this._template.FireResist,
			this._template.ColdResist,
			this._template.PoisonResist,
			this._template.EnergyResist
		);
	endfunction

	function _InitModifiers(this)
		this._modifiers := dictionary{};
		
		if (this._prop.Modifiers.size() >= 1)
			foreach modifier in (this._prop.Modifiers.keys())
				this.SetModifier(AttributeModifier(modifier, this._prop.Modifiers[modifier]));
			endforeach
		else
			if (this._tipoProtecao)
				var modifiersSlot := GetModifiersBySlot(this._coverage[1]);
				var modifiersProt := GetModifiersByProtection(this._tipoProtecao);
				this.SetModifier(AttributeModifier(DEXTERITY, modifiersSlot[DEXTERITY].GetValue() + modifiersProt[DEXTERITY].GetValue()));
				this.SetModifier(AttributeModifier(SNEAK, modifiersSlot[SNEAK].GetValue() + modifiersProt[SNEAK].GetValue()));
				this.SetModifier(AttributeModifier(MECHANICS, modifiersSlot[MECHANICS].GetValue() + modifiersProt[MECHANICS].GetValue()));
				this.SetModifier(AttributeModifier(MAGERY, modifiersSlot[MAGERY].GetValue() + modifiersProt[MAGERY].GetValue()));
				this.SetModifier(AttributeModifier(AWARENESS, modifiersSlot[AWARENESS].GetValue() + modifiersProt[AWARENESS].GetValue()));
			endif
		endif
	endfunction
endclass