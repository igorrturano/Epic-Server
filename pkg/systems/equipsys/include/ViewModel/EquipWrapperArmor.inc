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

		if (this._prop) // Caso tenha prop com valores já definidos
			this._resists := ArmorResist(
				this._prop.SlashResist,
				this._prop.PiercingResist,
				this._prop.BluntResist,

				this._prop.FireResist,
				this._prop.ColdResist,
				this._prop.PoisonResist,
				this._prop.EnergyResist
			);
			this._resistsMod := ArmorResist();
			this.AddModifier(AttributeModifier(DEXTERITY, this._prop.Penalties[DEXTERITY]));
			this.AddModifier(AttributeModifier(SNEAK, this._prop.Penalties[SNEAK]));
			this.AddModifier(AttributeModifier(MECHANICS, this._prop.Penalties[MECHANICS]));
			this.AddModifier(AttributeModifier(MAGERY, this._prop.Penalties[MAGERY]));
			this.AddModifier(AttributeModifier(AWARENESS, this._prop.Penalties[AWARENESS]));

		else // Caso não tenha prop ou seja um item não existente (template)
			var templateResists := getResistPhysicalByProtection(this._template.tipoProtecao);
			var slotResists := getResistPhysicalBySlot(this._coverage);
			this._resists := ArmorResist(
				templateResists.GetSlashResist() + slotResists.GetSlashResist(),
				templateResists.GetPiercingResist() + slotResists.GetPiercingResist(),
				templateResists.GetBluntResist() + slotResists.GetBluntResist(),

				this._template.FireResist,
				this._template.ColdResist,
				this._template.PoisonResist,
				this._template.EnergyResist
			);
			this._resistsMod := ArmorResist();

			var penaltiesSlot := getPenaltiesBySlot(this._coverage);
			var penaltiesProt := getPenaltiesByProtection(this._tipoProtecao);
			this.AddModifier(AttributeModifier(DEXTERITY, penaltiesSlot[DEXTERITY].GetValue() + penaltiesProt[DEXTERITY].GetValue()));
			this.AddModifier(AttributeModifier(SNEAK, penaltiesSlot[SNEAK].GetValue() + penaltiesProt[SNEAK].GetValue()));
			this.AddModifier(AttributeModifier(MECHANICS, penaltiesSlot[MECHANICS].GetValue() + penaltiesProt[MECHANICS].GetValue()));
			this.AddModifier(AttributeModifier(MAGERY, penaltiesSlot[MAGERY].GetValue() + penaltiesProt[MAGERY].GetValue()));
			this.AddModifier(AttributeModifier(AWARENESS, penaltiesSlot[AWARENESS].GetValue() + penaltiesProt[AWARENESS].GetValue()));
		endif
		
		this._weight := this._GetWeight();
		this._strRequired := this._GetSTRRequired();
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

		// Resists
		this._prop.+SlashResist := this._resists.GetSlashResist();
		this._prop.+PiercingResist := this._resists.GetPiercingResist();
		this._prop.+BluntResist := this._resists.GetBluntResist();
		this._prop.+FireResist := this._resists.GetFireResist();
		this._prop.+ColdResist := this._resists.GetColdResist();
		this._prop.+EnergyResist := this._resists.GetEnergyResist();
		this._prop.+PoisonResist := this._resists.GetPoisonResist();

		// Penalties
		this._prop.+Penalties := dictionary{};
		foreach penalty in (this._penalties)
			this._prop.Penalties[penalty.GetAttribute()] := penalty.GetValue();
		endforeach

		this._item.SetProp(PROPNAME, this._prop);
	endfunction

	function ToString(this)
		var str := "";
		str += $"Armor {EquipWrapper::ToString(this)},
		Coverage: {this._coverage}, Protection Type: {this._tipoProtecao},
		SlashResist: {this._resists.GetSlashResist()} (+{this._resistsMod.GetSlashResist()}),
		PiercingResist: {this._resists.GetPiercingResist()} (+{this._resistsMod.GetPiercingResist()}),
		BluntResist: {this._resists.GetBluntResist()} (+{this._resistsMod.GetBluntResist()});

		BluntResist: {this._resists} (+{this._resistsMod})";

		foreach penalty in (this._penalties)
			str += $",
			{penalty.GetAttribute()}: {penalty.GetValue()}";
		endforeach

		return str;
	endfunction

	// Private functions
	function _GetWeight(this)
		var weight := 0;
		var prot := SplitWords(this._tipoProtecao);
		
		if (Upper(prot[1]) == "ESCUDO") // Se for escudo, peso é o DEX Penalty
			weight := this._penalties[DEXTERITY].GetValue() ?: 0;
		else // Se não for, é a soma do maior valor de resistência + resistência por slot
			weight := Max(array{this._resists.GetSlashResist(), this._resists.GetPiercingResist(), this._resists.GetBluntResist()});
			weight += CInt(getResistPhysicalBySlot(this._coverage));
		endif

		return weight;
	endfunction
endclass