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
include ":nature:include/nature";

class PropagacaoEstiloGas( MarcadorPropagacaoBase )

  function PropagacaoEstiloGas( this, esteItemMarcador, propPropagacao, momentoNascimento, multiplicadorCicloMax, graficos, descansoMinimo )
    super( esteItemMarcador, propPropagacao, momentoNascimento, descansoMinimo );

    // esteItemMarcador.usescript := ":propagacao:dblClickMarcadorIncendio";

    this.mobileOrigem := SystemFindObjectBySerial( cint( propPropagacao.origem ),
                                                           SYSFIND_SEARCH_OFFLINE_MOBILES );
    this.InicializaVariaveis(multiplicadorCicloMax, graficos);
  endfunction

  function InicializaVariaveis(this,multiplicadorCicloMax,graficos)
    //flags de controle;
    this.isAcumulando := true;
    this.isVisaoBloqueada := false;
    this.isEstimuloEnviado := false;
    this.isEstimuloParadaEnviado := false;
    this.deveProgredir := false;

    //Parametros
    this.ciclosExpancao := cInt(this.PegaPropPropagacao().marcadorProps.ciclosExpancao);
    this.duracaoCiclos := cint(this.PegaPropPropagacao().marcadorProps.duracaoCiclos);

    //variaveis
    this.ciclosRetorno := 1;
    this.ciclosDormenteLimite := 2;
    this.ciclosDormente := 0;
    this.ciclos := 0;
    this.ciclosRetornoMarcador := 0;
    this.tileProspectado := this.Prospectar();
    this.multiplicadorCicloMax := CDbl(multiplicadorCicloMax);
    this.graficos := graficos;
  endfunction

  uninit function AplicaEfeitoMarcador(this, mobileEncontrado);

  function EfetuarCiclo( this )
    if ( !this.IsEstadoControle( CONTROLE_PROPAGACAO::MORTO ) )
      this.ciclos++;

      if(!this._IsParametrosValidos())
        this._TratarParametrosInvalidos();
      endif

      this._ProcessaTile();
      this._ProcessaEstado();
      this._ProcessaEnvioEstimulo();

      this._AvaliaDormencia();

      if ( this._DevePropagar() )
        this.Propagar();
      endif

    endif
  endfunction

  function EnviaEstimulo( this, valor )
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
    this.AtualizaEstado(this.estado, true);
    this.MarcarAdjacentes();
    if ( !this.tileProspectado )
      this.tileProspectado := this.Prospectar();
    endif
  endfunction

  function InjetaCombustivel( this, deveResetarEstado )
    if ( deveResetarEstado )
      this.AtualizaEstado( this.PegaPropPropagacao().estadoInicial );
      this.InicializaVariaveis(this.multiplicadorCicloMax, this.graficos);
    endif
    this.mobileOrigem := SystemFindObjectBySerial( cint( this.PegaPropPropagacao().origem ),
                                                           SYSFIND_SEARCH_OFFLINE_MOBILES );
  endfunction

  function RecebeEstimulo( this, valor, isRemetenteAcumulando := false )
    isRemetenteAcumulando == false;
    this.isAcumulando := valor > 0 && !this.isEstimuloParadaEnviado;
    this.deveProgredir := this.isAcumulando;
    if(!this.deveProgredir && this.ciclosRetornoMarcador == 0)
      this.ciclosRetornoMarcador := this.ciclos;
    endif
    this._ProcessaEstado();
  endfunction

  function GanchoAntesAtualizaEstado( this, estado )
    return estado;
  endfunction

  function AtualizaComportamentoMarcador(this)
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

  function ProspectarPersonalizado( this )
    return this.Prospectar( array{ TILE_SCANNER::ITENS, TILE_SCANNER::STATICOS,
                                   TILE_SCANNER::MOBILES } );
  endfunction

  function _TratarParametrosInvalidos(this)
    if(this.mobileOrigem)
      SendSysMessageEX(this.mobileOrigem,"Erro ao criar Propagação de Gás, por favor comunique a staff.");
    endif
    
    this._AdormecerMarcador();
    this.AtribuiEstadoControle(CONTROLE_PROPAGACAO::MORTO);
  endfunction

  function _IsParametrosValidos(this)
    return !(TypeOfInt(this.ciclosExpancao) != OT_LONG 
              || TypeOfInt(this.duracaoCiclos) != OT_LONG
              || this.duracaoCiclos == 0
              || this.ciclosExpancao == 0
              );
  endfunction

  function _ProcessaEnvioEstimulo(this)

    if(this._DeveEnviarEstimuloProgresso())
      this.isEstimuloEnviado := true;
      this.EnviaEstimulo( 1 );
    else
      if ( this._DeveMandarEstimuloParada() )
        this.isEstimuloParadaEnviado := true;
        this.EnviaEstimulo( -1 );
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

      if ( this._IsUltimaGeracao())
        this._AdormecerMarcador();
        return 1;
      elseif(this.ciclos >= this._CalculaCicloMaximo())
        this._AdormecerMarcador();
        return 1;
      endif
      
    endif

    if ( this._DeveFicarIntenso() )
      this.AtualizaEstado( ESTADO_PROPAGACAO::INTENSO );
    else
      if ( this._DeveFicarDormente() )
        this.AtualizaEstado( ESTADO_PROPAGACAO::DORMENTE );
      endif
    endif
  endfunction

  function _CalculaCicloMaximo(this)
    return cint(this.duracaoCiclos * this.multiplicadorCicloMax * this.PegaPropPropagacao().geracaoMaxima * this.ciclosExpancao);
  endfunction

  function _AdormecerMarcador(this)
    this.AtualizaEstado( ESTADO_PROPAGACAO::DORMENTE );
    this.isAcumulando := false;
  endfunction

  function _DeveFicarIntenso( this )
    return this.estado < ESTADO_PROPAGACAO::INTENSO 
        && this.isAcumulando 
        && !this.isVisaoBloqueada 
        && this.deveProgredir;
  endfunction

  function _DeveFicarDormente( this )
    
    return (this.esteItemMarcador.graphic != this.graficos[ESTADO_PROPAGACAO::DORMENTE] 
            && !this.isAcumulando
            && this._IsProximaGeracaoDormente()) 
            || this.isVisaoBloqueada;
  endfunction

  function _IsProximaGeracaoDormente(this)
    var totalEncontrado := 0;
    var totalDormente := 0;

    if( this._IsUltimaGeracao())
      return true;
    endif

    foreach marcador in (this.marcadoresAdjacentes)
      Sleepms(2);
      if(marcador.PegaGeracao() == this.geracao + 1)
        totalEncontrado++;
        if(marcador.LerEstado() == ESTADO_PROPAGACAO::DORMENTE)
          totalDormente++;
        endif
      endif
    endforeach

    return totalEncontrado == totalDormente;
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
    return this.PegaPropPropagacao().geracaoMaxima <= this.geracao || this.IsLimiteGeracaoAlcancado(this.geracao);
  endfunction

  function _ProcessaObjetosEncontrados( this )
    this.isVisaoBloqueada := false;
    this.tileProspectado := this.ProspectarPersonalizado();

    foreach item in ( this.tileProspectado.PegaItensEncontrados() )
      Sleepms( 2 );
      if(item.serial == this.esteItemMarcador.serial) continue; endif
      var tilesInfo := BuscaConfiguracaoTiles( item );
      this._VerificaVisaoBloqueada( tilesInfo );
      
    endforeach

    foreach itemEstatico in ( this.tileProspectado.PegaEstaticosEncontrados() )
      Sleepms( 2 );
      var tilesInfo := BuscaConfiguracaoTiles( itemEstatico );
      this._VerificaVisaoBloqueada( tilesInfo );
    endforeach

    foreach mobileEncontrado in ( this.tileProspectado.PegaMobilesEncontrados() )
      Sleepms( 2 );
      if(this.esteItemMarcador.LerEstado() == ESTADO_PROPAGACAO::INTENSO)
        this.AplicaEfeitoMarcador(mobileEncontrado);
      endif
    endforeach

  endfunction

  function _VerificaVisaoBloqueada( this, tilesInfo )
    if ( !this.isVisaoBloqueada )
      this.isVisaoBloqueada := tilesInfo.BlockSight && tilesInfo.Blocking;
    endif
  endfunction

  function _DevePropagar( this )
    return this.estado > ESTADO_PROPAGACAO::DORMENTE 
           && !this.IsEstadoControle( CONTROLE_PROPAGACAO::MORTO ) 
           && this.geracao < this.PegaPropPropagacao().geracaoMaxima
           && this.isAcumulando;
  endfunction

endclass
