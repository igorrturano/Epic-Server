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
include ":itemutils:itemInfo";

var LISTA_TIPO_MATERIAL := PegaListaTipoMateriais();
var itemsConfig := ReadConfigFile(":*:itemdesc");

function ClassificaNomeReceita(nomeReceita)
	var dividido := SplitWords(nomeReceita,"-");
	var classe := CLASSE_PADRAO;

	if(TypeOfInt(dividido[2]) == OT_ERROR || TypeOfInt(dividido[2]) == OT_UNINIT || TypeOfInt(dividido[2]) == OT_UNKNOWN) 
		classe := CLASSE_PADRAO;
	else
		classe := dividido[2];
	endif

	return dictionary {
		"nome" -> dividido[1],
		"classe" -> classe
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
		COMPONENTE_TIERS::MEDIOCRE : return struct {id:= COMPONENTE_TIERS::MEDIOCRE, desc := "Normal" , valor := 0}; break;
		COMPONENTE_TIERS::COMUM : return struct {id:= COMPONENTE_TIERS::COMUM, desc := "Melhorado" , valor := 1}; break;
		COMPONENTE_TIERS::NOTAVEL : return struct {id:= COMPONENTE_TIERS::NOTAVEL, desc := "Notável" , valor := 2}; break;
		COMPONENTE_TIERS::SUPERIOR : return struct {id:= COMPONENTE_TIERS::SUPERIOR, desc := "Superior" , valor := 3}; break;
		COMPONENTE_TIERS::EXCEPCIONAL : return struct {id:= COMPONENTE_TIERS::EXCEPCIONAL, desc := "Excepcional" , valor := 4}; break;
		default:
			return struct {id := error, desc := "Erro!" , valor := 0}; break;
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

function PegaDificuldadeFrabicarObjeto(quem, objtype, nomeProfissao, nomePericia, classe, sucessosNecessarios , produto := 0, estacao  := 0)
	var skill :=  PegaSkillNecessaria(objtype, nomeProfissao, classe);
	return PegaDificuldade(quem, skill, nomePericia,sucessosNecessarios, produto, estacao);
endfunction 

function PegaDificuldade( quem, skillNecessaria, nomePericia, sucessosNecessarios, produto := 0, estacao  := 0)
	 var retorno := struct;
	 
	 retorno.+skill := CDbl(skillNecessaria);
	 retorno.+chancePelaSkill := 0;
	 retorno.+modificadoresChance := array;
	 retorno.+chanceModificadaTotal := 0;
	 retorno.+chanceBase := PegaChanceFabricacaoBase(quem,
												nomePericia,
												retorno.skill,
												retorno.chancePelaSkill,
												sucessosNecessarios);
	
	
	var propFabricado := GetObjProperty(produto, PROPNAME_FABRICADO );
	
	if(propFabricado)
		foreach componente in (propFabricado.esquema.componentesEscolhidos)
			Sleepms(2);
			
			var chanceCalculada := cint(componente.poderAvaliado.poderContado) * -1;
			
			var modificador := struct{
				origem := BLUE + "{}Dificuldade {}<br>".format(chanceCalculada < 0? RED_FIRE: SOFT_GREEN, componente.item.nome),
				valor :=  chanceCalculada
			};

			retorno.modificadoresChance.append(modificador);
			retorno.chanceModificadaTotal += chanceCalculada;
			retorno.chancePelaSkill += chanceCalculada;
		endforeach
	endif

	retorno.+chanceTotal := retorno.chanceBase + retorno.chanceModificadaTotal;
	retorno.chanceTotal := cdbl(retorno.chanceTotal) < 0.0  ? 0.0 : cdbl(retorno.chanceTotal);
	retorno.+chanceTratamentoFinal := PegaChanceTratamentoFinal(retorno.chanceTotal / 100, sucessosNecessarios);
	return retorno;
endfunction

function PegaChanceTratamentoFinal(chanceSucesso,sucessosNecessarios)

	sucessosNecessarios := cint(sucessosNecessarios);
	var chanceFalha := 1.0 - chanceSucesso;
	var chanceFinal := 0.0;

	for y := 0 to (PegaMaximoFalhas(0,sucessosNecessarios) - 1)
		sleepms(2);
		var combination := Comb(sucessosNecessarios + y - 1 , y);
		chanceFinal := chanceFinal + combination * Pow(chanceSucesso, sucessosNecessarios) * Pow(chanceFalha, y);
	endfor
	
	return Trunc(chanceFinal * 100, 1);
endfunction

function PegaDeslocamentoFacilitador(sucessos)
	if(sucessos >= 14)
		return 46;
	elseif(sucessos > 10)
		return 40;
	elseif(sucessos > 8)
		return 20;
	elseif(sucessos > 6)
		return 10;
	else
		return 0;
	endif
endfunction

function PegaChanceFabricacaoBase(quem, pericia, dificuldadeItem, byref chancePelaSkill, sucessosNecessarios)
	var valorPericia := AP_GetTrueSkill(quem, pericia);
	valorPericia := ApplySkillModifiers(quem, pericia, valorPericia); 
	chancePelaSkill := CalcSuccessPercent(quem, pericia,CDbl(dificuldadeItem), CDbl(valorPericia));
	if(chancePelaSkill == 0)
		return 0;
	endif
	var chanceFacilitada := chancePelaSkill + PegaDeslocamentoFacilitador(sucessosNecessarios);
	if(chanceFacilitada >= 100)
		return 100;
	endif
	return  Trunc(chanceFacilitada, 1);
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
			if(!descriptor.movable)
				descriptor.movable := 1;
			endif
			descriptor.CProps.insert(PROPNAME_MATERIAL,propFabricadoMontado.esquema.material);
			descriptor.CProps.insert(PROPNAME_DISPENSAVEL_FLAG,true);
		endif

		descriptor.CProps.insert(PROPNAME_FABRICADO, CriaNovaPropFabricado(propFabricadoMontado));
		descriptor.maxhp := propFabricadoMontado.esquema.modificadorHpTotal? descriptor.maxhp + propFabricadoMontado.esquema.modificadorHpTotal : descriptor.maxhp;

		if(!propFabricadoMontado.receita.isEmpilhavel)
			descriptor.CProps.insert(PROPNAME_ARTIFICE_SERIAL,quem.serial);
		endif

		descriptor.color := propFabricadoMontado.receita.corObrigatoria ? propFabricadoMontado.receita.corObrigatoria : propFabricadoMontado.esquema.cor;
		var quantidadeFinal := 1;
		if(propFabricadoMontado.receita.lote && propFabricadoMontado.receita.lote > 1)
			quantidadeFinal := cint(quantidade * propFabricadoMontado.receita.lote);
		endif
			
		if(!propFabricadoMontado.receita.isEmpilhavel)
			for i := 1 to quantidadeFinal
					Sleepms(2);
					var item := CreateItemInBackpack(quem,descriptor);
					if(!item)
						item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, 1, quem.realm);
					endif

					AplicaAtributosItemFabricado(quem,item, propFabricadoMontado);
			endfor
		else
			var item := CreateItemInBackpack(quem,descriptor, quantidadeFinal);
			if(!item)
				item := CreateItemAtLocation( quem.x, quem.y, quem.z, descriptor, cint(quantidadeFinal), quem.realm);
			endif

			AplicaAtributosItemFabricado(quem,item, propFabricadoMontado);
		endif
	else
		SendSysMessageEX(quem, "O Item " +  produto.desc + " não é um Produto Montado.",SSM_FAIL);
		Print("Erro: O Item " +  produto.desc + " não é um Produto Montado.");
		return 0;
	endif
	
	return 1;
