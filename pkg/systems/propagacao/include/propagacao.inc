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

function CriaMarcadorPropagacaoAdjacente(esteItemMarcador, x,y,z,realm, propPropagacaoOrigem, estadoInicial := ESTADO_PROPAGACAO::DORMENTE)
	var structPropagacao := struct;

	structPropagacao.+geracao := cint(propPropagacaoOrigem.geracao) + 1;
	structPropagacao.+origem := propPropagacaoOrigem.origem;
	structPropagacao.+marcadorOrigem := esteItemMarcador.serial;
	structPropagacao.+tipoPropagacao := propPropagacaoOrigem.tipoPropagacao;
	structPropagacao.+marcadorOrigemPid := propPropagacaoOrigem.marcadorOrigemPid;
	structPropagacao.+estadoInicial := estadoInicial;
	structPropagacao.+isMarcadorCoberto := propPropagacaoOrigem.isMarcadorCoberto;
	structPropagacao.+geracaoMaxima := propPropagacaoOrigem.geracaoMaxima;

	var marcadorProps := propPropagacaoOrigem.marcadorProps;
	if(marcadorProps)
		if(marcadorProps.devePropagarProps)
			structPropagacao.+marcadorProps := propPropagacaoOrigem.marcadorProps;
		endif
	endif

	return CriarMarcadorPropagacao(x,y,z,realm,structPropagacao, true);
	
endfunction

function CriarMarcadorPropagacao(x,y,z,realm,structPropagacao, isAdjacente := false)
	
	if(structPropagacao.origem == 0 || TypeOfInt(structPropagacao.origem) == OT_UNINIT || TypeOfInt(structPropagacao.origem) == OT_ERROR  )
		Print("[Erro - Propagação] Tentativa de criar marcador de propagação sem origem válida.");
		return error { error_text := "Origem inválida. A Origem é obrigatória." };
	endif

	var marcadorExistente;

	if(!isAdjacente)
		marcadorExistente :=  EncontraMarcadorPropagacao(x,y,z,realm,structPropagacao.tipoPropagacao,5);
	endif
	
	if(!marcadorExistente)
		var propPropagacao := MapeiaPropPropagacao(structPropagacao);
		var marcadorDescriptor := GetItemDescriptor(MARCADOR_PROPAGACAO_OBJTYPE);
		marcadorDescriptor.CProps.insert(PROPNAME_PROP_PROPAGACAO,propPropagacao);
		marcadorDescriptor.CProps.insert(PROPNAME_INDESTRUTIVEL,true);

		var marcador := CreateItemAtLocation(x,y,z,marcadorDescriptor,1,realm);
		if(marcador)
			MarcaLimitadoraGlobal(propPropagacao.origem, 1);
		endif

		return marcador;
	else 
		ConfiguraExistente(marcadorExistente, structPropagacao);
		return marcadorExistente;
	endif
endfunction

function MapeiaPropPropagacao(structRecebido)
	var structPropagacao := NovaPropPropagacao();

	structPropagacao.geracao := structRecebido.geracao ? structRecebido.geracao : structPropagacao.geracao;
	structPropagacao.origem := structRecebido.origem ? structRecebido.origem : structPropagacao.origem;
	structPropagacao.tipoPropagacao := structRecebido.tipoPropagacao ? structRecebido.tipoPropagacao : structPropagacao.tipoPropagacao;
	structPropagacao.estadoInicial := structRecebido.estadoInicial ? structRecebido.estadoInicial : structPropagacao.estadoInicial;
	structPropagacao.marcadorOrigemPid := structRecebido.marcadorOrigemPid ? structRecebido.marcadorOrigemPid : structPropagacao.marcadorOrigemPid;
	structPropagacao.marcadorOrigem := structRecebido.marcadorOrigem ? structRecebido.marcadorOrigem : structPropagacao.marcadorOrigem;
	structPropagacao.esteMarcadorPid := structRecebido.esteMarcadorPid ? structRecebido.esteMarcadorPid : structPropagacao.esteMarcadorPid;
	structPropagacao.marcadorProps := structRecebido.marcadorProps ? structRecebido.marcadorProps : structPropagacao.marcadorProps;
	structPropagacao.efeitosPersonalizados := structRecebido.efeitosPersonalizados ? structRecebido.efeitosPersonalizados : structPropagacao.efeitosPersonalizados;
	structPropagacao.combustivelExterno := structRecebido.combustivelExterno ? structRecebido.combustivelExterno : structPropagacao.combustivelExterno;
	structPropagacao.isMarcadorCoberto := structRecebido.isMarcadorCoberto ? structRecebido.isMarcadorCoberto : structPropagacao.isMarcadorCoberto;
	structPropagacao.lerEstadoAtual := structRecebido.lerEstadoAtual ? structRecebido.lerEstadoAtual : structPropagacao.lerEstadoAtual;
	structPropagacao.geracaoMaxima := structRecebido.geracaoMaxima ? structRecebido.geracaoMaxima : structPropagacao.geracaoMaxima;

	return structPropagacao;
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

