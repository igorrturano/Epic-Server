use uo;
use os;
use polsys;
use cfgfile;
use math;

include "/include/epicPropNames";
include ":propagacao:/include/util/propagacaoConstants";

function MontaCatalogoDano()
	var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
	var base := GetConfigStringArray(propagacaoConfig["incendio"],"danoPorEstadoBase");
	var catalogo := dictionary;
	foreach itemCatalogo in base
      Sleepms(2);
      var estadoEDano := SplitWords(itemCatalogo);
      catalogo.insert(estadoEDano[1],estadoEDano[2]);
    endforeach

	return catalogo;
endfunction

function BuscaConfiguracaoTiles(item)
	var tiles_cfg := ReadConfigFile( "::tiles" );
	var graphic := item.graphic ? item.graphic : item.objtype;

	return tiles_cfg[graphic];
endfunction

function EncontraMarcadorPropagacao(x,y, z, realm, tipoPropagacaoOrigem, alturaMaxima := 19)
	
	var itemsEncontrados := ListItemsNearLocationOfType( x, y, z,0, MARCADOR_PROPAGACAO_OBJTYPE, realm);
	//Print("*PROPAGACAO* itemsEncontrados: {} :".format(itemsEncontrados.size()));
	foreach itemEncontrado in itemsEncontrados
		Sleepms(2);
		var propPropagacaoEncontrada := GetObjProperty(itemEncontrado,PROPNAME_PROP_PROPAGACAO);
		var isAoAlcance:= PegaAbsoluta(z,itemEncontrado.z) <= alturaMaxima;
		if(IsMarcadorPropagacaoValido(itemEncontrado,z,alturaMaxima,tipoPropagacaoOrigem))
			return itemEncontrado;
		endif
	endforeach
	return error{errortext := "Não encotrado."};
endfunction

function IsMarcadorPropagacaoValido(marcador, z, alturaMaxima,tipoPropagacaoOrigem)
	var propPropagacaoEncontrada := GetObjProperty(marcador,PROPNAME_PROP_PROPAGACAO);
	var isAoAlcance:= PegaAbsoluta(z,marcador.z) <= alturaMaxima;
	return propPropagacaoEncontrada.tipoPropagacao == tipoPropagacaoOrigem && isAoAlcance;
endfunction

function CriaMarcadorPropagacaoAdjacente(esteItemMarcador, x,y,z,realm, propPropagacao, estadoInicial := ESTADO_PROPAGACAO::DORMENTE)
	var structPropagacao := NovaPropPropagacao(
								cint(propPropagacao.geracao) + 1,
								propPropagacao.origem,
								esteItemMarcador.serial,
								propPropagacao.tipoPropagacao,
								estadoInicial,
								propPropagacao.esteMarcadorPid,
								0,
								0,
								0,
								0,
								propPropagacao.isMarcadorCoberto
							);

	return CriarMarcadorPropagacao(x,y,z,realm,structPropagacao, true);
	
endfunction

function CriarMarcadorPropagacao(x,y,z,realm,propPropagacao, isAdjacente := false)
	
	var marcadorExistente;

	if(!isAdjacente)
		marcadorExistente :=  EncontraMarcadorPropagacao(x,y,z,realm,propPropagacao.tipoPropagacao,5);
	endif
	
	if(!marcadorExistente)
		var marcadorDescriptor := GetItemDescriptor(MARCADOR_PROPAGACAO_OBJTYPE);
		marcadorDescriptor.CProps.insert(PROPNAME_PROP_PROPAGACAO,propPropagacao);
		marcadorDescriptor.CProps.insert(PROPNAME_INDESTRUTIVEL,true);

		var marcador := CreateItemAtLocation(x,y,z,marcadorDescriptor,1,realm);
		return marcador;
	else 
		ConfiguraExistente(marcadorExistente, propPropagacao);
		return marcadorExistente;
	endif
endfunction

function ConfiguraExistente(marcadorExistente, novaProp)
	var velhaProp := GetObjProperty(marcadorExistente, PROPNAME_PROP_PROPAGACAO);
	if(velhaProp)
		novaProp.esteItemMarcador.serial := velhaProp.esteItemMarcador.serial;
		novaProp.esteMarcadorPid := velhaProp.esteMarcadorPid;
	endif
	SetObjProperty(marcadorExistente,PROPNAME_PROP_PROPAGACAO,novaProp);
	marcadorExistente.Abastecer(true);
endfunction

function NovaPropPropagacao(
						geracao := 0,
						origem := 0,
						marcadorOrigem := -1,
						tipoPropagacao := PROPAGACAO_TIPO::INCENDIO,
						estadoInicial := ESTADO_PROPAGACAO::DORMENTE,
						marcadorOrigemPid := 0, 
						esteMarcadorPid := 0,
						catalogoDano := 0,
						efeitosPersonalizados := 0,
						combustivelExterno := 0,
						isMarcadorCoberto := 0
						)

	var prop := struct{
				geracao := geracao,
				origem := origem,
				marcadorOrigem := marcadorOrigem,
				tipoPropagacao := tipoPropagacao, 
				estadoInicial := estadoInicial,
				catalogoDano := catalogoDano ? catalogoDano : MontaCatalogoDano(),
				efeitosPersonalizados := efeitosPersonalizados,
				combustivelExterno := combustivelExterno,
				isMarcadorCoberto := isMarcadorCoberto,
				lerEstadoAtual := estadoInicial
			};

	if( esteMarcadorPid)
		prop.+marcadorOrigemPid := marcadorOrigemPid;
	endif

	if( esteMarcadorPid)
		prop.+esteMarcadorPid := esteMarcadorPid;
	endif

	return prop;
endfunction

function IsLimiteGeracaoAlcancado( geracaoAtual)
	var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
	var geracaoMaximo := cint(propagacaoConfig["sistema"].geracaoMaxima);
	geracaoMaximo := geracaoMaximo ? geracaoMaximo : -1;
	
	//Print(" geracaoAtual --- " + geracaoAtual + " --- " + geracaoMaximo);
	if(geracaoMaximo != -1)
		return cint(geracaoAtual) >= geracaoMaximo;
	else
		return false;
	endif
endfunction

function PegaAbsoluta(z, zAlvo)
	return Cint(Abs(z - zAlvo));
endfunction

function NovoEfeitoPersonalizado( byref dictEfeito, 
										estado,
										graphic, 
										cor :=0, 
										isInvisibel := 1, 
										sfx := 0,
										efx := -1, 
										duracao := 10,
										effectItemObjtype := 0, 
										chanceRandom := 2,
										minRandom := 1, 
										maxRandom := 8)
	if(dictEfeito)
		dictEfeito.insert(cstr(estado), struct {
											graphic := graphic, 
											cor := cor, 
											isInvisibel := isInvisibel, 
											sfx := sfx,
											efx := efx, 
											duracao := duracao, 
											effectItemObjtype := effectItemObjtype, 
											chanceRandom := chanceRandom,
											minRandom := minRandom,
											maxRandom := maxRandom});
		return 1;
	else
		return error{ error_text := "É necessário um dicionario como referência"}; 
	endif

endfunction

function NovoStructCombustivelMagico(combustivel,fator)
	return struct {combustivel := combustivel, fator := fator};
endfunction

function PegaProcessoMarcador(marcadorPropagacao)
	var propPropagacao := GetObjProperty(marcadorPropagacao, PROPNAME_PROP_PROPAGACAO);
    return GetProcess(cint(propPropagacao.esteMarcadorPid)); 
endfunction