include ":propagacao:include/propagacao";

function CriaMarcadorPatogenico(
									x,
									y,
									z,
									realm,
									serialConjurador,
									alcance := 1,
									ciclosAteExpancao, 
									duracaoNuvemEmCiclos, 
									gravidadeInfeccao, 
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
												origem := serialConjurador,
												marcadorProps := struct { 
																	ciclosExpancao := ciclosAteExpancao,
																	duracaoCiclos := duracaoNuvemEmCiclos,
																	duracaoPatogenico := duracaoPatogenico,
																	arraySintomas := arraySintomas,
																	gravidadeInfeccao := gravidadeInfeccao,
																	nivelInfeccao := nivelInfeccao,
																	devePropagarProps := true
																}
											}	
											);
	//marcador.Abastecer(true);
endfunction