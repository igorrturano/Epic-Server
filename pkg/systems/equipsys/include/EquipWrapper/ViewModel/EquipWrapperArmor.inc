include "EquipWrapper";
include "IEquipWrapperArmor";
include ":equipsys:Controller/ArmorController";

include ":combat:Model/ArmorResist";

class EquipWrapperArmor(EquipWrapper, IEquipWrapperArmor)
	var _coverage := array{};
	var _tipoProtecao := "";

	var _resistsTier := ArmorResist();
	var _resists := ArmorResist();
	var _resistsModTier := ArmorResist();
	var _resistsMod := ArmorResist();
	var _penalties := struct{};

	function EquipWrapperArmor(this, byRef item)
		Super(item);

		if (item.isA(POLCLASS_ARMOR)) // Caso o itemRef exista
			this._coverage := GetConfigStringArray(this._template, "Coverage");
			this._tipoProtecao := item.getType();
			this._resistsTier := ArmorResist(
				SplitWords(this._template.EpicPhysicalResists)[1],
				SplitWords(this._template.EpicPhysicalResists)[2],
				SplitWords(this._template.EpicPhysicalResists)[3],
				SplitWords(this._template.EpicElementalResists)[1],
				SplitWords(this._template.EpicElementalResists)[2],
				SplitWords(this._template.EpicElementalResists)[3],
				SplitWords(this._template.EpicElementalResists)[4]
			);
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
			this._coverage := GetConfigStringArray(this._template, "Coverage");
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
			

			var penaltiesSlot := getPenaltiesBySlot(this._coverage);
			var penaltiesProt := getPenaltiesByProtection(this._tipoProtecao);
			this._penalties := struct {
				DexPen := penaltiesSlot.DexPen + penaltiesProt.DexPen,
				SneakPen := penaltiesSlot.SneakPen + penaltiesProt.SneakPen,
				MAPen := penaltiesSlot.MAPen + penaltiesProt.MAPen,
				MageryPen := penaltiesSlot.MageryPen + penaltiesProt.MageryPen,
				AwarenessPen := penaltiesSlot.AwarenessPen + penaltiesProt.AwarenessPen
			};
		endif
	endfunction
	
	function GetTipoProtecao(this)
		return this._tipoProtecao ?: error{"errortext" := "The item have no Protection Type!"};
	endfunction

	function GetCoverage(this)
		return this._coverage ?: error{"errortext" := "The item have no Coverage!"};
	endfunction

	function GetPhysicalResists(this)
		return this._physicalResists ?: error{"errortext" := "The item have no Physical Resists!"};
	endfunction

	function GetElementalResists(this)
		return this._elementalResists ?: error{"errortext" := "The item have no Elemental Resists!"};
	endfunction

	function GetPenalties(this)
		return this._penalties ?: error{"errortext" := "The item have no Penalties!"};
	endfunction

	function SetPhysicalResists(this, resists)
		if (this._item == error) return this._item; endif
		this._physicalResists := resists;
	endfunction

	function SetElementalResists(this, resists)
		if (this._item == error) return this._item; endif
		this._elementalResists := resists;
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
		this._prop.SlashResist := this._physicalResists.SlashResist;
		this._prop.PiercingResist := this._physicalResists.PiercingResist;
		this._prop.BluntResist := this._physicalResists.BluntResist;
		this._prop.FireResist := this._elementalResists.FireResist;
		this._prop.ColdResist := this._elementalResists.ColdResist;
		this._prop.EnergyResist := this._elementalResists.EnergyResist;
		this._prop.PoisonResist := this._elementalResists.PoisonResist;

		// Penalties
		this._prop.DexPen := this._penalties.DexPen;
		this._prop.SneakPen := this._penalties.SneakPen;
		this._prop.MAPen := this._penalties.MAPen;
		this._prop.MageryPen := this._penalties.MageryPen;
		this._prop.AwarenessPen := this._penalties.AwarenessPen;

		this._item.setAttrBase(this._prop);
	endfunction

	function ToString(this)
		return $"Armor {EquipWrapper::ToString(this)},
		Coverage: {this._coverage}, Protection Type: {this._tipoProtecao},
		SlashResist: {this._physicalResists.SlashResist} ({this._physicalResistsTier.SlashResist}),
		PiercingResist: {this._physicalResists.PiercingResist} ({this._physicalResistsTier.PiercingResist}),
		BluntResist: {this._physicalResists.BluntResist} ({this._physicalResistsTier.BluntResist})

		DexPen: {this._penalties.DexPen},
		SneakPen: {this._penalties.SneakPen},
		MAPen: {this._penalties.MAPen},
		MageryPen: {this._penalties.MageryPen},
		AwarenessPen: {this._penalties.AwarenessPen}";
	endfunction
endclass