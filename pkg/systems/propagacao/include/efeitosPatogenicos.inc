include ":propagacao:include/propagacao";

function CriaMarcadorPatogenico(x,y,z,realm,distancia := 1, ciclosExpancao, duracaoCiclos)
	var marcador := CriarMarcadorPropagacao(
											x,
											y,
											z,
											realm, 
											struct {
												tipoPropagacao := PROPAGACAO_TIPO::PATOGENICO,
												estadoInicial := ESTADO_PROPAGACAO::INTENSO,
												geracaoMaxima := distancia,
												marcadorProps := struct { ciclosExpancao := ciclosExpancao, duracaoCiclos := duracaoCiclos}
											}	
											);
	marcador.Abastecer(true);
endfunction