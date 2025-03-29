//Author: Deiro || Rafael Santos
use uo;
use os;
use Math;
use cfgfile;

include "/include/math";
include ":attributes:attributes";
include "/include/epicPropNames";
include "/include/epicConstants";

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

function PegaDificuldade( quem, descritorFabricacao, nomePericia, produto := 0, estacao  := 0)
	 var retorno := struct;
	 
	 retorno.+skill := CDbl(GetConfigString(descritorFabricacao,"skill"));
	 retorno.+chancePelaSkill := 0;
	 retorno.+modificadoresChance := array;
	 retorno.+chanceModificadaTotal := 0;
	 retorno.+chanceBase := PegaChanceFabricacaoBase(quem,
												nomePericia,
												retorno.skill,
												retorno.chancePelaSkill);
	
	var propFabricado := GetObjProperty(produto, PROPNAME_FABRICADO );
	
	if(propFabricado)
		foreach componente in (propFabricado.esquema.componentesEscolhidos)
			Sleepms(2);
			
			var chanceCalculada := cint(componente.poderAvaliado.poderContado) * -1;
			
			var modificador := struct{
				origem := BLUE + "Dificuldade {} :{} {}%<br>".format(componente.item.nome,chanceCalculada < 0? RED_FIRE: SOFT_GREEN, "{}"),
				valor :=  chanceCalculada
			};

			retorno.modificadoresChance.append(modificador);
			retorno.chanceModificadaTotal += chanceCalculada;
			retorno.chancePelaSkill += chanceCalculada;
		endforeach
	endif

	retorno.+chanceTotal := retorno.chanceBase + retorno.chanceModificadaTotal;
	retorno.chanceTotal := retorno.chanceTotal < 0 ? 0: retorno.chanceTotal;
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

	var descriptor := GetItemDescriptor(CInt(propFabricadoMontado.receita.objtype));
		descriptor.CProps.insert(PROPNAME_MATERIAL,propFabricadoMontado.esquema.material);
		descriptor.CProps.insert(PROPNAME_FABRICADO, CriaNovaPropFabricado(propFabricadoMontado));
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
		descriptor.maxhp := propFabricadoMontado.esquema.modificadorHpTotal? descriptor.maxhp + propFabricadoMontado.esquema.modificadorHpTotal : descriptor.maxhp;
		if(!propFabricadoMontado.receita.isEmpilhavel)
			descriptor.CProps.insert(PROPNAME_ARTIFICE_SERIAL,quem.serial);
		endif
		descriptor.color := propFabricadoMontado.esquema.cor;
		
	var item := CreateItemInBackpack(quem,descriptor, cint(quantidade));
	if(!item)
		item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, cint(quantidade), quem.realm);
	endif

	if(!item)
		SendSysMessageEX(quem, "Erro ao criar objeto.",SSM_FAIL);
	endif

	item.weight_multiplier_mod := propFabricadoMontado.esquema.modificadorPesoTotal ? ((propFabricadoMontado.esquema.modificadorPesoTotal / 100) + 1) : 0;

endfunction

function CriaNovaPropFabricado ( propFabricadoMontado)

	var componentesRefatorado := dictionary;

	//Refatora o dict para usar como indice a caracteristica, assim dá para ir direto ao assunto se quiser pegar o componente que tá dando aquela caracteristica.
	foreach componente in (propFabricadoMontado.esquema.componentesEscolhidos)
		Sleepms(2);
		componentesRefatorado.insert(componente.componenteNecessario.caracteristica,componente);
	endforeach
	
	var novaPropFabricada := struct{
				estado := ITEM_PRONTO ,
				isCompletamenteComum := propFabricadoMontado.esquema.isCompletamenteComum,
				qualidade := propFabricadoMontado.esquema.qualidade,
				modificadorHpTotal := propFabricadoMontado.esquema.modificadorHpTotal,
				modificadorPesoTotal := propFabricadoMontado.esquema.modificadorPesoTotal,
				componentes := componentesRefatorado};
				
	return novaPropFabricada;
