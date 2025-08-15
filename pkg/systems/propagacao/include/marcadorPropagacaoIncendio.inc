use cfgfile;
use uo;
use Util;

include ":propagacao:include/marcadorPropagacaoBase";
include ":propagacao:include/propagacao";
include ":propagacao:include/combustivelIncendio";
include "/include/epicPropNames";
include "/include/epicConstants";
include "include/client";
include "/include/math";
include ":destroyable:include/destroyItem";
include ":combat:damage";

class MarcadorPropagacaoIncendio( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoIncendio(this, esteItemMarcador, propPropagacao)
    super(esteItemMarcador,propPropagacao);

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.mobileOrigemIncendio := SystemFindObjectBySerial( this.propPropagacao.origem, SYSFIND_SEARCH_OFFLINE_MOBILES);
    this.resourceConfig := ReadConfigFile(":crafting:config/resourceList");
    this.combustiveis := array{CombustivelEstaticos(),CombustivelTerreno()};
    this.estimuloLimite := propagacaoConfig["incendio"].estimuloLimite;
    this.ciclosDormenteLimite := propagacaoConfig["incendio"].ciclosDormenteLimite;
    this.ciclosDormente := 0;
    this.limiteEstimuloEstadoSuperior := this._CalculaLimiteEstimuloSuperior();
    this.limiteEstimuloEstadoInferior := this._CalculaLimiteEstimuloInferior();
    this.catalogoDano := propPropagacao.catalogoDano ? propPropagacao.catalogoDano : this.AtribuiCatalogoDano(GetConfigStringArray(propagacaoConfig["incendio"],"danoPorEstadoBase"));
  endfunction

  function AtribuiCatalogoDano(this, arrayCatalogo)
     return MontaCatalogoDano(arrayCatalogo);
  endfunction

  function EfetuarCiclo(this)
    if(!this.isMarcadorMorto)
      var tileProspectado := this.Prospectar();
      this.isAcumulando := false;

      this.ProcessaTile(tileProspectado);

      if(this.isAcumulando && this.estado > ESTADO_PROPAGACAO_LATENTE)
          this.RecebeEstimulo(this.estado);
      else
          var valorEstimulo := -this.estado;
          if(this.ciclosDesdeUltimoEstimulo > 0)
            valorEstimulo -= this.ciclosDesdeUltimoEstimulo;
          endif 
          this.RecebeEstimulo(valorEstimulo);
          this.IncrementaCiclosDesdeUltimoEstimulo();
      endif

      if(this.DevePropagar())
        this.Propagar();
        this.EnviaEstimulo(this.estado);
      endif
    endif
  endfunction

  function ProcessaTile(this, tileProspectado)

    var danoDado := this.catalogoDano[cstr(this.estado)]; 
    var rolagemDano := RandomDiceRoll(danoDado);

    foreach item in (tileProspectado.PegaItensEncontrados())
      Sleepms(2);

      if( item.serial == this.esteItemMarcador.serial) continue; endif

      if(item.isMarcadorTipo(PROPAGACAO_TIPO_INCENDIO) && item.serial != this.esteItemMarcador.serial)
        DestroyItem(item); // Marreta contra condição de corrida nr 2;
        continue;
      endif

      this.isAcumulando := this._AvaliaInflamabilidade(item);
      if(this.estado > ESTADO_PROPAGACAO_DORMENTE)
        DanificaItem(this.mobileOrigemIncendio,item,struct {desc := "Incêndio", dano := rolagemDano, tipo := DMG_FIRE});
      endif
      
    endforeach

    foreach mobile in (tileProspectado.PegaMobilesEncontrados())
      Sleepms(2);
      if(this.estado > ESTADO_PROPAGACAO_DORMENTE)
        DamageFLS(mobile, cint(rolagemDano), DMG_FIRE, 0, "Incêndio");
      endif
    endforeach
    if(!this.isAcumulando)
      this._VerificaCombustivel(TileProspectado);
    endif

  endfunction

  function _VerificaCombustivel(this,tileProspectado)
      foreach tipoCombustivel in (this.combustiveis)
        Sleepms(2);
        
        if(tipoCombustivel.TemCombustivel())
          tipoCombustivel.ConsomeCombustivel(this.estado);
          this.isAcumulando := true;
        else
          this.isAcumulando := tipoCombustivel.ProcuraCombustivel(this.esteItemMarcador,tileProspectado);
        endif
        if(this.isAcumulando)
          if(this.fatorMudancaEstado != tipoCombustivel.fatorInflamavel)
            this.fatorMudancaEstado := tipoCombustivel.fatorInflamavel;
          endif
          break;
        endif
      endforeach
  endfunction

  function _AvaliaInflamabilidade(this, item)
    var material :=  this.resourceConfig[GetObjProperty(item, PROPNAME_MATERIAL)];
    var isIndestrutivel := GetObjProperty(item,PROPNAME_INDESTRUTIVEL);
    if(material.inflamavel && !isIndestrutivel)
      if(this.fatorMudancaEstado < material.inflamavel)
        this.fatorMudancaEstado := material.inflamavel;
      endif
      return true;
    endif
    return false;
  endfunction

  function OnRecebeEstimulo(this,valor)

    if(valor > 0)
        this.ZeraCiclosDesdeUltimoEstimuloPositivo();
    endif

    if(valor <= 0)
      if(this.ciclosDormente >= cint(this.ciclosDormenteLimite) && this.estado == ESTADO_PROPAGACAO_DORMENTE)
        this.isMarcadorMorto := true;
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
    foreach adjacente in (this.marcadoresAdjacentes)
      Sleepms(2);
      var propAdjacente := GetObjProperty(adjacente, PROPNAME_PROP_PROPAGACAO);
      var pidAdjacente := GetProcess(propAdjacente.esteMarcadorPid);

      if(pidAdjacente)
        pidAdjacente.SendEvent(struct{id := PROPAGACAO_EV_ESTIMULO, valor := valor, isAcumulando := this.isAcumulando, estado := this.estado});
      endif
      
    endforeach
  endfunction

  function AtualizaEstado(this);
    this.limiteEstimuloEstadoSuperior := _CalculaLimiteEstimuloSuperior(this);
    this.limiteEstimuloEstadoInferior := _CalculaLimiteEstimuloInferior(this);


    if(this._isMedidorAcimaLimiteEstado())
      var valorAvanco := this._ObtemQuantidadeAvancoEstado();

      if(this._DeveAvancarEstado(valorAvanco))

        var novoValorEstado := this.estado + valorAvanco;
        var isMaximo := novoValorEstado > ESTADO_PROPAGACAO_INTENSO;
        this.estado := isMaximo ? ESTADO_PROPAGACAO_INTENSO : novoValorEstado;

        if(!isMaximo)
          this.AtualizaComportamentoMarcador();
        endif

        this.medidorEstimulo := isMaximo ? this.limiteEstimuloEstadoSuperior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoSuperior;
      endif
    elseif(this._isMedidorAbaixoLimiteEstado())

      var valorRegresso := this._ObtemQuantidadeRegressaoEstado();

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
    case(this.estado)
      ESTADO_PROPAGACAO_DORMENTE:
        this.AtualizaComportamento(0x19AB, 900);
        break;
      ESTADO_PROPAGACAO_LATENTE:
        this.AtualizaComportamento(0x0DE9, 0);
        break;
      ESTADO_PROPAGACAO_PRINCIPIO:
        this.AtualizaComportamento(0x19AB, 0,0,SFX_1DE,0x3728,0x3CFF);
        break;
      ESTADO_PROPAGACAO_REGULAR:
        this.AtualizaComportamento(0xB007, 0,0,SFX_1DE,0x3728,10,0x3CFF);
        break;
      ESTADO_PROPAGACAO_INTENSO:
        this.AtualizaComportamento(0x6E26, 0,0,SFX_1DE,0x3728,100,0x3CFF);
        break;
    endcase
  endfunction

  function AtualizaComportamento(this, graphic, cor :=0, isInvisibel := 1, sfx := 0,efx := 0, duracao := 10, effectItemObjtype := 0, chanceRandom := 2 , minRandom := 1, maxRandom := 8)
    
    var efeitosPersonalizados := this.PegaPropPropagacao().efeitosPersonalizados;
    this.esteItemMarcador.graphic := efeitosPersonalizados.graphic ? efeitosPersonalizados.graphic : graphic;
    this.esteItemMarcador.color :=  efeitosPersonalizados.cor ? efeitosPersonalizados.cor : cor ;
    this.esteItemMarcador.invisible := efeitosPersonalizados.isInvisibel ? efeitosPersonalizados.isInvisibel : isInvisibel;
    if(sfx)
      PlaySoundEffect( this.esteItemMarcador,efeitosPersonalizados.sfx ? efeitosPersonalizados.sfx : sfx);
    endif

    if(efx)
      PlayObjectCenteredEffectEx(
          this.esteItemMarcador,
          efeitosPersonalizados.efx ? efeitosPersonalizados.efx : efx,
          1, 
          efeitosPersonalizados.duracao ? efeitosPersonalizados.duracao : duracao );
    endif

    var itemObjtype := efeitosPersonalizados.effectItemObjtype ? efeitosPersonalizados.effectItemObjtype : effectItemObjtype;
    
    if(itemObjtype)
      var min := efeitosPersonalizados.minRandom ? efeitosPersonalizados.minRandom : minRandom;
      var max := efeitosPersonalizados.maxRandom ? efeitosPersonalizados.maxRandom : maxRandom;
      var random := RandomIntMinMax(min,max);
      if(random < chanceRandom)
        CreateItemAtLocation(this.esteItemMarcador.x,this.esteItemMarcador.y, this.esteItemMarcador.z,itemObjtype,1,this.esteItemMarcador.realm);
      endif
    endif
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
