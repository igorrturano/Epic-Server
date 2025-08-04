include "EquipWrapper";

class EquipWrapperArmor(EquipWrapper)
	var _coverage; // String
	var _tipoProtecao; // String

	var _physicalResists; // Struct
	var _penalties; // Struct

	function EquipWrapperArmor(this, item)
		var template, itemAttr, coverage, tipoProtecao;
		Super(item);

		if (item.isA(POLCLASS_ARMOR))
			template := item.getCfgElem();
			itemAttr := item.getAttrBase();

			this._coverage := template.coverage;
			this._tipoProtecao := item.getType();
			this._physicalResists := struct {
				SlashResist := itemAttr.SlashResist,
				PiercingResist := itemAttr.PiercingResist,
				BluntResist := itemAttr.BluntResist
			};

		else
			template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);
			this._coverage := template.coverage;
			this._tipoProtecao := template.tipoProtecao;
			var templateResists := getResistPhysicalByProtectionType(template.tipoProtecao);
			this._physicalResists := struct {
				SlashResist := templateResists[1] + getResistPhysicalBySlot(this.coverage)[1],
				PiercingResist := templateResists[2] + getResistPhysicalBySlot(this.coverage)[2],
				BluntResist := templateResists[3] + getResistPhysicalBySlot(this.coverage)[3]
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
		return this._tipoProtecao;
	endfunction

	function GetCoverage(this)
		return this._coverage;
	endfunction

	function getPhysicalResists(this)
		return this._physicalResists;
	endfunction

	function getPenalties(this)
		return this._penalties;
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