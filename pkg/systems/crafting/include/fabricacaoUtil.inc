//Author: Deiro || Rafael Santos

use uo;
use os;
use basic;
use math;
use cfgfile;

include "/include/math";
include ":attributes:attributes";
include "/include/epicPropNames";
include "/include/epicConstants";
include ":keys:key";
include ":crafting:/include/caracteristicasFabricacao";
include ":crafting:/include/util/fabricacaoConstants";
include "include/epicUtils";

var LISTA_TIPO_MATERIAL := PegaListaTipoMateriais();
var itemsConfig := ReadConfigFile(":*:itemdesc");

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

function PegaMaterialConfig(materialObjtype)
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	return configRecursos[hex(materialObjtype)];
endfunction

function PegaListaTipoMateriais()
	var configRecursos := ReadConfigFile(":crafting:config/resourceList");
	var materiaisChaves :=  GetConfigStringKeys(configRecursos);
	var dictRetorno := Dictionary;
	
	foreach chave in materiaisChaves
		Sleepms(2);
		var elementoCfg := configRecursos[chave];
		var tipo := elementoCfg.Type;
		tipo := Upper(tipo);
		if(!dictRetorno.exists(tipo))
			dictRetorno.insert(tipo, array);
		endif

		dictRetorno[tipo].append(struct{chave := chave, name := elementoCfg.Name});

	endforeach

	return dictRetorno;
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
function DistribuiQualidadeComponentes(quantidade,valorPico, deslocamentoPico := -0.7) 
	
	if(quantidade == error || quantidade.size() == 0)
		return error{"errortext":="O parâmetro quantidade é obrigatório."};
	endif

	//quanto menor o declinio, mais espalhado fica o resultado.
	//o pico é o equivalente a qual valor bate o pico no eixo y.
	var	chancesPorTier := DistribuicaoGaussianaNormalizada({1,2,3,4,5},valorPico + deslocamentoPico,1); 
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
		COMPONENTE_TIERS::MEDIOCRE : return struct {desc := "Normal" , valor := 0}; break;
		COMPONENTE_TIERS::COMUM : return struct {desc := "Melhorado" , valor := 1}; break;
		COMPONENTE_TIERS::NOTAVEL : return struct {desc := "Notável" , valor := 2}; break;
		COMPONENTE_TIERS::SUPERIOR : return struct {desc := "Superior" , valor := 3}; break;
		COMPONENTE_TIERS::EXCEPCIONAL : return struct {desc := "Excepcional" , valor := 4}; break;
		default:
			return struct {desc := "Erro!" , valor := 0}; break;
	endcase
endfunction

function PegaComponenteTiers()
	return array{
				COMPONENTE_TIERS::MEDIOCRE,
				COMPONENTE_TIERS::COMUM,
				COMPONENTE_TIERS::NOTAVEL,
				COMPONENTE_TIERS::SUPERIOR,
				COMPONENTE_TIERS::EXCEPCIONAL
			};
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

