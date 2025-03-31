//Author: Deiro || Rafael Santos
use uo;
use os;
use Math;
use cfgfile;

include "/include/math";
include ":attributes:attributes";
include "/include/epicPropNames";
include "/include/epicConstants";
include ":keys:key";

const CHANCE_FABRICACAO_MAX := 80.0;
var itemsConfig := ReadConfigFile(":*:itemdesc");

enum ESTADOS_FABRICACAO
	ITEM_PRONTO,
	ITEM_MONTADO,
	ITEM_GERADO
endenum

enum COMPONENTES_ENCAIXE_ORDEM
	DESC_ENCAIXE_CLASSE := 1,
	DESC_ENCAIXE_CARACTERISTICA := 2,
	DESC_ENCAIXE_ENCAIXE := 3,
	DESC_ENCAIXE_QTD_NECESSARIA := 4,
	DESC_ENCAIXE_CONDICIONAL := 5,
endenum

function ClassificaNomeReceita(nomeReceita)
	var dividido := SplitWords(nomeReceita,"-");
	return dictionary {
		"nome" -> dividido[1],
		"classe" -> dividido[2]
	};
endfunction

function PegaNomeReceitaClassificada(receitaClassificada)
	var nomeClassificado;

	if(receitaClassificada.classe)
		nomeClassificado := receitaClassificada.nome + "-" + receitaClassificada.classe;
	else
		nomeClassificado := receitaClassificada.nome;
	endif

	return nomeClassificado;
endfunction

function PegaSkillNecessaria(objtype, profissao, classeReceita := 1)
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var InfoFabricacao := ReadConfigFile(profissaoConfig[profissao].InfoFabricacao);
	var skills := SplitWords(InfoFabricacao[objtype].skill);
	var skill;

	if(!classeReceita) // se mandarem uma undefined ou com erro, 
		skill := skills[1];
	else
		skill := skills[cint(classeReceita)];
	endif

	return skill ? CDbl(skill) : 0;
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

function CalculaValorPico(quem, pericia)
	return AP_GetSkill(quem,pericia) / 20;
endfunction

function PegaDificuldadeFrabicarObjeto(quem, objtype, nomeProfissao, nomePericia, classe , produto := 0, estacao  := 0)
	var skill :=  PegaSkillNecessaria(objtype, nomeProfissao, classe);
	return PegaDificuldade(quem, skill, nomePericia, produto, estacao);
endfunction 

function PegaDificuldade( quem, skillNecessaria, nomePericia, produto := 0, estacao  := 0)
	 var retorno := struct;
	 
	 retorno.+skill := CDbl(skillNecessaria);
	 retorno.+chancePelaSkill := 0;
	 retorno.+modificadoresChance := array;
	 retorno.+chanceModificadaTotal := 0;
	 retorno.+chanceBase := PegaChanceFabricacaoBase(quem,
												nomePericia,
												retorno.skill,
												retorno.chancePelaSkill);
	
	print("retorno " + retorno);
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

function CriaComponenteDescriptor(objtype, chaveMaterial, qualidade, encaixe, configRecursos)
		var descriptor := GetItemDescriptor(CInt(objtype));
		descriptor.CProps.insert(PROPNAME_MATERIAL,chaveMaterial);
		descriptor.CProps.insert(PROPNAME_TIER_COMPONENTE,qualidade);
		descriptor.CProps.insert(PROPNAME_ENCAIXE_COMPONENTE,encaixe);
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
		descriptor.color := configRecursos[chaveMaterial].Color;

		return descriptor;
endfunction

function CriaItemFabricado(quem, produto, quantidade := 1)

	var propFabricadoMontado := GetObjProperty(produto, PROPNAME_FABRICADO);
	var descriptor;

	if(propFabricadoMontado.receita.complexoComponente) // Se for item complexo que é considerado componente. [ex: fechadura]
		var qualidades := DistribuiQualidadeComponentes(1,CalculaValorPico(quem,propFabricadoMontado.receita.pericia));
		
		descriptor := CriaComponenteDescriptor(
										propFabricadoMontado.receita.objtype,
										propFabricadoMontado.esquema.material,
										qualidades.keys()[1],
										propFabricadoMontado.receita.complexoComponente.encaixe,
										ReadConfigFile(RECURSOS_CONFIG_PATH)
										);
	else
		descriptor := GetItemDescriptor(CInt(propFabricadoMontado.receita.objtype));
		descriptor.CProps.insert(PROPNAME_MATERIAL,propFabricadoMontado.esquema.material);
		descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
	endif

	descriptor.CProps.insert(PROPNAME_FABRICADO, CriaNovaPropFabricado(propFabricadoMontado));
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

	AplicaFechadura(quem, item);

	item.weight_multiplier_mod := propFabricadoMontado.esquema.modificadorPesoTotal ? ((propFabricadoMontado.esquema.modificadorPesoTotal / 100) + 1) : 0;

endfunction

function AplicaFechadura(quem, item)
	var levelFechadura := PegaCaracteristica(item, "tranca");
	
	if(levelFechadura && KP_IsLockable(item))
		    var lockid := AllocLockId();
			var key := CreateItemInBackpack( quem, "ironkey", 1 );
			if(key)
				SetObjProperty( item, "LockId", lockid );
				SetObjProperty(item,"level", levelFechadura > 5?  5 : levelFechadura);
				SetObjProperty( key, "LockId", lockid );
				SendSysMessageEx( quem, "A chave criada esta na sua mochila.", SSM_INFO );
			else
				SendSysMessageEx( quem, "Não foi possível criar a chave do item com fechadura.", SSM_FAIL );
			endif
	endif
endfunction

function PegaCaracteristica(item, caracteristica)
	var propFabricadoMontado := GetObjProperty(item, PROPNAME_FABRICADO);
	Print(propFabricadoMontado);
	return propFabricadoMontado.componentes[caracteristica].propriedades.valorPrincipal; 
endfunction

function CriaNovaPropFabricado ( propFabricadoMontado)

	var componentesRefatorado := dictionary;

	//Refatora o dict para usar como indice a caracteristica, assim dá para ir direto ao assunto se quiser pegar o componente que tá dando aquela caracteristica.
	foreach componente in (propFabricadoMontado.esquema.componentesEscolhidos)
		Sleepms(2);
		if(componente.componenteNecessario.caracteristica != "vazio")
			componentesRefatorado.insert(componente.componenteNecessario.caracteristica,componente);
		else
			componentesRefatorado.insert(_componente_iter,componente);
		endif
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

function isEstacaoValida(quem, estacaoAlvo, profissao)
	var profissaoConfig := ReadConfigFile(PROFISSAO_CONFIG_PATH);
	var estacoesValidas := GetConfigStringArray(profissaoConfig[profissao], "EstacoesValidas");

	if(cstr(estacaoAlvo.item.objtype) in estacoesValidas ||  "superficie" in estacoesValidas )
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
