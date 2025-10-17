include ":propagacao:include/propagacao";

function CriaMarcadorPatogenico(
									x,
									y,
									z,
									realm,
									alcance := 1,
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
												geracaoMaxima := alcance,
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