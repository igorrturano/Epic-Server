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
		
		var bundle := PegaFabricacaoBundle(this._itemOrTemplate, CARACTERISTICA_FABRICACAO_DANO);
		var danoBase := bundle.principal;
		var sub := bundle.subs;

		damage.SetSlashRawDamage(danoBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_CORTANTE]));
		damage.SetPiercingRawDamage(danoBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_PERFURANTE]));
		damage.SetBluntRawDamage(danoBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_CONTUSAO]));

		// Elemental da madeira e INERENTE ao componente (igual ao físico) —
		// nao precisa de ativacao. Metal nao tem elemental no dano (poe no
		// defesa) por isso depende de Encantar Objeto; isso e diferenca de
		// material, nao um gate. Encantar soma por cima via DamageModEncantar.
		//OBS: se no futuro tiver um componente só pra dano mágico tem que adicionar aqui a soma.
		damage.SetFireRawDamage(Cint(sub[SUBCARACTERISTICA_FABRICACAO_FOGO]));
		damage.SetColdRawDamage(Cint(sub[SUBCARACTERISTICA_FABRICACAO_GELO]));
		damage.SetPoisonRawDamage(Cint(sub[SUBCARACTERISTICA_FABRICACAO_VENENO]));
		damage.SetEnergyRawDamage(Cint(sub[SUBCARACTERISTICA_FABRICACAO_ENERGIA]));

		return damage;
	endfunction
endclass
