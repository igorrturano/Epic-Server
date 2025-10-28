use uo;

include ":propagacao:include/marcadores/impl/marcadorPropagacaoIncendio";
include ":propagacao:include/marcadores/impl/marcadorPropagacaoPatogenico";
include "/include/epicPropNames";

class MarcadorPropagacaoFabrica()
  function MarcadorPropagacaoFabrica(this)
  endfunction

  function Montar( this, parametros)
	if(parametros.marcador)
		var propPropagacao := GetObjProperty(parametros.marcador, PROPNAME_PROP_PROPAGACAO);
		if(propPropagacao)
			case (propPropagacao.tipoPropagacao)
				PROPAGACAO_TIPO::INCENDIO:
					return MarcadorPropagacaoIncendio(parametros.marcador, propPropagacao, parametros.momentoNascimento);
				PROPAGACAO_TIPO::PATOGENICO:
					return MarcadorPropagacaoPatogenico(parametros.marcador, propPropagacao, parametros.momentoNascimento);
				default:
					break;
			endcase
		endif
		return error{errortext := "* ERRO * O marcador informado não foi configurado."};
	else
		return error{errortext := "* ERRO * O marcador de propagação é obrigatório."};
	endif
  endfunction
endclass

class FabricaPropagacaoParametros()
	function FabricaPropagacaoParametros(this)
		this.marcador := 0;
		this.momentoNascimento := 0;
  	endfunction
endclass