endfunction

function AplicaAtributosItemFabricado(quem, item, propFabricadoMontado)
	if(!item)
		SendSysMessageEX(quem, "Erro [" + item + "] ao criar objeto.",SSM_FAIL);
		return 0;
	endif

	item.maxhp_mod := propFabricadoMontado.esquema.modificadorHpTotal? propFabricadoMontado.esquema.modificadorHpTotal : 0;
	item.hp := item.maxhp;
	item.weight_multiplier_mod := propFabricadoMontado.esquema.modificadorPesoTotal ? CDbl((propFabricadoMontado.esquema.modificadorPesoTotal / 100) + 1) : 1;
	
	var infoPote := MontaInfoPote(quem,  CInt(propFabricadoMontado.receita.objtype));
	if(infoPote) //Pote Alquimia 
		var quantidadeDoses := PegaValorFabricacao(item, "doses");
		if(quantidadeDoses)
			SetObjProperty(item, "doses", quantidadeDoses);
		endif
		SetObjProperty(item, "tier", infoPote.tier);
		SetObjProperty(item, "crafterskill", infoPote.crafterskill);
		SetObjProperty(item, "science", infoPote.science);
		SetObjProperty(item, "bonus", infoPote.bonus);
	endif

	AplicaFechadura(quem, item);
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
	var retornoEfeitos := struct;
	if(FIND(UPPER(caracteristica),"VAZIO", 1) > 0)
		retornoEfeitos.+valorPrincipal := 0;
		retornoEfeitos.+valorLiteralPrincipal := 0;
		retornoEfeitos.+valorTotalSubefeitos := 0;
		return retornoEfeitos;
	endif
	
	var configRecursos := ReadConfigFile(RECURSOS_CONFIG_PATH);
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
			componentesRefatorado.insert(componente.componenteNecessario.caracteristica,componente);
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
	var estacoesValidas := GetConfigString(profissaoConfig[profissao], "EstacoesValidas");
	estacoesValidas := SplitWords(estacoesValidas);
	
	foreach estacaoValida in estacoesValidas
		Sleepms(2);
		if(Lower(cstr(hex(estacaoAlvo.item.objtype))) == lower(cstr(hex(estacaoValida))) ||  "superficie" == estacaoValida)
			var ocupanteSerial := GetObjProperty(estacaoAlvo.item,PROPNAME_ISESTACAO_OCUPADA);
			
			if(ocupanteSerial)
				var ocupante := SystemFindObjectBySerial(ocupanteSerial);
				if(ocupante && Distance(ocupante,estacaoAlvo) <= 2)
					SendSysMessageEX(quem,"A estação está em uso.", SSM_FAIL);
					return 0;
				endif
			endif
			SetObjProperty(estacaoAlvo.item,PROPNAME_ISESTACAO_OCUPADA, quem.serial);
			return 1;
		endif
	endforeach
	SendSysMessageEX(quem,"Estação inválida.", SSM_FAIL);
	return 0;
