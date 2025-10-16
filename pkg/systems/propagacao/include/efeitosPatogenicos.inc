include ":propagacao:include/propagacao";

function CriaMarcadorPatogenico(
									x,
									y,
									z,
									realm,
									distancia := 1,
									ciclosAteExpancao, 
									duracaoNuvemEmCiclos, 
									gravidade, 
									arraySintomas,
									duracaoPatogenico,
									nivelInfeccao
								)

	var marcador := CriarMarcadorPropagacao(
											x,
											y,
											z,
											realm, 
											struct {
												tipoPropagacao := PROPAGACAO_TIPO::PATOGENICO,
												estadoInicial := ESTADO_PROPAGACAO::INTENSO,
												geracaoMaxima := distancia,
												marcadorProps := struct { 
																	ciclosExpancao := ciclosAteExpancao,
																	duracaoCiclos := duracaoNuvemEmCiclos,
																	duracaoPatogenico := duracaoPatogenico,
																	arraySintomas := arraySintomas,
																	gravidadeInfeccao := gravidade,
																	nivelInfeccao := nivelInfeccao
																}
											}	
											);
	marcador.Abastecer(true);
endfunction