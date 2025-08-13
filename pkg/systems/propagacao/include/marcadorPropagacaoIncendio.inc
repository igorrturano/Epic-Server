use cfgfile;
use Util;

include ":propagacao:include/marcadorPropagacaoBase";
include ":propagacao:include/propagacao";
include "/include/epicPropNames";
include "include/client";
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
    this.ciclosDormente := 0;
    this.listaStaticosInflamaveis :=  GetConfigStringArray(propagacaoConfig["incendio"],"staticInflamavel");
    this.limiteEstimuloEstadoSuperior := this._CalculaLimiteEstimuloSuperior();
    this.limiteEstimuloEstadoInferior := this._CalculaLimiteEstimuloInferior();
  endfunction

  function EfetuarCiclo(this)
    if(!this.isMarcadorMorto)
     // Print("*PROPAGACAO* Iniciando Ciclo do marcador {} ".format(this.esteItemMarcador.serial));
      var tileProspectado := this.Prospectar();
      this.isAcumulando := false;

      this._AvaliaIsAcumulando(tileProspectado);

      //TODO verificar statics
      //TODO verificar chao

      if(this.isAcumulando && this.estado > ESTADO_PROPAGACAO_LATENTE)
          //Print("*PROPAGACAO* Marcador {} Recebeu estimulo positivo.".format(this.esteItemMarcador.serial));
          this.RecebeEstimulo(this.estado, this.estado);
      else
          //Print("*PROPAGACAO* Marcador {} Recebeu estimulo negativo.".format(this.esteItemMarcador.serial));
          var valorEstimulo := -this.estado;
          if(this.ciclosDesdeUltimoEstimulo > 0)
            valorEstimulo -= this.ciclosDesdeUltimoEstimulo;
          endif 
          this.RecebeEstimulo(valorEstimulo, this.estado);
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

  function _AvaliaIsAcumulando(this, TileProspectado)
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
              //Print("this.esteItemMarcador.serial - " + this.esteItemMarcador.serial + " z - " + this.esteItemMarcador.z + " --- " + estatico.objtype  + " z - " + estatico.z);
              if(isAoAlcance)
                if(cstr(estatico.objtype) in (this.listaStaticosInflamaveis))
                   //Print("estatico " + estatico.objtype);
                  this.combustiveis.insert("estaticos",100);
                  this.isAcumulando := true;
                  this.fatorMudancaEstado := 1; //TODO dar um jeito de cada static dar um diferente.
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

  function OnRecebeEstimulo(this,valor)

    if(valor > 0)
        this.ZeraCiclosDesdeUltimoEstimuloPositivo();
    endif

    if(valor <= 0)
      if(this.ciclosDormente >= cint(this.ciclosDormenteLimite) && this.estado == ESTADO_PROPAGACAO_DORMENTE)
        this.isMarcadorMorto := true;
         //Print("*PROPAGACAO* Marcador {} Morreu após ficar muitos ciclos Dormente.".format(this.esteItemMarcador.serial));
      endif
      if(valor == 0)
        return 1;
      endif
    endif

    return 0;

  endfunction

  function RecebeEstimulo(this, valor ,isRemetenteAcumulando := false)
    if(this.OnRecebeEstimulo(valor))
      return 0;
    endif
    
    this.medidorEstimulo += (this._DeveInverterEstimulo(valor,isRemetenteAcumulando) ?  valor * -1 : valor);
    //Print("*PROPAGACAO* Marcador {} mudou seu medidor de estimulo para {}.".format(this.esteItemMarcador.serial, this.medidorEstimulo));
    this.AtualizaEstado();
  endfunction

  function _DeveInverterEstimulo(this, valorEstimulo, isRemetenteAcumulando)
    if(isRemetenteAcumulando && !this.isAcumulando && this.estado > ESTADO_PROPAGACAO_LATENTE && valorEstimulo > 0)
      return true;
    elseif(!isRemetenteAcumulando && !this.isAcumulando && valorEstimulo > 0)
      if(this.estado > ESTADO_PROPAGACAO_LATENTE)
        return true;
      endif
    endif
    return false;
  endfunction

  function EnviaEstimulo(this, valor)
    //Print("*PROPAGACAO* Marcador {} Enviou estimulo.".format(this.esteItemMarcador.serial));
    foreach adjacente in (this.marcadoresAdjacentes)
      Sleepms(2);
      var propAdjacente := GetObjProperty(adjacente, PROPNAME_PROP_PROPAGACAO);
      var pidAdjacente := GetProcess(propAdjacente.esteMarcadorPid);
      //Print("*PROPAGACAO* Marcador {} Enviou {} estimulo para o processo {}.".format(this.esteItemMarcador.serial, valor ,pidAdjacente));

      if(pidAdjacente)
        pidAdjacente.SendEvent(struct{id := PROPAGACAO_EV_ESTIMULO, valor := valor, isAcumulando := this.isAcumulando, estado := this.estado});
      endif
      
    endforeach
  endfunction

  function AtualizaEstado(this);
    this.limiteEstimuloEstadoSuperior := _CalculaLimiteEstimuloSuperior(this);
    this.limiteEstimuloEstadoInferior := _CalculaLimiteEstimuloInferior(this);

    if(GetObjProperty(this.esteItemMarcador,"DEBUG"))
      Print(" --- this.estimuloLimite: {} - this.medidorEstimulo: {} - limiteEstimuloEstadoInferior: {} ".format(this.estimuloLimite,this.medidorEstimulo, this.limiteEstimuloEstadoInferior));
    endif

    if(this._isMedidorAcimaLimiteEstado())
      var valorAvanco := this._ObtemQuantidadeAvancoEstado();

      if(this._DeveAvancarEstado(valorAvanco))
        var novoValorEstado := this.estado + valorAvanco;
        var isMaximo := novoValorEstado > ESTADO_PROPAGACAO_INTENSO;
        this.estado := isMaximo ? ESTADO_PROPAGACAO_INTENSO : novoValorEstado;
        if(!isMaximo)
          this.AtualizaComportamentoMarcador();
        endif
        //Print("*PROPAGACAO* Marcador {} Subiu de Estado e agora é {}.".format(this.esteItemMarcador.serial, this.estado));
        this.medidorEstimulo := isMaximo ? this.limiteEstimuloEstadoSuperior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoSuperior;
      endif
    elseif(this._isMedidorAbaixoLimiteEstado())
      var valorRegresso := this._ObtemQuantidadeRegressaoEstado();
      if(GetObjProperty(this.esteItemMarcador,"DEBUG"))
        Print(" --- this.valorRegresso: {} - this.medidorEstimulo: {} - limiteEstimuloEstadoInferior: {} ".format(this.valorRegresso,this.medidorEstimulo, this.limiteEstimuloEstadoInferior));
      endif
      //Print("*PROPAGACAO* Marcador  estado agora é {}".format( this.estado));
      if(this._DeveRegredirEstado(valorRegresso))
        var novoValorEstado := this.estado - valorRegresso;
        var isMinimo := novoValorEstado < ESTADO_PROPAGACAO_DORMENTE;
        this.estado := isMinimo ? ESTADO_PROPAGACAO_DORMENTE : novoValorEstado;
        if(!isMinimo)
          this.AtualizaComportamentoMarcador();
        endif
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
        this.esteItemMarcador.invisible := 1;
        this.esteItemMarcador.color := 900;
        break;
      ESTADO_PROPAGACAO_LATENTE:
        this.esteItemMarcador.graphic := 0x0DE9;
        this.esteItemMarcador.invisible := 0;
        //PlayObjectCenteredEffectEx(this.esteItemMarcador,0x377A,1,255,1359);
        break;
      ESTADO_PROPAGACAO_PRINCIPIO:
        this.esteItemMarcador.graphic := 0x19AB;
        this.esteItemMarcador.color := 0;
        this.esteItemMarcador.invisible := 0;
        PlaySoundEffect( this.esteItemMarcador,SFX_1DE);
        PlayObjectCenteredEffectEx(this.esteItemMarcador,0x3728,1,10);
        var random := RandomIntMinMax(1,8);
        if(random < 2)
          CreateItemAtLocation(this.esteItemMarcador.x,this.esteItemMarcador.y, this.esteItemMarcador.z,0x3CFF,1,this.esteItemMarcador.realm);
        endif
        break;
      ESTADO_PROPAGACAO_REGULAR:
        this.esteItemMarcador.graphic := 0xB007;
        this.esteItemMarcador.color := 0;
        this.esteItemMarcador.invisible := 0;
        PlaySoundEffect( this.esteItemMarcador,SFX_1DE);
        PlayObjectCenteredEffectEx(this.esteItemMarcador,0x3728,1,10);
        var random := RandomIntMinMax(1,8);
        if(random < 2)
          CreateItemAtLocation(this.esteItemMarcador.x,this.esteItemMarcador.y, this.esteItemMarcador.z,0x3CFF,1,this.esteItemMarcador.realm);
        endif
        break;
      ESTADO_PROPAGACAO_INTENSO:
        this.esteItemMarcador.graphic := 0x6E26 ;
        this.esteItemMarcador.color := 0;
        this.esteItemMarcador.invisible := 0;
        PlaySoundEffect( this.esteItemMarcador,SFX_1DE);
        PlayObjectCenteredEffectEx(this.esteItemMarcador,0xB46F,1,100);
        var random := RandomIntMinMax(1,8);
        if(random < 2)
          CreateItemAtLocation(this.esteItemMarcador.x,this.esteItemMarcador.y, this.esteItemMarcador.z,0x3CFF,1,this.esteItemMarcador.realm);
        endif
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
    if(this.estado <= ESTADO_PROPAGACAO_LATENTE)
      return Cint(-this.estimuloLimite / 2);
    endif
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