function CriaItemFabricado(quem, produto, quantidade := 1, propFabricado := 0)
	var propFabricadoMontado;

	if(propFabricado)
		propFabricadoMontado := propFabricado;
	else
		propFabricadoMontado := GetObjProperty(produto, PROPNAME_FABRICADO);
	endif

	if(propFabricadoMontado)
		var descriptor;
		var receitaObjtype :=  CInt(propFabricadoMontado.receita.objtype);

		if(propFabricadoMontado.receita.complexoComponente) // Se for item complexo que é considerado componente. [ex: fechadura]
			var qualidades := DistribuiQualidadeComponentes(1,CalculaValorPico(quem,propFabricadoMontado.receita.pericia));
			
			descriptor := CriaComponenteDescriptor(
											receitaObjtype,
											propFabricadoMontado.esquema.material,
											qualidades.keys()[1],
											propFabricadoMontado.receita.complexoComponente.encaixe,
											ReadConfigFile(RECURSOS_CONFIG_PATH)
											);
		else
			descriptor := GetItemDescriptor(receitaObjtype);
			descriptor.CProps.insert(PROPNAME_MATERIAL,propFabricadoMontado.esquema.material);
			descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
		endif

		descriptor.CProps.insert(PROPNAME_FABRICADO, CriaNovaPropFabricado(propFabricadoMontado));
		descriptor.maxhp := propFabricadoMontado.esquema.modificadorHpTotal? descriptor.maxhp + propFabricadoMontado.esquema.modificadorHpTotal : descriptor.maxhp;
		//TODO trecho a seguir foi trazido do antigo sistema de Alchemy. remover uma vez que o novo sistema estiver UP.
		var alchemyRecipes := ReadConfigFile(":alchemy:itemdesc");
		var science := (alchemyRecipes[receitaObjtype].science);
		if(science)
			var tier := (alchemyRecipes[receitaObjtype].tier);
			var crafterskill := Cint((AP_GetSkill(quem, SCIENCE))/10);
			var crafterInt := AP_GetTrueStat (quem, INTELLIGENCE);
			var bonus := (((crafterInt-50)/10)+(crafterskill));
			descriptor.CProps.insert("bonus",bonus);
			descriptor.CProps.insert("tier",tier);
			descriptor.CProps.insert("crafterskill",crafterskill*10);
			descriptor.CProps.insert("science",science);
		endif

		if(!propFabricadoMontado.receita.isEmpilhavel)
			descriptor.CProps.insert(PROPNAME_ARTIFICE_SERIAL,quem.serial);
		endif

		descriptor.color := propFabricadoMontado.esquema.cor;

		var quantidadeFinal := 1;
		if(propFabricadoMontado.receita.lote && propFabricadoMontado.receita.lote > 1)
			quantidadeFinal := cint(quantidade * propFabricadoMontado.receita.lote);
		endif
			
		var item := CreateItemInBackpack(quem,descriptor, quantidadeFinal);
		if(!item)
			item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, cint(quantidadeFinal), quem.realm);
		endif

		item.maxhp_mod := propFabricadoMontado.esquema.modificadorHpTotal? propFabricadoMontado.esquema.modificadorHpTotal : 0;
		item.hp := item.maxhp;
		
		if(!item)
			SendSysMessageEX(quem, "Erro [" + item + "] ao criar objeto.",SSM_FAIL);
			return 0;
		endif

		AplicaFechadura(quem, item);
		item.setAttrBase();
		item.weight_multiplier_mod := propFabricadoMontado.esquema.modificadorPesoTotal ? ((propFabricadoMontado.esquema.modificadorPesoTotal / 100) + 1) : 0;
	
	else
		SendSysMessageEX(quem, "O Item " +  produto.desc + " não é um Produto Montado.",SSM_FAIL);
		Print("Erro: O Item " +  produto.desc + " não é um Produto Montado.");
		return 0;
	endif
	
	return 1;
endfunction

function ResolveEfeitoCaracteristica(valorEfeito)
	var isFormatoTier := Find(valorEfeito,"t-",1);
	
	if(isFormatoTier)
		var tier := Cint(SubStr(valorEfeito,isFormatoTier + 2, 1));
		return PegaComponenteTierDesc(tier).valor;
	else
		return Cint(valorEfeito);
	endif

endfunction

function InterpretaMaterialEfeitos(caracteristica, chaveMaterial)
	var configRecursos := ReadConfigFile(RECURSOS_CONFIG_PATH);
	var retornoEfeitos := struct;
	var efeitosMaterial := GetConfigStringDictionary(configRecursos[chaveMaterial],"ComponenteEfeito");
	var efeitosMaterialString := efeitosMaterial[caracteristica];
	var subEfeitosMaterial;
	var valorEfeitoMaterial := 0;

	if(efeitosMaterialString)
		subEfeitosMaterial := SplitWords(efeitosMaterialString);
		valorEfeitoMaterial := ResolveEfeitoCaracteristica(subEfeitosMaterial[subEfeitosMaterial.size()]);
		retornoEfeitos.+valorPrincipal := valorEfeitoMaterial;
		retornoEfeitos.+valorLiteralPrincipal := subEfeitosMaterial[subEfeitosMaterial.size()];
		retornoEfeitos.+valorTotalSubefeitos := 0;
		
		if(subEfeitosMaterial.size() > 1)
			subEfeitosMaterial.shrink(subEfeitosMaterial.size() - 1); // Remove o valorPrincipal e deixa só os subefeitos.
			var subefeitosDict := dictionary{};

			for indice := 1 to subEfeitosMaterial.size()
				var valorSubEfeito := ResolveEfeitoCaracteristica(subEfeitosMaterial[indice + 1]);
				subefeitosDict.insert( subEfeitosMaterial[indice], valorSubEfeito); // indice 1 = nomeSubEfeito, indice 2 = valorSubEfeito
				retornoEfeitos.valorTotalSubefeitos += valorSubEfeito;
				++indice;// dessa forma o indice sobe de 2 em dois, pois o for já adiciona +1.
				Sleepms(2);
			endfor
			retornoEfeitos.+efeitosAdicionais := subefeitosDict;
		endif 
	endif
	
	return retornoEfeitos;
endfunction

function AplicaFechadura(quem, item)
	var levelFechadura := PegaValorFabricacao(item, CARACTERISTICA_FABRICACAO_TRANCA);
	
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

