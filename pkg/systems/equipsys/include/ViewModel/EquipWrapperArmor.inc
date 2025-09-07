include "EquipWrapper";
include ":equipsys:Controller/ArmorController";

class EquipWrapperArmor(EquipWrapper)
	var _coverage := array{};
	var _tipoProtecao := "";

	var _physicalResists := struct{};
	var _elementalResists := struct{};
	var _penalties := struct{};

	function EquipWrapperArmor(this, item)
		var template, itemPropBase, coverage, tipoProtecao;
		Super(item);

		if (item.isA(POLCLASS_ARMOR)) // Caso o itemRef exista
			template := item.getCfgElem();
			itemPropBase := item.getAttrBase();

			this._coverage := GetConfigStringArray(template, "Coverage");
			this._tipoProtecao := item.getType();
			this._physicalResists := struct {
				SlashResist := itemPropBase.SlashResist,
				PiercingResist := itemPropBase.PiercingResist,
				BluntResist := itemPropBase.BluntResist
			};
			this._elementalResists := struct {
				FireResist := itemPropBase.FireResist,
				ColdResist := itemPropBase.ColdResist,
				EnergyResist := itemPropBase.EnergyResist,
				PoisonResist := itemPropBase.PoisonResist
			};

		else // Caso seja apenas um template
			template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);
			this._coverage := GetConfigStringArray(template, "Coverage");
			this._tipoProtecao := template.tipoProtecao;
			var templateResists := getResistPhysicalByProtection(template.tipoProtecao);
			this._physicalResists := struct {
				SlashResist := templateResists[1] + getResistPhysicalBySlot(this.coverage)[1],
				PiercingResist := templateResists[2] + getResistPhysicalBySlot(this.coverage)[2],
				BluntResist := templateResists[3] + getResistPhysicalBySlot(this.coverage)[3]
			};
			this._elementalResists := struct {
				FireResist := template.FireResist,
				ColdResist := template.ColdResist,
				EnergyResist := template.EnergyResist,
				PoisonResist := template.PoisonResist
			};

			var penaltiesSlot := getPenaltiesBySlot(this.coverage);
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

	function ToString(this)
		return $"Armor {EquipWrapper::ToString(this)},
		Coverage: {this.GetCoverage()}, Protection Type: {this.GetTipoProtecao()}
		SlashResist: {this._physicalResists.SlashResist},
		PiercingResist: {this._physicalResists.PiercingResist},
		BluntResist: {this._physicalResists.BluntResist}
		
		DexPen: {this._penalties.DexPen},
		SneakPen: {this._penalties.SneakPen},
		MAPen: {this._penalties.MAPen},
		MageryPen: {this._penalties.MageryPen},
		AwarenessPen: {this._penalties.AwarenessPen}";
	endfunction
endclass