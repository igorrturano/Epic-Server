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

		// Escala as contribuicoes de crafting (material + qualidade do crafter) do
		// scale interno 0-4 para % de reducao. PegaComponenteTierDesc.valor e
		// PegaValorFabricacao continuam retornando 0-4 (consumidos por outras
		// caracteristicas: dano, critico, durabilidade, etc.); o ajuste vive aqui
		// porque ResistModFabricacao e o ponto onde os valores viram resist %.
		// Refactor proprio (split crafter vs material por design_combate.md) fica
		// no parking lot ate refatorarmos controladoraComplexos.inc dict-keys.
		var ARMOR_PCT_MULT := 5;

		var bundle := PegaFabricacaoBundle(this._itemOrTemplate, CARACTERISTICA_FABRICACAO_DEFESA);
		var defesaBase := bundle.principal;
		var sub := bundle.subs;

		resist.SetSlashResist((defesaBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_CORTANTE])) * ARMOR_PCT_MULT);
		resist.SetPiercingResist((defesaBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_PERFURANTE])) * ARMOR_PCT_MULT);
		resist.SetBluntResist((defesaBase + Cint(sub[SUBCARACTERISTICA_FABRICACAO_CONTUSAO])) * ARMOR_PCT_MULT);

		//OBS: se no futuro tiver um componente só pra resistência mágica tem que adicionar aqui a soma.
		resist.SetFireResist(Cint(sub[SUBCARACTERISTICA_FABRICACAO_FOGO]) * ARMOR_PCT_MULT);
		resist.SetColdResist(Cint(sub[SUBCARACTERISTICA_FABRICACAO_GELO]) * ARMOR_PCT_MULT);
		resist.SetPoisonResist(Cint(sub[SUBCARACTERISTICA_FABRICACAO_VENENO]) * ARMOR_PCT_MULT);
		resist.SetEnergyResist(Cint(sub[SUBCARACTERISTICA_FABRICACAO_ENERGIA]) * ARMOR_PCT_MULT);

		return resist;
	endfunction
endclass