function CriaNovaPropFabricado ( propFabricadoMontado, estado := ITEM_PRONTO)
	var componentesRefatorado := dictionary;
	if(estado != ITEM_GERADO)
		//Refatora o dict para usar como indice a caracteristica, assim dá para ir direto ao assunto se quiser pegar o componente que tá dando aquela caracteristica.
		foreach componente in (propFabricadoMontado.esquema.componentesEscolhidos)
			Sleepms(2);
			if(componente.componenteNecessario.caracteristica != "vazio")
				componentesRefatorado.insert(componente.componenteNecessario.caracteristica,componente);
			else
				componentesRefatorado.insert(_componente_iter,componente);
			endif
		endforeach
	else
		componentesRefatorado := propFabricadoMontado.esquema.componentesEscolhidos;
	endif

	var corFinal :=  propFabricadoMontado.receita.corObrigatoria ? propFabricadoMontado.receita.corObrigatoria : propFabricadoMontado.esquema.cor;
	return CriaObjetoPropFabricado(
							estado,
							propFabricadoMontado.esquema.isCompletamenteComum,
							propFabricadoMontado.esquema.qualidade,
							propFabricadoMontado.esquema.modificadorHpTotal,
							propFabricadoMontado.esquema.modificadorPesoTotal,
							corFinal,
							propFabricadoMontado.esquema.classe,
							componentesRefatorado,
							propFabricadoMontado.fabricante ? propFabricadoMontado.fabricante : 0,
							propFabricadoMontado.esquema.material
							);
endfunction

function CriaObjetoPropFabricado(
									estado, 
									isCompletamenteComum,
									qualidade, 
									modificadorHpTotal,
									modificadorPesoTotal,
									cor, 
									classe, 
									componentes, 
									fabricante,
									material
									)
	return struct{
				estado := estado ,
				isCompletamenteComum := isCompletamenteComum,
				qualidade := qualidade,
				modificadorHpTotal := modificadorHpTotal,
				modificadorPesoTotal := modificadorPesoTotal,
				cor := cor,
				classe := classe,
				componentes := componentes,
				fabricante := fabricante,
				materialPrincipal := material};
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
				return 0;
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
	case(qtdSucessos)
		"EXTREMAMENTE_LONGO": return struct{ desc := "Extremamente Longo" , sucessos := 24}; break;
		"MUITO_LONGO": return struct{ desc := "Muito Longo" , sucessos := 20}; break;
		"LONGO": return struct{ desc := "Longo" , sucessos := 15}; break;
		"EXTREMAMENTE_RAPIDO": return struct{ desc := "Extremamente Rápido" , sucessos := 2}; break;
		"MUITO_RAPIDO": return struct{ desc := "Muito Rápido" , sucessos := 5}; break;
		"RAPIDO": return struct{ desc := "Rápido" , sucessos := 8}; break;
		default: return struct{ desc := "Razoável" , sucessos := 12}; break;
	endcase
endfunction

function MontaCraftProp(fabricacaoConfig, prop)

	if(TypeOfInt(prop) == OT_STRUCT)
		
		var teste := prop.?deveReciclar;
		if(!(prop.?deveReciclar))
			prop.+deveReciclar := false;
		endif

		if(prop.pericias && prop.pericias.size() > 0)
			return prop;
		endif
	endif

	var propRetorno := struct {
		pericias  := dictionary,
		encaixesMemorizados := array,
		deveReciclar := false
	};

	foreach pericia in GetConfigStringArray(fabricacaoConfig["Skills"], "Skill")
		var temp := dictionary;
		var visualizadas := struct;
		visualizadas.+receitas := array;
		visualizadas.+categorias := array;

		temp.+skill  := fabricacaoConfig[pericia].Attribute;
		temp.+visualizadas := visualizadas;
		temp.+last   := 0;
		propRetorno.pericias[temp.skill] := temp;
		Sleepms(2);
	endforeach
	
	return propRetorno;

endfunction

function LimparMemoriaEncaixes(quem)

	var propCraft := GetObjProperty(quem, "Craft");
	if(propCraft)
		propCraft.encaixesMemorizados := array;
		SetObjProperty(quem, "Craft", propCraft);
	endif

endfunction

function PegaTipoMaterialAleatorio(tipoMaterial)
	var recursosConfig := ReadConfigFile(":crafting:config/resourceList");
	var chavesElemento := GetConfigStringKeys( recursosConfig );
	var materiais := array;
	
	foreach chave in chavesElemento
		var recurso := recursosConfig[chave];
		Sleepms(2);
		if(UPPER(recurso.type) == UPPER(tipoMaterial))
			materiais.append(chave);
		endif
	endforeach

	return materiais[RandomIntMinMax(1,materiais.size())];
endfunction