use uo;
use os;
use polsys;
use cfgfile;
use math;

include "/include/epicPropNames";

const MARCADOR_PROPAGACAO_OBJTYPE := 0xe13a;

enum propagacaoEvents
	PROPAGACAO_EV_TIMEOUT, // OBS: Esta tem que ser a posicao zero, pois é o retorno da wait_for_event.
	PROPAGACAO_EV_ESTIMULO,
	PROPAGACAO_EV_COMBUSTIVEL,
	PROPAGACAO_EV_RESFRIAMENTO,
endenum

enum tipoPropagacao
	PROPAGACAO_TIPO_INCENDIO := 1,
endenum

enum resfriamentos
	PROPAGACAO_RESFRIAMENTO_BASICO := 10,
	PROPAGACAO_RESFRIAMENTO_RAZOAVEL := 55,
	PROPAGACAO_RESFRIAMENTO_INTENSO := 300,
endenum

enum tipoCombustiveis
	PROPAGACAO_COMBUSTIVEL_TERRENO := "terrenoInflamavel",
	PROPAGACAO_COMBUSTIVEL_ESTATICOS := "staticInflamavel",
	PROPAGACAO_COMBUSTIVEL_MAGICO := "magico",
endenum

enum estadosMarcadorPropagacao
	ESTADO_PROPAGACAO_DORMENTE, //Só aguarda estimulo.
	ESTADO_PROPAGACAO_LATENTE, //Danifica, mas não propaga
	ESTADO_PROPAGACAO_PRINCIPIO, //Danifica e propaga
	ESTADO_PROPAGACAO_REGULAR, //Danifica + e propaga
	ESTADO_PROPAGACAO_INTENSO //Danifica ++ e propaga
endenum

enum tipoCalculoEnergia
	CALCULO_ENERGIA_GRAPHIC, 
	CALCULO_ENERGIA_LANDTILE, 
endenum

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

function EncontraMarcadorPropagacao(x,y, z, realm, tipoPropagacaoOrigem, alturaMaxima := 19)
	
	var itemsEncontrados := ListItemsNearLocationOfType( x, y, z,0, MARCADOR_PROPAGACAO_OBJTYPE, realm);
	//Print("*PROPAGACAO* itemsEncontrados: {} :".format(itemsEncontrados.size()));
	foreach itemEncontrado in itemsEncontrados
		Sleepms(2);
		var propPropagacaoEncontrada := GetObjProperty(itemEncontrado,PROPNAME_PROP_PROPAGACAO);
		var isAoAlcance:= PegaAbsoluta(z,itemEncontrado.z) <= alturaMaxima;
		if(propPropagacaoEncontrada.tipoPropagacao == tipoPropagacaoOrigem && isAoAlcance)
			return itemEncontrado;
		endif
	endforeach
	return error{errortext := "Não encotrado."};
endfunction

function CriaMarcadorPropagacaoAdjacente(esteItemMarcador, x,y,z,realm, propPropagacao, estadoInicial := ESTADO_PROPAGACAO_DORMENTE)
	var structPropagacao := NovaPropPropagacao(
								cint(propPropagacao.geracao) + 1,
								propPropagacao.origem,
								esteItemMarcador.serial,
								propPropagacao.tipoPropagacao,
								estadoInicial,
								propPropagacao.esteMarcadorPid
							);

	return CriarMarcadorPropagacao(x,y,z,realm,structPropagacao);
	
endfunction

function CriarMarcadorPropagacao(x,y,z,realm,propPropagacao)
	var marcadorExistente :=  EncontraMarcadorPropagacao(x,y,z,realm,PROPAGACAO_TIPO_INCENDIO,5);
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
	novaProp.esteItemMarcador.serial := velhaProp.esteItemMarcador.serial;
	novaProp.esteMarcadorPid := velhaProp.esteMarcadorPid;
	SetObjProperty(marcadorExistente,PROPNAME_PROP_PROPAGACAO,novaProp);
endfunction

function NovaPropPropagacao(
						geracao := 0,
						origem := 0,
						marcadorOrigem := -1,
						tipoPropagacao := PROPAGACAO_TIPO_INCENDIO,
						estadoInicial := ESTADO_PROPAGACAO_DORMENTE,
						marcadorOrigemPid := 0, 
						esteMarcadorPid := 0,
						catalogoDano := 0,
						efeitosPersonalizados := 0,
						combustivelMagico := 0
						)

	var prop := struct{
				geracao := geracao,
				origem := origem,
				marcadorOrigem := marcadorOrigem,
				tipoPropagacao := tipoPropagacao, 
				estadoInicial := estadoInicial,
				catalogoDano := catalogoDano,
				efeitosPersonalizados := efeitosPersonalizados,
				combustivelMagico := combustivelMagico
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