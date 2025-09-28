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

class MarcadorPropagacaoPatogenico( MarcadorPropagacaoBase )

  function MarcadorPropagacaoPatogenico( this, esteItemMarcador, propPropagacao, momentoNascimento )
    super( esteItemMarcador, propPropagacao, momentoNascimento );

    // esteItemMarcador.usescript := ":propagacao:dblClickMarcadorIncendio";

    var propagacaoConfig := ReadConfigFile( ":propagacao:config/propagacao" );
    this.mobileOrigem := SystemFindObjectBySerial( cint( propPropagacao.origem ),
                                                           SYSFIND_SEARCH_OFFLINE_MOBILES );
    //flags de controle;
    this.isAcumulando := true;
    this.isVisaoBloqueada := false;
    this.isEstimuloEnviado := false;
    this.isEstimuloParadaEnviado := false;
    this.deveProgredir := false;

    //Parametros
    this.ciclosRetorno := 4;
    this.ciclosExpancao := 2;
    this.duracaoCiclos := 10;

    //variaveis
    this.ciclosDormenteLimite := 15;
    this.ciclosDormente := 0;
    this.ciclos := 0;
    this.ciclosRetornoMarcador := 0;
    this.tileProspectado := this.Prospectar();
    this.multiplicadorCicloMax := CDbl(propagacaoConfig["patogenica"].multiplicadorCiclosMaximo);
    this.graficos := dictionary{ ESTADO_PROPAGACAO::DORMENTE -> 0x3794,
                                 ESTADO_PROPAGACAO::INTENSO -> 0x113B };
  endfunction

  function EfetuarCiclo( this )
    if ( !this.IsEstadoControle( CONTROLE_PROPAGACAO::MORTO ) )
      this.ciclos++;
      Print( this.ciclos + " - " + this.PegaPropPropagacao().geracaoMaxima + " - " + this.isAcumulando + " geracao: " + this.geracao
           + " geracaoMaxima " + this.PegaPropPropagacao().geracaoMaxima + " serial: " + this.esteItemMarcador.serial );
      this._ProcessaTile();
      this._ProcessaEstado();
      this._ProcessaEnvioEstimulo();

      this._AvaliaDormencia();

      if ( this._DevePropagar() )
        this.Propagar();
      endif

    endif
  endfunction

  function _ProcessaEnvioEstimulo(this)

    if(this._DeveEnviarEstimuloProgresso())
      this.isEstimuloEnviado := true;
      this._EnviaEstimulo( 1 );
    else
      if ( this._DeveMandarEstimuloParada() )
        this.isEstimuloParadaEnviado := true;
        this._EnviaEstimulo( -1 );
      endif
    endif
  endfunction

  function _ProcessaTile( this )
    this._ProcessaObjetosEncontrados();
  endfunction

  function _AvaliaDormencia( this )
    if ( this.estado == ESTADO_PROPAGACAO::DORMENTE )
      this.ciclosDormente++;
    endif

    if ( this.ciclosDormente >= cint( this.ciclosDormenteLimite ) &&
         this.estado == ESTADO_PROPAGACAO::DORMENTE )
      this.estadoControle := CONTROLE_PROPAGACAO::MORTO;
    endif
  endfunction

  function _DeveMandarEstimuloParada( this )
    if(this.geracao == 0)
      Print("(this.ciclosRetorno + this.ciclosRetornoMarcador) " + (this.ciclosRetorno + this.ciclosRetornoMarcador) + " ciclos " + this.ciclos);
    endif
  
    return !this.isAcumulando  
            && this.ciclos >= (this.ciclosRetorno + this.ciclosRetornoMarcador)
            && !this.isVisaoBloqueada
            && !this.isEstimuloParadaEnviado;
  endfunction

  function _DeveEnviarEstimuloProgresso( this )
    return this.ciclos >= this.ciclosExpancao && !this.isEstimuloEnviado;//Duracao
  endfunction

  function _ProcessaEstado( this )
    if ( this.ciclos >= this.duracaoCiclos )
      if ( this._IsUltimaGeracao() )
        this._AdormecerMarcador();
      elseif(this.ciclos >= this._CalculaCicloMaximo())
        this._AdormecerMarcador();
      endif
    endif

    if ( this._DeveFicarIntenso() )
      this._AtualizaEstado( ESTADO_PROPAGACAO::INTENSO );
    else
      if ( this._DeveFicarDormente() )
        this._AtualizaEstado( ESTADO_PROPAGACAO::DORMENTE );
      endif
    endif
  endfunction

  function _CalculaCicloMaximo(this)
    return cint(this.duracaoCiclos * this.multiplicadorCicloMax * this.PegaPropPropagacao().geracaoMaxima * this.ciclosExpancao);
  endfunction

  function _AdormecerMarcador(this)
    this._AtualizaEstado( ESTADO_PROPAGACAO::DORMENTE );
    this.isAcumulando := false;
  endfunction

  function _DeveFicarIntenso( this )
    return this.estado < ESTADO_PROPAGACAO::INTENSO 
        && this.isAcumulando 
        && !this.isVisaoBloqueada 
        && this.deveProgredir;
  endfunction

  function _DeveFicarDormente( this )
    return this.esteItemMarcador.graphic != this.graficos[ESTADO_PROPAGACAO::DORMENTE] 
            && !this.isAcumulando 
            || this.isVisaoBloqueada;
  endfunction

  function _AtualizaEstado( this, estado )
    this.estado := estado;
    var propPropagacao := this.PegaPropPropagacao();
    propPropagacao.lerEstadoAtual := this.estado;
    SetObjProperty( PROPNAME_PROP_PROPAGACAO, this.esteItemMarcador, propPropagacao );

    case ( this.estado )
      ESTADO_PROPAGACAO::INTENSO:
        this.AtribuiEstadoControle(CONTROLE_PROPAGACAO::ATIVO); 
        this.esteItemMarcador.invisible := 0;
        break;
      ESTADO_PROPAGACAO::DORMENTE: 
        this.AtribuiEstadoControle(CONTROLE_PROPAGACAO::DORMENTE);
        this.esteItemMarcador.invisible := 1;
        break;
      default: this.esteItemMarcador.invisible := 1;
    endcase
    
    this._AtualizaGrafico();
  endfunction

  function _AtualizaGrafico( this )
    var graficoConfigurado := this.graficos[this.estado];
    if ( graficoConfigurado != this.esteItemMarcador.graphic )
      if ( graficoConfigurado )
        this.esteItemMarcador.graphic := graficoConfigurado;
      else
        Print( "** Erro ao resolver graphic para marcador {} do tipo {}".format( this.esteItemMarcador.serial,
                                                                                 this.PegaPropPropagacao().tipoPropagacao ) );
      endif
    endif
  endfunction

  function _IsUltimaGeracao( this )
    return this.PegaPropPropagacao().geracaoMaxima <= this.geracao;
  endfunction

  function _EnviaEstimulo( this, valor )
    foreach adjacente in ( this.marcadoresAdjacentes )
      Sleepms( RandomIntMinMax( 20, 60 ) );
      var propAdjacente := GetObjProperty( adjacente, PROPNAME_PROP_PROPAGACAO );
      var pidAdjacente := GetProcess( propAdjacente.esteMarcadorPid );

      if ( pidAdjacente )
        pidAdjacente.SendEvent( struct{ id := EV_PROPAGACAO::ESTIMULO, valor := valor } );
      endif

    endforeach
  endfunction

  function Iniciar( this )
    this._AtualizaEstado(this.estado);
    this.MarcarAdjacentes();
    if ( !this.tileProspectado )
      this.tileProspectado := this.Prospectar();
    endif
  endfunction

  function InjetaCombustivel( this, deveResetarEstado )
    if ( deveResetarEstado )
      this.AtribuiEstado( this.PegaPropPropagacao().estadoInicial );
      this.isAcumulando := true;
    endif
    this.mobileOrigem := SystemFindObjectBySerial( cint( this.PegaPropPropagacao().origem ),
                                                           SYSFIND_SEARCH_OFFLINE_MOBILES );
  endfunction

  function RecebeEstimulo( this, valor, isRemetenteAcumulando := false )
    isRemetenteAcumulando == false;
    this.isAcumulando := valor > 0;
    this.deveProgredir := this.isAcumulando;
    if(!this.deveProgredir && this.ciclosRetornoMarcador == 0)
      this.ciclosRetornoMarcador := this.ciclos;
    endif
    this._ProcessaEstado();
    Print( "Valor recebido ! " + valor + " - " + this.isAcumulando  + " geracao: " + this.geracao);
  endfunction

  function _ProcessaObjetosEncontrados( this )
    this.isVisaoBloqueada := false;
    this.tileProspectado := this.ProspectarPersonalizado();

    foreach item in ( this.tileProspectado.PegaItensEncontrados() )
      Sleepms( 2 );
      if(item.serial == this.esteItemMarcador.serial) continue; endif
      var tilesInfo := BuscaConfiguracaoTiles( item );
      if ( !this.isVisaoBloqueada )
        this.isVisaoBloqueada := this._ChecaVisaoBloqueada( tilesInfo );
      endif
    endforeach

    foreach itemEstatico in ( this.tileProspectado.PegaEstaticosEncontrados() )
      Sleepms( 2 );
      var tilesInfo := BuscaConfiguracaoTiles( itemEstatico );
      if ( !this.isVisaoBloqueada )
        this.isVisaoBloqueada := this._ChecaVisaoBloqueada( tilesInfo );
      endif
    endforeach

  endfunction

  function _ChecaVisaoBloqueada( this, tilesInfo )
    return tilesInfo.BlockSight && tilesInfo.Blocking;
  endfunction

  function _DevePropagar( this )
    return this.estado > ESTADO_PROPAGACAO::DORMENTE && !this.IsEstadoControle( CONTROLE_PROPAGACAO::MORTO ) &&
                                                        this.geracao < this.PegaPropPropagacao().geracaoMaxima;
  endfunction

  function Propagar( this )
    return this.MarcarAdjacentes();
  endfunction

  function ProspectarPersonalizado( this )
    return this.Prospectar( array{ TILE_SCANNER::ITENS, TILE_SCANNER::STATICOS,
                                   TILE_SCANNER::MOBILES } );
  endfunction

endclass
