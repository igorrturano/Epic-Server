use cfgfile;

include ":propagacao:include/marcadorPropagacaoBase";
include ":propagacao:include/propagacao";
include "/include/epicPropNames";
include "/include/math";

class MarcadorPropagacaoIncendio( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoIncendio(this, esteItemMarcador, propPropagacao)
    super(esteItemMarcador,propPropagacao);

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.resourceConfig := ReadConfigFile(":crafting:config/resourceList");
    this.isSoloConsumido := false;
    this.isEstaticoConsumido := false;
    this.combustiveis := dictionary{};
    this.estimuloLimite := propagacaoConfig["incendio"].estimuloLimite;
    this.ciclosDormenteLimite := propagacaoConfig["incendio"].ciclosDormenteLimite;
    this.listaStaticosInflamaveis :=  GetConfigStringArray(propagacaoConfig["incendio"],"staticInflamavel");
    this.limiteEstimuloEstadoSuperior := this._CalculaLimiteEstimuloSuperior();
    this.limiteEstimuloEstadoInferior := this._CalculaLimiteEstimuloInferior();
  endfunction

  function EfetuarCiclo(this)
    if(!this.isMarcadorMorto)
     // Print("*PROPAGACAO* Iniciando Ciclo do marcador {} ".format(this.esteItemMarcador.serial));
      var tileProspectado := this.Prospectar();
      this.isAcumulando := false;

      this._IsAcumulando(tileProspectado);

      //TODO verificar statics
      //TODO verificar chao

      if(this.isAcumulando && this.estado > ESTADO_PROPAGACAO_LATENTE)
          //Print("*PROPAGACAO* Marcador {} Recebeu estimulo positivo.".format(this.esteItemMarcador.serial));
          this.RecebeEstimulo(this.estado);
      else
          //Print("*PROPAGACAO* Marcador {} Recebeu estimulo negativo.".format(this.esteItemMarcador.serial));
          this.RecebeEstimulo(-this.estado);
          this.IncrementaCiclosDesdeUltimoEstimulo();
      endif

      //Print("**** estado atual " + this.estado);
      if(this.DevePropagar())
         //Print("*PROPAGACAO* Marcador {} tentando propagar.".format(this.esteItemMarcador.serial));
        this.Propagar();
        this.EnviaEstimulo(this.estado);
      endif
    endif
  endfunction

  function _IsAcumulando(this, TileProspectado)
    foreach item in (tileProspectado.PegaItensEncontrados())
      Sleepms(2);
      var material :=  this.resourceConfig[GetObjProperty(item, PROPNAME_MATERIAL)];
      if(material.inflamavel)
        if(this.fatorMudancaEstado < material.inflamavel)
          this.fatorMudancaEstado := material.inflamavel;
        endif
        this.isAcumulando := true;
      endif
    endforeach

    if(!this.isAcumulando)
      var combustivelRestante := this.combustiveis["estaticos"];
      //Print("combustivelRestante  " + combustivelRestante);
      if(combustivelRestante > 0)
        combustivelRestante -= this.estado;
        this.combustiveis.insert("estaticos",combustivelRestante);
        this.isAcumulando := true;
      else
        if(combustivelRestante <= 0 && TypeOfInt(combustivelRestante) != OT_UNINIT)
          this.isEstaticoConsumido := true;
          this.isAcumulando := false;
        else
          if(!this.isEstaticoConsumido)
            foreach estatico in (tileProspectado.PegaEstaticosEncontrados())
              Sleepms(2);
             
              var isAoAlcance :=  this.esteItemMarcador.z + 5 >= estatico.z && this.esteItemMarcador.z - 5 <= estatico.z;
              Print("this.esteItemMarcador.serial - " + this.esteItemMarcador.serial + " z - " + this.esteItemMarcador.z + " --- " + estatico.objtype  + " z - " + estatico.z);
              if(isAoAlcance)
                if(cstr(estatico.objtype) in (this.listaStaticosInflamaveis))
                   Print("estatico " + estatico.objtype);
                  this.combustiveis.insert("estaticos",100);
                  this.isAcumulando := true;
                  break;
                endif
              endif
            endforeach
          endif
          this.isAcumulando := false;
        endif
      endif
    endif

  endfunction

  function RecebeEstimulo(this, valor, isExternoAcumulando := false)
    
    if(valor > 0)
        this.ZeraCiclosDesdeUltimoEstimuloPositivo();
    endif

    if(valor <= 0)
      if(this.ciclosDesdeUltimoEstimulo >= cint(this.ciclosDormenteLimite) && this.estado == ESTADO_PROPAGACAO_DORMENTE)
        this.isMarcadorMorto := true;
         //Print("*PROPAGACAO* Marcador {} Morreu após ficar muitos ciclos Dormente.".format(this.esteItemMarcador.serial));
      endif
    endif
    
    this.medidorEstimulo += (this._DeveInverterEstimulo(valor,isExternoAcumulando) ?  valor * -1 : valor);
    //Print("*PROPAGACAO* Marcador {} mudou seu medidor de estimulo para {}.".format(this.esteItemMarcador.serial, this.medidorEstimulo));
    this.AtualizaEstado(isExternoAcumulando);
  endfunction

  function _DeveInverterEstimulo(this, valorEstimulo, isExternoAcumulando)
    if(this.isAcumulando && valorEstimulo < 0)
      return true;
    elseif(isExternoAcumulando && this.isAcumulando && valorEstimulo < 0)
      return true;
    elseif(valorEstimulo <= 0)
      return false;
    elseif(isExternoAcumulando && !this.isAcumulando && this.estado > ESTADO_PROPAGACAO_LATENTE && valorEstimulo > 0)
      return true;
    elseif(!isExternoAcumulando && !this.isAcumulando && valorEstimulo > 0)
      return true;
    elseif(!isExternoAcumulando && this.isAcumulando && valorEstimulo > 0 && this.estado < ESTADO_PROPAGACAO_LATENTE)
      return true;
    else
      return false;
    endif
  endfunction

  function EnviaEstimulo(this, valor)
    //Print("*PROPAGACAO* Marcador {} Enviou estimulo.".format(this.esteItemMarcador.serial));
    foreach adjacente in (this.marcadoresAdjacentes)
      Sleepms(2);
      var propAdjacente := GetObjProperty(adjacente, PROPNAME_PROP_PROPAGACAO);
      var pidAdjacente := GetProcess(propAdjacente.esteMarcadorPid);
      //Print("*PROPAGACAO* Marcador {} Enviou {} estimulo para o processo {}.".format(this.esteItemMarcador.serial, valor ,pidAdjacente));

      if(pidAdjacente)
        pidAdjacente.SendEvent(struct{id := PROPAGACAO_EV_ESTIMULO, valor := valor, isAcumulando := this.isAcumulando});
      endif
      
    endforeach
  endfunction

  function AtualizaEstado(this, isExternoAcumulando);
    _CalculaLimiteEstimuloSuperior(this);
    _CalculaLimiteEstimuloInferior(this);

    if(this._isMedidorAcimaLimiteEstado())
      var valorAvanco := this._ObtemQuantidadeAvancoEstado();

      if(this._DeveAvancarEstado(valorAvanco))
        var novoValorEstado := this.estado + valorAvanco;
        var isMaximo := novoValorEstado > ESTADO_PROPAGACAO_INTENSO;
        this.estado := isMaximo ? ESTADO_PROPAGACAO_INTENSO : novoValorEstado;
        this.AtualizaComportamentoMarcador();
        //Print("*PROPAGACAO* Marcador {} Subiu de Estado e agora é {}.".format(this.esteItemMarcador.serial, this.estado));
        this.medidorEstimulo := isMaximo ? this.limiteEstimuloEstadoSuperior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoSuperior;
      endif
    elseif(this._isMedidorAbaixoLimiteEstado())
      var valorRegresso := this._ObtemQuantidadeRegressaoEstado();
      //Print("*PROPAGACAO* Marcador  estado agora é {}".format( this.estado));
      if(this._DeveRegredirEstado(valorRegresso))
        var novoValorEstado := this.estado - valorRegresso;
        var isMinimo := novoValorEstado < ESTADO_PROPAGACAO_DORMENTE;
        this.estado := isMinimo ? ESTADO_PROPAGACAO_DORMENTE : novoValorEstado;
        this.AtualizaComportamentoMarcador();

        this.medidorEstimulo := isMinimo ? this.limiteEstimuloEstadoInferior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoInferior;
      endif
    endif
  endfunction

  function AtualizaComportamentoMarcador(this)
    var corTEste := this.estado * 13;
    case(this.estado)
      ESTADO_PROPAGACAO_DORMENTE:
        this.esteItemMarcador.graphic := 0x19AB;
        this.esteItemMarcador.color := 900;
        break;
      ESTADO_PROPAGACAO_LATENTE:
        this.esteItemMarcador.graphic := 0x19AB;
        this.esteItemMarcador.color := corTEste;
        break;
      ESTADO_PROPAGACAO_PRINCIPIO:
        this.esteItemMarcador.graphic := 0x19AB;
        this.esteItemMarcador.color := 0;
        break;
      ESTADO_PROPAGACAO_REGULAR:
        this.esteItemMarcador.graphic := 0xB007;
        this.esteItemMarcador.color := 0;
        break;
      ESTADO_PROPAGACAO_INTENSO:
        this.esteItemMarcador.graphic := 0x6E26;
        this.esteItemMarcador.color := 0;
        break;
    endcase
  endfunction

  function _DeveAvancarEstado(this, valorAvanco)
    var resultadoAvanco := this.estado + valorAvanco;
    if(resultadoAvanco > ESTADO_PROPAGACAO_LATENTE)
      return  this.isAcumulando;
    else
      return true;
    endif
  endfunction

  function _DeveRegredirEstado(this, valorAvanco)
    return true;
  endfunction
  
  function _isMedidorAcimaLimiteEstado(this)
    return this.medidorEstimulo >= this.limiteEstimuloEstadoSuperior;
  endfunction

  function _isMedidorAbaixoLimiteEstado(this)
    return this.medidorEstimulo <= this.limiteEstimuloEstadoInferior;
  endfunction

  function _CalculaLimiteEstimuloSuperior(this)
    var limiteEstimuloEstadoSuperior := Round(cint(this.estimuloLimite) / cint(this.fatorMudancaEstado));
    limiteEstimuloEstadoSuperior := limiteEstimuloEstadoSuperior <= 0 ? 1 : limiteEstimuloEstadoSuperior;

    return Cint(limiteEstimuloEstadoSuperior);
  endfunction

  function _CalculaLimiteEstimuloInferior(this)
    return Cint(-this.estimuloLimite);
  endfunction

  function _ObtemQuantidadeAvancoEstado(this)
    var limiteSuperiorExtrapolado := cint(this.medidorEstimulo) / this.limiteEstimuloEstadoSuperior;
    var isLimiteSuperiorExtrapolado := limiteSuperiorExtrapolado > 1;
    var retorno;

    if(isLimiteSuperiorExtrapolado)
          retorno := this.estado + Round(limiteSuperiorExtrapolado);
    else
         retorno := 1;
    endif

    return retorno;
  endfunction

    function _ObtemQuantidadeRegressaoEstado(this)
    var limiteInferiorExtrapolado := cint(this.medidorEstimulo) / (cint(this.limiteEstimuloEstadoInferior) * -1);
    var isLimiteInferiorExtrapolado := limiteInferiorExtrapolado < -1;
    var retorno;

    if(isLimiteInferiorExtrapolado)
        retorno := cInt(limiteInferiorExtrapolado);
    else
         retorno := 1;
    endif

    return retorno;
  endfunction

  function PegaEstado(this)
    return this.estado;
  endfunction

  function DevePropagar(this)
    return 
        this.estado > ESTADO_PROPAGACAO_LATENTE
        && !this.IsMarcadorMorto;
  endfunction

  function Propagar(this)
    return this.MarcarAdjacentes();
  endfunction

endclass