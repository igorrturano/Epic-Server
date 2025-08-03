include "EquipWrapper";

class EquipArmorWrapper(EquipWrapper)
	var _coverage;
	var _tipoProtecao;

	var _physicalResists;
	
	function EquipArmorWrapper(this, item)
		var template, coverage, tipoProtecao;
		Super(item);

		if (item.isA(POLCLASS_ARMOR))
			template := item.getCfgElem();
			this._coverage := template.coverage;
			this._tipoProtecao := item.getType();
			this._physicalResists := struct {
				SlashResist := item.getAttrBase().SlashResist,
				PiercingResist := item.getAttrBase().PiercingResist,
				BluntResist := item.getAttrBase().BluntResist
			};

		else
			template := FindConfigElem(ReadConfigFile(":*:itemdesc"), item);
			this._coverage := template.coverage;
			this._tipoProtecao := template.tipoProtecao;
			var templateResists := getResistPhysicalByProtectionType(template.tipoProtecao);
			this._physicalResists := struct {
				SlashResist := templateResists[1],
				PiercingResist := templateResists[2],
				BluntResist := templateResists[3]
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

	function ToString(this)
		return $"Armor {EquipWrapper::ToString(this)},
		Coverage: {this.GetCoverage()}, Protection Type: {this.GetTipoProtecao()}
		SlashResist: {this._physicalResists.SlashResist},
		PiercingResist: {this._physicalResists.PiercingResist},
		BluntResist: {this._physicalResists.BluntResist}";
	endfunction
endclass