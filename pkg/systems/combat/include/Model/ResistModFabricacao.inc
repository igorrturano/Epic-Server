include "ResistModBase";
include ":crafting:/include/caracteristicasFabricacao";
include "ArmorResist";

class ResistModFabricacao(ResistModBase)

	function ResistModFabricacao(this ,itemOrTemplate ,damage := 0)
		this._itemOrTemplate := itemOrTemplate;

		Super(MOD_SOURCES::CRAFTING, damage);
	endfunction

	function MapResist(this, param)
		var resist;

		if(param is @ArmorResist)
			resist := param;
		else
			resist := ArmorResist();
		endif
		
		resist.SetSlashRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CORTANTE)));
		resist.SetPiercingRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_PERFURANTE)));
		resist.SetBluntRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CONTUSAO)));
		
		//OBS: se no futuro tiver um componente só pra resistência mágica tem que adicionar aqui a soma.
		resist.SetFireRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_FOGO)));
		resist.SetColdRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_GELO)));
		resist.SetPoisonRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_VENENO)));
		resist.SetEnergyRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_ENERGIA)));
		
		return resist;
	endfunction

	function _GetCraftingFullBonus(this)
		return  PegaValorFabricacao(this._itemOrTemplate,CARACTERISTICA_FABRICACAO_DEFESA);
	endfunction
endclass