endfunction

function CriaObjetoPosicao(quem)
	return struct{x := quem.x, y := quem.y, z := quem.z, realm := quem.realm};
endfunction

function isEstacaoValida(quem, estacaoAlvo, pericia)
	if(estacaoAlvo.item)
		var ocupanteSerial := GetObjProperty(estacaoAlvo.item,PROPNAME_ISESTACAO_OCUPADA);
		
		if(ocupanteSerial)
			var ocupante := SystemFindObjectBySerial(ocupanteSerial);
			if(ocupante && Distance(ocupante,estacaoAlvo) <= 2)
				SendSysMessageEX(quem,"A estação está em uso.", SSM_FAIL);
			endif
		endif
	else
		SendSysMessageEX(quem,"Estação inválida.", SSM_FAIL);
		return 0;
	endif
	SetObjProperty(estacaoAlvo.item,PROPNAME_ISESTACAO_OCUPADA, quem.serial);
	return 1;
endfunction

function PegaDescDuracaoFabricacao(qtdSucessos)
	if(qtdSucessos > 30)
		return "Extremamente Longo";
	elseif(qtdSucessos <= 30 && qtdSucessos > 25)
		return "Muito Longo";
	elseif(qtdSucessos <= 20 && qtdSucessos > 15)
		return "Longo";
	elseif(qtdSucessos <= 15 && qtdSucessos > 10)
		return "Razoável";
	elseif(qtdSucessos <= 10 && qtdSucessos > 5)
		return "Rápido";
	else
		return "Muito Rápido";
	endif
endfunction

function MontaCraftProp(fabricacaoConfig)

	var baseDict  := dictionary;
	foreach pericia in GetConfigStringArray(fabricacaoConfig["Skills"], "Skill")
		var temp := dictionary;
		var visualizadas := struct;
		visualizadas.+receitas := array;
		visualizadas.+categorias := array;

		temp.+skill  := fabricacaoConfig[pericia].Attribute;
		temp.+visualizadas := visualizadas;
		temp.+last   := 0;
		baseDict[temp.skill] := temp;
		Sleepms(2);
	endforeach
	
	return baseDict;

endfunction

// TODO [futuras versões] pensar em um modo de aplicar uma dificuldade a mais dependendo do componente.
function DistribuiQualidadeComponentes(quantidade,valorPico, deslocamentoPico := -1) 
	
	if(quantidade == error || quantidade.size() == 0)
		return error{"errortext":="O parâmetro quantidade é obrigatório."};
	endif

	var	chancesPorTier := DistribuicaoGaussianaNormalizada({1,2,3,4,5},valorPico + deslocamentoPico,1.4);
	var retorno := dictionary;

	//Print("Chances: "  + chancesPorTier);
	for i:=1 to quantidade
		var isFalhouTudo := true;
		var rolagem := RandomFloat(100.0);
		var chanceAcumulada := 0;

		foreach chance in chancesPorTier
			chanceAcumulada += chance;
			//Print("Chance Tier " + _chance_iter + ": " + chance);
			//Print("rolagem: " + rolagem + " chance: " + chanceAcumulada);
			if (rolagem <= chanceAcumulada)
				
				isFalhouTudo := false;
				if(retorno.exists(_chance_iter)) // popula o dict de retono com chave=tier e valor=quantidade de itens nesse tier.
					retorno[_chance_iter] += 1;
				else
					retorno.insert(_chance_iter,1);
				endif
				break;
			endif
			Sleepms(2);
		endforeach

		if(isFalhouTudo) //Se falhou tudo, coloca na pior qualidade.
			if(retorno.exists(1))
				retorno[1] += 1;
			else
				retorno.insert(1,1);
			endif
		endif
		Sleepms(2);
	endfor
	return retorno;

endfunction
