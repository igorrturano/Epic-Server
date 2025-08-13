use uo;

include ":propagacao:include/marcadorPropagacaoIncendio";
include "/include/epicPropNames";

class MarcadorPropagacaoFabrica()
  function MarcadorPropagacaoFabrica(this)
  endfunction

  function Montar( this, parametros)
	//Print("*PROPAGACAO* Iniciando Fabrica.");
	if(parametros.marcador)
		var propPropagacao := GetObjProperty(parametros.marcador, PROPNAME_PROP_PROPAGACAO) ;
		//Print("*PROPAGACAO* propPropagacao --- " + propPropagacao);
		if(propPropagacao)
			case (propPropagacao.tipoPropagacao)
				PROPAGACAO_TIPO_INCENDIO:
					//Print("*PROPAGACAO* Iniciando Marcador Tipo Incêndio Instânciado.");
					return MarcadorPropagacaoIncendio(parametros.marcador, propPropagacao);
			endcase
		endif
			return error{errortext := "O marcador informado não foi configurado."};
	else
		return error{errortext := "O marcador de propagação é obrigatório."};
	endif
  endfunction
endclass

class FabricaPropagacaoParametros()
	function FabricaPropagacaoParametros(this)
		this.marcador := 0;
  	endfunction
endclass