function NovaPropPropagacao()

	var prop := struct{
				geracao := 0,
				origem := 0,
				marcadorOrigem := -1,
				tipoPropagacao := 0, 
				estadoInicial := ESTADO_PROPAGACAO::DORMENTE,
				marcadorOrigemPid := 0,
				esteMarcadorPid := 0,
				marcadorProps := struct,
				efeitosPersonalizados := dictionary,
				combustivelExterno := 0,
				isMarcadorCoberto := 0,
				lerEstadoAtual := ESTADO_PROPAGACAO::DORMENTE,
				geracaoMaxima := 0
			};
	return prop;
endfunction

function MarcaLimitadoraGlobal(origem, valor)
	if(origem)
		origem := CStr(origem);
		
		Set_Critical(1);
		var global := GetGlobalProperty(PROPNAME_LIMITADORA_GLOBAL);

		if(!global)
			global := dictionary;
		endif

		var valorAtual := global[origem];
		var valorResultante := cint(valorAtual) + valor;

		if(global.exists(origem))

			if(valorResultante <= 0)
				global.erase(origem);
			else
				global[origem] := valorAtual + valor;
		endif	
		else
			global.insert(origem, valor);
		endif

		SetGlobalProperty(PROPNAME_LIMITADORA_GLOBAL, global);
		Set_Critical(0);
		return 1;
	else
		Print("[Erro - Propagação] Tentativa de marcar limitadora global sem marcador origem.");
		return 0;
	endif
endfunction

function LerLimitadoraGlobal(origem)
	Set_Critical(1);
	var global := GetGlobalProperty(PROPNAME_LIMITADORA_GLOBAL);
	if(global && origem)
		origem := CStr(origem);
		return global[origem] ? global[origem] : 0;
	endif
	Set_Critical(0);
	return 0;
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
										opcoesPersonalizadas
										)
	if(dictEfeito)
		if(!opcoesPersonalizadas)
			return error{ error_text := "As opções são obrigatórias, veja função 'NovoOpcoesEfeitoPersonalizado'"};
		endif
		dictEfeito.insert(cstr(estado), opcoesPersonalizadas);
		return 1;
	else
		return error{ error_text := "É necessário um dicionario como referência"}; 
	endif

endfunction

function NovoOpcoesEfeitoPersonalizado(graphic)
	return struct {
					graphic := graphic, 
					cor := 0, 
					isInvisibel := 1, 
					sfx := 0,
					efx := -1, 
					duracao := 10, 
					effectItemObjtype := 0, 
					chanceRandom := 2,
					minRandom := 1,
					maxRandom := 8
				};

endfunction

function NovoStructCombustivelMagico(combustivel,fator)
	return struct {combustivel := combustivel, fator := fator};
endfunction

function PegaProcessoMarcador(marcadorPropagacao)
	var propPropagacao := GetObjProperty(marcadorPropagacao, PROPNAME_PROP_PROPAGACAO);
    return GetProcess(cint(propPropagacao.esteMarcadorPid)); 
endfunction