endfunction

function PegaDescDuracaoFabricacao(tagTempo)
	case(tagTempo)
		"EXTREMAMENTE_LONGO": return struct{ desc := "Extremamente Longo" , sucessos := 14}; break;
		"MUITO_LONGO": return struct{ desc := "Muito Longo" , sucessos := 12}; break;
		"LONGO": return struct{ desc := "Longo" , sucessos := 10}; break;
		"EXTREMAMENTE_RAPIDO": return struct{ desc := "Extremamente Rápido" , sucessos := 2}; break;
		"MUITO_RAPIDO": return struct{ desc := "Muito Rápido" , sucessos := 4}; break;
		"RAPIDO": return struct{ desc := "Rápido" , sucessos := 6}; break;
		default: return struct{ desc := "Razoável" , sucessos := 8}; break;
	endcase
endfunction

function PegaMaximoFalhas(item, sucessos := 0)
		if(sucessos == 0)
			var propFabricado := GetObjProperty(item,PROPNAME_FABRICADO);
			sucessos := propFabricado.receita.sucessosNecessarios;
		endif

		return cint(sucessos * MULTIPLICADOR_FALHAS);
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

function MontaInfoPote(quem, receitaObjtype)
	var alchemyRecipes := ReadConfigFile(":alchemy:itemdesc");
	var science := (alchemyRecipes[receitaObjtype].science);
	
	if(science)
		var crafterskill := Cint((AP_GetSkill(quem, SCIENCE))/10);
		var crafterInt := AP_GetTrueStat (quem, INTELLIGENCE);
		var bonus := (((crafterInt-50)/10)+(crafterskill));

		return struct {
			tier := alchemyRecipes[receitaObjtype].tier,
			crafterskill := crafterskill * 10,
			science := alchemyRecipes[receitaObjtype].science,
			bonus := bonus
		};
	endif
	return 0;
