include "ResistModBase";
include ":crafting:/include/caracteristicasFabricacao";
include "ArmorResist";

class ResistModFabricacao(ResistModBase)

	function ResistModFabricacao(this ,itemOrTemplate ,resist := 0)
		this._itemOrTemplate := itemOrTemplate;

		Super(MOD_SOURCES::CRAFTING, resist);
	endfunction

	function MapResist(this, param)
		var resist;

		if(param is @ArmorResist)
			resist := param;
		else
			resist := ArmorResist();
		endif

		
		resist.SetSlashResist(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CORTANTE)));
		resist.SetPiercingResist(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_PERFURANTE)));
		resist.SetBluntResist(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CONTUSAO)));
		
		//OBS: se no futuro tiver um componente só pra resistência mágica tem que adicionar aqui a soma.
		resist.SetFireResist(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_FOGO)));
		resist.SetColdResist(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_GELO)));
		resist.SetPoisonResist(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_VENENO)));
		resist.SetEnergyResist(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_ENERGIA)));
		
		return resist;
	endfunction

	function _GetCraftingFullBonus(this)
		return  PegaValorFabricacao(this._itemOrTemplate,CARACTERISTICA_FABRICACAO_DEFESA);
	endfunction
endclass
