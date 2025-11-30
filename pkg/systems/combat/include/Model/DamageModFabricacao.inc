include "DamageModBase";
include ":crafting:/include/caracteristicasFabricacao";
include "Damage";

class DamageModFabricacao(DamageModBase)

	function DamageModFabricacao(this ,itemOrTemplate ,damage := 0)
		this._itemOrTemplate := itemOrTemplate;

		Super(MOD_SOURCES::CRAFTING, damage);
	endfunction

	function MapDamage(this, param)
		var damage;

		if(param is @Damage)
			damage := param;
		else
			damage := Damage();
		endif
		
		damage.SetSlashRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CORTANTE)));
		damage.SetPiercingRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_PERFURANTE)));
		damage.SetBluntRawDamage(this._GetCraftingFullBonus() + cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_CONTUSAO)));
		
		//OBS: se no futuro tiver um componente só pra dano mágico tem que adicionar aqui a soma.
		damage.SetFireRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_FOGO)));
		damage.SetColdRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_GELO)));
		damage.SetPoisonRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_VENENO)));
		damage.SetEnergyRawDamage(cint(PegaSubValoresFabricacao(this._itemOrTemplate,SUBCARACTERISTICA_FABRICACAO_ENERGIA)));
		
		return damage;
	endfunction

	function _GetCraftingFullBonus(this)
		return  PegaValorFabricacao(this._itemOrTemplate,CARACTERISTICA_FABRICACAO_DANO);
	endfunction
endclass
