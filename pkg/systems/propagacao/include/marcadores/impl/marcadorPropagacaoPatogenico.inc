include ":propagacao:include/marcadores/propagacaoEstiloGas";

class MarcadorPropagacaoPatogenico( PropagacaoEstiloGas )
	function MarcadorPropagacaoPatogenico(this, esteItemMarcador, propPropagacao, momentoNascimento)
		var propagacaoConfig := ReadConfigFile( ":propagacao:config/propagacao" );

		super(
			esteItemMarcador,
			propPropagacao,
			momentoNascimento,
			propagacaoConfig["patogenica"].multiplicadorCiclosMaximo,
			dictionary{ ESTADO_PROPAGACAO::DORMENTE -> 0x3794, ESTADO_PROPAGACAO::INTENSO -> 0x113B }
		); 
	endfunction
	

	function AplicaEfeitoMarcador(this, mobileEncontrado)
		
	endfunction
endclass