endfunction

function DefinePropFabricado(esquema, receita , quem)
	//Limpa propriedades desnecessárias ou rendundantes.
	receita.-componentesNecessarios;
	receita.-materiaisNecessarios;
	receita.-textoDescItem;
	receita.-dificuldade;
	receita.-isVisualizada;
	receita.-uso;

	esquema.-modificadorCache;

	var propFabricado := struct{ 
		estado := ITEM_MONTADO,
		receita := receita,
		esquema := esquema};
	
	if(!receita.isEmpilhavel)
		propFabricado.+fabricante := quem.serial;
	endif

	return propFabricado;
endfunction

function GetMaterialName(obj);

  if ( (obj == "wood") || (obj == "leather") || (obj == "cloth") || (obj == "metal") || (obj == "glass") || (obj == "ossos") || (obj == "stone") || obj == "arrowhead")
    return obj;
  else
    var items := ReadconfigFile("::itemdesc");
    if (items[obj])
      return items[obj].desc;
    endif
  endif

endfunction

function TruncatePlural(obj_type, add_S := 0)
        obj_type := CInt(obj_type);

        var config := ReadConfigFile(":*:itemdesc");
        config := config[obj_type];

        var name;
        if (config.desc)
                name := config.desc;
        elseif (config.name)
                name := config.name;
        else
                return error{"errortext":="Invalid Objtype : " + hex(obj_type) + "."};
        endif

        if ( !name["%"] )
                if ( add_S == 0 )
                        return name;
                endif
                if ( name[len(name)] in array{"s"} )
                        return name;
                elseif ( name[len(name)] in array{"h","j","x","z"} )
                        return name + "es";
                endif
                return name + "s";
        endif

        // Will deal with:
        //
        //      Bread Stick%s
        //      Bread Loa%ves/f%
        //      Lump%s% of Coal
        //      Loa%ves/f% of Bread
        //
        var first := find(name, "%", 1);
        var slash := find(name, "/", first);
        var second := find(name, "%", first + 1);
        if ( slash && !second )
                // Force-append a % if a '/' exists!
                name := name + "%";
                second := len(name);
        elseif ( !slash )
                slash := second;
        endif

        // wwwww%xx[/yy]%[zzzzz]
        // 12345678     9 01234
        // 12345678 901 2 34567

        var s_plur := name[first+1, slash-(first+1)];
        var s_sing := "";
        if ( slash != first )
                s_sing := name[slash+1, second-(slash+1)];
        endif

        var s_first := name[1, first-1];
        var s_middle := s_plur;
        var s_last := "";

        if ( len(name) > second )
                s_last := name[second+1, len(name)-second];
        endif

        if ( add_S == 0 )
                s_middle := s_sing;
        endif

        if ( !s_first )
                s_first := "";
        endif
        if ( !s_middle )
                s_middle := "";
        endif
        if ( !s_last )
                s_last := "";
        endif
        return s_first + s_middle + s_last;

endfunction

