include "EquipWrapper";
include "IEquipWrapperArmor";
include ":equipsys:Controller/ArmorController";

include ":combat:Model/ArmorResist";

class EquipWrapperArmor(EquipWrapper, IEquipWrapperArmor)
	var _coverage := array{};
	var _tipoProtecao := "";

	var _resists := ArmorResist();
	var _resistsMod := ArmorResist();

	function EquipWrapperArmor(this, byRef item)
		Super(item);

		this._coverage := GetConfigStringArray(this._template, "Coverage");
		this._tipoProtecao := this._template.TipoProtecao ?: error{"errortext" := "The item have no Protection Type!"};
		this._resists := this._GetArmorResist();
		this._weight := this._GetWeight();
		this._strRequired := this._GetSTRRequired();
		this._AddModifiers();

		if (this._prop)
			this._resistsMod := ArmorResist(
				CInt(this._prop.SlashResistMod),
				CInt(this._prop.PiercingResistMod),
				CInt(this._prop.BluntResistMod),
				CInt(this._prop.FireResistMod),
				CInt(this._prop.ColdResistMod),
				CInt(this._prop.PoisonResistMod),
				CInt(this._prop.EnergyResistMod)
			);
		else // Caso não tenha prop ou seja um item não existente (template)
			this._resistsMod := ArmorResist(); // TODO: Não tem mod, é tudo 0
		endif
	endfunction
	
	function GetTipoProtecao(this)
		return this._tipoProtecao ?: error{"errortext" := "The item have no Protection Type!"};
	endfunction

	function GetCoverage(this)
		return this._coverage ?: error{"errortext" := "The item have no Coverage!"};
	endfunction

	function GetResists(this)
		return this._resists ?: error{"errortext" := "The item have no Resists!"};
	endfunction

	function GetResistsMod(this)
		return this._resistsMod ?: error{"errortext" := "The item have no Resists Modifier!"};
	endfunction
	
	function SetResists(this, resists)
		if (this._item == error) return this._item; endif
		this._resists := resists;
	endfunction

	function SetResistsMod(this, resists)
		if (this._item == error) return this._item; endif
		this._resistsMod := resists;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif
		EquipWrapper::Update(this);

		this._prop.+SlashResist := this._resists.GetSlashResist();
		this._prop.+PiercingResist := this._resists.GetPiercingResist();
		this._prop.+BluntResist := this._resists.GetBluntResist();
		this._prop.+FireResist := this._resists.GetFireResist();
		this._prop.+ColdResist := this._resists.GetColdResist();
		this._prop.+EnergyResist := this._resists.GetEnergyResist();
		this._prop.+PoisonResist := this._resists.GetPoisonResist();
		this._AddModifiers();
		foreach modifier in (this._modifiers)
			this._prop.Modifiers[modifier.GetAttribute()] := modifier.GetValue();
		endforeach

		this._AddModifiers();
		foreach modifier in (this._modifiers)
			this._prop.Modifiers[modifier.GetAttribute()] := modifier.GetValue();
		endforeach

		this._item.SetProp(PROPNAME, this._prop);
	endfunction

	function ToString(this)
		var str := $"Armor {EquipWrapper::ToString(this)}" +
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

	function _AddModifiers(this)
		if (this._prop.Modifiers.size() > 1)
			foreach modifier in (this._prop.Modifiers.keys())
				this.AddModifier(AttributeModifier(modifier, this._prop.Modifiers[modifier]));
			endforeach
		else
			if (this._tipoProtecao)
				var modifiersSlot := GetModifiersBySlot(this._coverage);
				var modifiersProt := GetModifiersByProtection(this._tipoProtecao);
				this.AddModifier(AttributeModifier(DEXTERITY, modifiersSlot[DEXTERITY].GetValue() + modifiersProt[DEXTERITY].GetValue()));
				this.AddModifier(AttributeModifier(SNEAK, modifiersSlot[SNEAK].GetValue() + modifiersProt[SNEAK].GetValue()));
				this.AddModifier(AttributeModifier(MECHANICS, modifiersSlot[MECHANICS].GetValue() + modifiersProt[MECHANICS].GetValue()));
				this.AddModifier(AttributeModifier(MAGERY, modifiersSlot[MAGERY].GetValue() + modifiersProt[MAGERY].GetValue()));
				this.AddModifier(AttributeModifier(AWARENESS, modifiersSlot[AWARENESS].GetValue() + modifiersProt[AWARENESS].GetValue()));
			endif
		endif
	endfunction
endclass