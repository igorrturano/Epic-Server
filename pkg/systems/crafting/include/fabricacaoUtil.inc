//Author: Deiro || Rafael Santos
use uo;
use os;
use Math;
use cfgfile;

include "/include/math";
include ":attributes:attributes";
include "/include/epicPropNames";

const CHANCE_FABRICACAO_MAX := 80.0;

enum ESTADOS_FABRICACAO
	ITEM_PRONTO,
	ITEM_MONTADO,
	ITEM_GERADO
endenum

function PegaComponenteTierDesc(tier)
	case (tier)
		1 : return struct {desc := "Mediocre" , valor := -1}; break;
		2 : return struct {desc := "Comum" , valor := 0}; break;
		3 : return struct {desc := "Notável" , valor := 1}; break;
		4 : return struct {desc := "Superior" , valor := 2}; break;
		5 : return struct {desc := "Excepcional" , valor := 3}; break;
		default:
			return struct {desc := "Erro!" , valor := 0}; break;
	endcase
endfunction

function PegaDificuldade( quem, descritorFabricacao, nomePericia)
	 var retorno := struct;
	 
	 retorno.+skill := CDbl(GetConfigString(descritorFabricacao,"skill"));
	 retorno.+chancePelaSkill := 0;
	 retorno.+chanceBase := PegaChanceFabricacaoBase(quem,
												nomePericia,
												retorno.skill,
												retorno.chancePelaSkill);

		return retorno;
endfunction

function PegaChanceFabricacaoBase(quem, pericia, dificuldadeItem, byref chancePelaSkill)
	var valorPericia := AP_GetTrueSkill(quem, pericia);
	valorPericia := ApplySkillModifiers(quem, pericia, valorPericia); 
	chancePelaSkill := CalcSuccessPercent(quem, pericia,CDbl(dificuldadeItem), CDbl(valorPericia));
	return  Trunc(CHANCE_FABRICACAO_MAX * (chancePelaSkill/100.0), 1);
endfunction

function CriaItemFabricado(quem, produto, quantidade := 1)

	var propFabricadoMontado := GetObjProperty(produto, PROPNAME_FABRICADO);

	var novaPropFabricada := struct{
				estado := ITEM_PRONTO ,
				isCompletamenteComum := propFabricadoMontado.esquema.isCompletamenteComum,
				componentes := propFabricadoMontado.esquema.componentesEscolhidos};

	var descriptor := GetItemDescriptor(CInt(propFabricadoMontado.receita.objtype));
		descriptor.CProps.insert(PROPNAME_MATERIAL,propFabricadoMontado.esquema.material);
		descriptor.CProps.insert(PROPNAME_FABRICADO, novaPropFabricada);
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
		if(!propFabricadoMontado.receita.isEmpilhavel)
			descriptor.CProps.insert(PROPNAME_ARTIFICE_SERIAL,quem.serial);
		endif
		descriptor.color := propFabricadoMontado.esquema.cor;
		
	var item := CreateItemInBackpack(quem,descriptor, quantidade);

endfunction