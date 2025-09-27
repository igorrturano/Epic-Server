use cfgfile;
use uo;
use util;

include ":propagacao:include/marcadores/marcadorPropagacaoBase";
include ":propagacao:include/propagacao";
include "/include/epicPropNames";
include "/include/epicConstants";
include "include/client";
include "/include/math";
include ":destroyable:include/destroyItem";
include ":combat:damage";
include ":nature:include/nature";

class MarcadorPropagacaoPatogenico( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoPatogenico(this, esteItemMarcador, propPropagacao, momentoNascimento)
    super(esteItemMarcador,propPropagacao, momentoNascimento); 

    //esteItemMarcador.usescript := ":propagacao:dblClickMarcadorIncendio";

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(propPropagacao.origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
    this.estimuloLimite := 25;
    this.ciclosDormenteLimite := 15;
    this.ciclosDormente := 0;
    this.duracaoCiclos := 10;
    this.ciclos := 0;
    this.ciclosRetorno := 2;
    this.tileProspectado := this.ProspectarPersonalizado();
    this.isAcumulando := true;
    this.graficos := dictionary {ESTADO_PROPAGACAO::DORMENTE -> 0x3794, ESTADO_PROPAGACAO::INTENSO -> 0x113B};
  endfunction

  function EfetuarCiclo(this)
    if(!this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO))
      this.ciclos++;
      Print(this.ciclos  + " - " +  this.PegaPropPropagacao().geracaoMaxima + " - " + this.isAcumulando + " geracao: " + this.geracao + " geracaoMaxima " + this.PegaPropPropagacao().geracaoMaxima);

      this._ProcessaEstado();

      if(this._DeveMandarEstimuloParada())
        this._EnviaEstimulo(-1);
      endif

      this._AvaliaDormencia();

      if(this._DevePropagar())
        this.Propagar();
      endif

    endif
  endfunction

  function _AvaliaDormencia(this)
    if(this.estado == ESTADO_PROPAGACAO::DORMENTE )
      this.ciclosDormente++;
    endif

    if(this.ciclosDormente >= cint(this.ciclosDormenteLimite) && this.estado == ESTADO_PROPAGACAO::DORMENTE)
      this.estadoControle := CONTROLE_PROPAGACAO::MORTO;
    endif
  endfunction

  function _DeveMandarEstimuloParada(this)
    return !this.isAcumulando && this.ciclosDormente % this.ciclosRetorno;
  endfunction

  function _ProcessaEstado(this)
    if(this.ciclos >= this.duracaoCiclos)
      if(this._IsUltimaGeracao())
        this._AtualizaEstado(ESTADO_PROPAGACAO::DORMENTE);
        this.isAcumulando := false;
      endif
    endif

    if(this._DeveFicarIntenso())
      this._AtualizaEstado(ESTADO_PROPAGACAO::INTENSO);
    else
      if(this._DeveFicarDormente())
        this._AtualizaEstado(ESTADO_PROPAGACAO::DORMENTE);
      endif
    endif
  endfunction

  function _DeveFicarIntenso(this)
    return this.estado < ESTADO_PROPAGACAO::INTENSO && this.ciclos >= 3 && this.isAcumulando;
  endfunction

  function _DeveFicarDormente(this)
    return this.esteItemMarcador.graphic != this.graficos[ESTADO_PROPAGACAO::DORMENTE] && !this.isAcumulando;
  endfunction

  function _AtualizaEstado(this, estado)
    this.estado := estado;
    var propPropagacao := this.PegaPropPropagacao();
    propPropagacao.lerEstadoAtual := this.estado;
    SetObjProperty(PROPNAME_PROP_PROPAGACAO,this.esteItemMarcador, propPropagacao);
    case(this.estado)
      ESTADO_PROPAGACAO::INTENSO:
        this.esteItemMarcador.invisible := 1;
        break;
      default:
        this.esteItemMarcador.invisible := 0;
    endcase
    this._AtualizaGrafico();
  endfunction

  function _AtualizaGrafico(this)
    var graficoConfigurado :=  this.graficos[this.estado];
    if(graficoConfigurado != this.esteItemMarcador.graphic)
      if(graficoConfigurado)
        this.esteItemMarcador.graphic := graficoConfigurado;
      else
        Print("** Erro ao resolver graphic para marcador {} do tipo {}".format(this.esteItemMarcador.serial , this.PegaPropPropagacao().tipoPropagacao));
      endif
    endif
  endfunction

  function _IsUltimaGeracao(this)
    return this.PegaPropPropagacao().geracaoMaxima <= this.geracao;
  endfunction

  function _EnviaEstimulo(this, valor)
    foreach adjacente in (this.marcadoresAdjacentes)
      Sleepms(RandomIntMinMax(20, 80));
      var propAdjacente := GetObjProperty(adjacente, PROPNAME_PROP_PROPAGACAO);
      var pidAdjacente := GetProcess(propAdjacente.esteMarcadorPid);

      if(pidAdjacente)
        pidAdjacente.SendEvent(
                      struct{
                        id := EV_PROPAGACAO::ESTIMULO, 
                        valor := valor
                      });
      endif
      
    endforeach
  endfunction

  function Iniciar(this)
    if( this.estado > ESTADO_PROPAGACAO::DORMENTE )
      this.esteItemMarcador.graphic := 0x113B;
    endif
    this.MarcarAdjacentes();
    if(!this.tileProspectado)
      this.tileProspectado := this.ProspectarPersonalizado();
    endif
  endfunction

  function InjetaCombustivel(this, deveResetarEstado)
    if(deveResetarEstado)
      this.AtribuiEstado(this.PegaPropPropagacao().estadoInicial);
    endif
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(this.PegaPropPropagacao().origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
  endfunction


  function RecebeEstimulo(this, valor ,isRemetenteAcumulando := false)
    isRemetenteAcumulando == false;
    this.isAcumulando := valor > 0 && !this._isVisaoTileBloqueada();
    Print("Valor recebido ! " + valor + " - " + this.isAcumulando);
  endfunction

  function _isVisaoTileBloqueada(this)

    foreach item in (this.tileProspectado.PegaItensEncontrados())
		Sleepms(2);
		var tilesInfo := BuscaConfiguracaoTiles(item);
		if(tilesInfo.BlockSight)
			return true;
		endif
	endforeach

	foreach itemEstatico in (this.tileProspectado.PegaEstaticosEncontrados())
		Sleepms(2);
		var tilesInfo := BuscaConfiguracaoTiles(itemEstatico);
		if(tilesInfo.BlockSight)
			return true;
		endif
	endforeach

	return false;
  endfunction

  function _DevePropagar(this)
    return this.estado > ESTADO_PROPAGACAO::DORMENTE
        && !this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO)
        && this.geracao < this.PegaPropPropagacao().geracaoMaxima;
  endfunction

  function Propagar(this)
    return this.MarcarAdjacentes();
  endfunction

  function ProspectarPersonalizado(this)
    return this.Prospectar(array {TILE_SCANNER::ITENS, TILE_SCANNER::STATICOS, TILE_SCANNER::MOBILES});
  endfunction

endclass
