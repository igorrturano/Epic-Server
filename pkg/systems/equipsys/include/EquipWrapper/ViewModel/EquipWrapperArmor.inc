include "EquipWrapper";
include "IEquipWrapperArmor";
include ":equipsys:Controller/ArmorController";

include ":combat:Model/ArmorResist";
include ":combat:Model/CombatPenalty";

class EquipWrapperArmor(EquipWrapper, IEquipWrapperArmor)
	var _coverage := array{};
	var _tipoProtecao := "";

	var _resists := ArmorResist();
	var _resistsMod := ArmorResist();
	var _penalties := array{}; // Array of CombatPenalty

	function EquipWrapperArmor(this, byRef item)
		Super(item);

		this._coverage := GetConfigStringArray(this._template, "Coverage");

		if (item.isA(POLCLASS_ARMOR)) // Caso o itemRef exista
			this._tipoProtecao := item.getType();
			this._resists := ArmorResist(
				this._prop.SlashResist,
				this._prop.PiercingResist,
				this._prop.BluntResist,

				this._prop.FireResist,
				this._prop.ColdResist,
				this._prop.PoisonResist,
				this._prop.EnergyResist
			);

		else // Caso seja apenas um template
			this._tipoProtecao := this._template.tipoProtecao;
			var templateResists := getResistPhysicalByProtection(this._template.tipoProtecao);
			var slotResists := getResistPhysicalBySlot(this._coverage);
			this._resists := ArmorResist(
				templateResists.SlashResist + slotResists.SlashResist,
				templateResists.PiercingResist + slotResists.PiercingResist,
				templateResists.BluntResist + slotResists.BluntResist,

				this._template.FireResist,
				this._template.ColdResist,
				this._template.PoisonResist,
				this._template.EnergyResist
			);
		endif

		var penaltiesSlot := getPenaltiesBySlot(this._coverage);
		var penaltiesProt := getPenaltiesByProtection(this._tipoProtecao);
		this._penalties.append(CombatPenalty(DEXTERITY, penaltiesSlot.DexPen + penaltiesProt.DexPen));
		this._penalties.append(CombatPenalty(SNEAK, penaltiesSlot.SneakPen + penaltiesProt.SneakPen));
		this._penalties.append(CombatPenalty(MECHANICS, penaltiesSlot.MAPen + penaltiesProt.MAPen));
		this._penalties.append(CombatPenalty(MAGERY, penaltiesSlot.MageryPen + penaltiesProt.MageryPen));
		this._penalties.append(CombatPenalty(AWARENESS, penaltiesSlot.AwarenessPen + penaltiesProt.AwarenessPen));
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

	function GetPenalties(this)
		return this._penalties ?: error{"errortext" := "The item have no Penalties!"};
	endfunction

	function SetResists(this, resists)
		if (this._item == error) return this._item; endif
		this._resists := resists;
	endfunction

	function SetResistsMod(this, resists)
		if (this._item == error) return this._item; endif
		this._resistsMod := resists;
	endfunction

	function SetPenalties(this, penalties)
		if (this._item == error) return this._item; endif
		this._penalties := penalties;
	endfunction

	function Update(this)
		if (this._item == error) return this._item; endif
		EquipWrapper::Update(this);

		this._prop.tipoProtecao := this._tipoProtecao;
		
		// Resists
		this._prop.SlashResist := this._resists.GetSlashResist();
		this._prop.PiercingResist := this._resists.GetPiercingResist();
		this._prop.BluntResist := this._resists.GetBluntResist();
		this._prop.FireResist := this._resists.GetFireResist();
		this._prop.ColdResist := this._resists.GetColdResist();
		this._prop.EnergyResist := this._resists.GetEnergyResist();
		this._prop.PoisonResist := this._resists.GetPoisonResist();

		// Penalties
		foreach penalty in (this._penalties)
			print("foreach->"+penalty.ToString());
			this._prop[penalty.GetAttribute()] := penalty.GetValue();
		endforeach

		this._item.setAttrBase(this._prop);
	endfunction

	function ToString(this)
		var str := "";
		str += $"Armor {EquipWrapper::ToString(this)},
		Coverage: {this._coverage}, Protection Type: {this._tipoProtecao},
		SlashResist: {this._resists.GetSlashResist()} (+{this._resistsMod.GetSlashResist()}),
		PiercingResist: {this._resists.GetPiercingResist()} (+{this._resistsMod.GetPiercingResist()}),
		BluntResist: {this._resists.GetBluntResist()} (+{this._resistsMod.GetBluntResist()})";

		foreach penalty in (this._penalties)
			str += $",
			{penalty.GetAttribute()}: {penalty.GetValue()}";
		endforeach

		return str;

		// return $"Armor {EquipWrapper::ToString(this)},
		// Coverage: {this._coverage}, Protection Type: {this._tipoProtecao},
		// SlashResist: {this._physicalResists.SlashResist} ({this._physicalResistsTier.SlashResist}),
		// PiercingResist: {this._physicalResists.PiercingResist} ({this._physicalResistsTier.PiercingResist}),
		// BluntResist: {this._physicalResists.BluntResist} ({this._physicalResistsTier.BluntResist})
		
		// DexPen: {this._penalties.DexPen},
		// SneakPen: {this._penalties.SneakPen},
		// MAPen: {this._penalties.MAPen},
		// MageryPen: {this._penalties.MageryPen},
		// AwarenessPen: {this._penalties.AwarenessPen}";
	endfunction
endclass