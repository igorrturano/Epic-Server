use uo;
use os;
use polsys;
use cfgfile;

include "/include/epicPropNames";

const MARCADOR_PROPAGACAO_OBJTYPE := 0xe13a;

enum propagacaoEvents
	PROPAGACAO_EV_ESTIMULO,
	PROPAGACAO_EV_RESFRIAMENTO,
endenum

enum tipoPropagacao
	PROPAGACAO_TIPO_INCENDIO,
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

function EncontraMarcadorPropagacao(x,y, z, realm)
	
	var itemsEncontrados := ListItemsAtLocation( x, y, z, realm);

	foreach itemEncontrado in itemsEncontrados
		if(itemEncontrado.objtype == MARCADOR_PROPAGACAO_OBJTYPE)
			return itemEncontrado;
		endif
		Sleepms(2);
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
	var marcadorDescriptor := GetItemDescriptor(MARCADOR_PROPAGACAO_OBJTYPE);
	marcadorDescriptor.CProps.insert(PROPNAME_PROP_PROPAGACAO,propPropagacao);
	marcadorDescriptor.CProps.insert(PROPNAME_INDESTRUTIVEL,true);

	var marcador := CreateItemAtLocation(x,y,z,marcadorDescriptor,1,realm);
	return marcador;
endfunction

function NovaPropPropagacao(
						geracao := 0,
						origem := -1,
						marcadorOrigem := -1,
						tipoPropagacao := PROPAGACAO_TIPO_INCENDIO,
						estadoInicial := ESTADO_PROPAGACAO_DORMENTE,
						marcadorOrigemPid := 0, 
						esteMarcadorPid := 0
						)

	var prop := struct{
				geracao := geracao,
				origem := origem,
				marcadorOrigem := marcadorOrigem,
				tipoPropagacao := tipoPropagacao, 
				estadoInicial := estadoInicial
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