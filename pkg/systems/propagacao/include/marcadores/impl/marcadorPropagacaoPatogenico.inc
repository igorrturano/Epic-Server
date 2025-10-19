include ":propagacao:include/marcadores/propagacaoEstiloGas";
include ":propagacao:include/afetaMobilePropagacao";
include ":medicine:disease";

class MarcadorPropagacaoPatogenico( PropagacaoEstiloGas )
	function MarcadorPropagacaoPatogenico(this, esteItemMarcador, propPropagacao, momentoNascimento)
		var propagacaoConfig := ReadConfigFile( ":propagacao:config/propagacao" );

		super(
			esteItemMarcador,
			propPropagacao,
			momentoNascimento,
			propagacaoConfig["patogenica"].multiplicadorCiclosMaximo,
			dictionary{ ESTADO_PROPAGACAO::DORMENTE -> 0x3794, ESTADO_PROPAGACAO::INTENSO -> 0x113B },
			cint(propagacaoConfig["patogenica"].descasoMinimo)
		);
		
		this.resfriamento := cint(propagacaoConfig["patogenica"].resfriamento);
		this.duracaoPatogenico := this.PegaPropPropagacao().marcadorProps.duracaoPatogenico;
		this.nivelInfeccao := this.PegaPropPropagacao().marcadorProps.nivelInfeccao;
		this.gravidadeInfeccao := this.PegaPropPropagacao().marcadorProps.gravidadeInfeccao;
		this.arraySintomas := this.PegaPropPropagacao().marcadorProps.arraySintomas;

	endfunction
	

	function AplicaEfeitoMarcador(this, mobileEncontrado)
		
		AfetaMobileInfectado(mobileEncontrado,this.PegaPropPropagacao(), this.resfriamento);
	endfunction
endclass