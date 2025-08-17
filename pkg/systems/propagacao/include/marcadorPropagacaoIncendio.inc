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
include ":nature:include/nature";

class MarcadorPropagacaoIncendio( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoIncendio(this, esteItemMarcador, propPropagacao)
    super(esteItemMarcador,propPropagacao); 

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(propPropagacao.origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
    this.resourceConfig := ReadConfigFile(":crafting:config/resourceList");
    this.combustiveis := dictionary{
                            PROPAGACAO_COMBUSTIVEL_ESTATICOS -> CombustivelEstaticos(), 
                            PROPAGACAO_COMBUSTIVEL_TERRENO -> CombustivelTerreno(), 
                            PROPAGACAO_COMBUSTIVEL_MAGICO -> CombustivelMagico()
                          };
    this.estimuloLimite := propagacaoConfig["incendio"].estimuloLimite;
    this.ciclosDormenteLimite := propagacaoConfig["incendio"].ciclosDormenteLimite;
    this.ciclosDormente := 0;
    this.limiteEstimuloEstadoSuperior := this._CalculaLimiteEstimuloSuperior();
    this.limiteEstimuloEstadoInferior := this._CalculaLimiteEstimuloInferior();
    this.catalogoDanoBase := MontaCatalogoDano();
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

    var danoDado := this.PegaCatalogoDano()[cstr(this.estado)]; 
    var rolagemDano := RandomDiceRoll(danoDado);

    foreach item in (tileProspectado.PegaItensEncontrados())
      Sleepms(2);

      if( item.serial == this.esteItemMarcador.serial) continue; endif

      if(item.IsMarcadorTipo(PROPAGACAO_TIPO_INCENDIO) 
        && item.serial != this.esteItemMarcador.serial 
        && this.estado > ESTADO_PROPAGACAO_DORMENTE)
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
      this._VerificaCombustivel(TileProspectado); //No caso de itens, o combustivel é a propria vida do item, como explorado acima.
      this._VerificaClima();
    endif

  endfunction

  function _VerificaClima(this)
    var clima := GetGlobalProperty("weather");
    var chuvaLeve := 0;
    
    if(!this.isMarcadorCoberto)
        if(cint(clima) == chuvaLeve && this.estado <= ESTADO_PROPAGACAO_PRINCIPIO)
          this.isAcumulando := false;
        elseif(cint(clima) > chuvaLeve )
          this.isAcumulando := false;
        endif
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

  function InjetaCombustivel(this)
    this.combustiveis.insere(PROPAGACAO_COMBUSTIVEL_MAGICO,CombustivelMagico());
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(this.PegaPropPropagacao().origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
  endfunction

  function _AvaliaInflamabilidade(this, item)
    var material :=  this.resourceConfig[GetObjProperty(item, PROPNAME_MATERIAL)];
    material := material ? material : this.resourceConfig[item.objtype];
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
        pidAdjacente.SendEvent(struct{
                      id := PROPAGACAO_EV_ESTIMULO, 
                      valor := valor, 
                      isAcumulando := this.isAcumulando, 
                      estado := this.estado});
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

        if(!isMaximo || valorAvanco > 1)
          this.AtualizaComportamentoMarcador();
        endif

        this.medidorEstimulo := isMaximo ? this.limiteEstimuloEstadoSuperior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoSuperior;
      endif
    elseif(this._isMedidorAbaixoLimiteEstado())

      var valorRegresso := this._ObtemQuantidadeRegressaoEstado();

      if(this._DeveRegredirEstado(valorRegresso))

        var novoValorEstado := this.estado + valorRegresso;
        var isMinimo := novoValorEstado < ESTADO_PROPAGACAO_DORMENTE;
        this.estado := isMinimo ? ESTADO_PROPAGACAO_DORMENTE : novoValorEstado;
       
        if(!isMinimo || valorRegresso < 1)
          this.AtualizaComportamentoMarcador();
        endif

        this.medidorEstimulo := isMinimo ? this.limiteEstimuloEstadoInferior : 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoInferior;
      endif
    endif
  endfunction

  function AtualizaComportamentoMarcador(this)
    var efeitosPersonalizados := this.PegaPropPropagacao().efeitosPersonalizados[cstr(this.estado)];
    var efeitoFumacinha := 0x3728;
    var objtypeNuvemFumaca := 0x3CFF;
    var efeitoFogo := 0xB46F;

    var fogoPequeno := 0x19AB;
    var brasas := 0x0DE9;
    var fogoMedio := 0xB007;
    var fogoIntenso := 0x6E26;

    case(this.estado)
      ESTADO_PROPAGACAO_DORMENTE:
        this.AtualizaComportamento(efeitosPersonalizados,fogoPequeno, 900);
        break;
      ESTADO_PROPAGACAO_LATENTE:
        this.AtualizaComportamento(efeitosPersonalizados,brasas, 0);
        break;
      ESTADO_PROPAGACAO_PRINCIPIO:
        this.AtualizaComportamento(efeitosPersonalizados,fogoPequeno, 0,0,SFX_1DE,efeitoFumacinha,objtypeNuvemFumaca);
        break;
      ESTADO_PROPAGACAO_REGULAR:
        this.AtualizaComportamento(efeitosPersonalizados,fogoMedio, 0,0,SFX_1DE,efeitoFumacinha,10,objtypeNuvemFumaca);
        break;
      ESTADO_PROPAGACAO_INTENSO:
        this.AtualizaComportamento(efeitosPersonalizados,fogoIntenso, 0,0,SFX_1DE,efeitoFogo,100,objtypeNuvemFumaca);
        break;
    endcase
  endfunction

  function AtualizaComportamento(this,efeitosPersonalizados, graphic, cor :=0, isInvisibel := 1, sfx := -1,efx := -1, duracao := 10, effectItemObjtype := -1, chanceRandom := 2 , minRandom := 1, maxRandom := 8)
     
    this.esteItemMarcador.graphic := efeitosPersonalizados.graphic ? efeitosPersonalizados.graphic : graphic;
    this.esteItemMarcador.color :=  efeitosPersonalizados.cor ? efeitosPersonalizados.cor : cor ;
    this.esteItemMarcador.invisible := efeitosPersonalizados.isInvisibel ? efeitosPersonalizados.isInvisibel : isInvisibel;
    if(efeitosPersonalizados.sfx != -1)
      PlaySoundEffect( this.esteItemMarcador,efeitosPersonalizados.sfx ? efeitosPersonalizados.sfx : sfx);
    endif

    if(efeitosPersonalizados.efx != -1)
      PlayObjectCenteredEffectEx(
          this.esteItemMarcador,
          efeitosPersonalizados.efx ? efeitosPersonalizados.efx : efx,
          1, 
          efeitosPersonalizados.duracao ? efeitosPersonalizados.duracao : duracao );
    endif

    var itemObjtype;
    
    if(efeitosPersonalizados.effectItemObjtype != -1)
      itemObjtype := efeitosPersonalizados.effectItemObjtype ? efeitosPersonalizados.effectItemObjtype : effectItemObjtype;
    endif

    if(itemObjtype)
      var min := efeitosPersonalizados.minRandom ? efeitosPersonalizados.minRandom : minRandom;
      var max := efeitosPersonalizados.maxRandom ? efeitosPersonalizados.maxRandom : maxRandom;
      var random := RandomIntMinMax(min,max);
      if(random < chanceRandom)
        var item := CreateItemAtLocation(this.esteItemMarcador.x,
                                        this.esteItemMarcador.y, 
                                        this.esteItemMarcador.z,
                                        itemObjtype,
                                        1,
                                        this.esteItemMarcador.realm);
        SetObjProperty(item, PROPNAME_INDESTRUTIVEL, 1);
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

    var limiteEstimuloEstadoSuperior := Round(cint(this.estimuloLimite * this._CalcuraInfluenciaTemperatura())  / cint(this.fatorMudancaEstado));
    limiteEstimuloEstadoSuperior := limiteEstimuloEstadoSuperior <= 0 ? 1 : limiteEstimuloEstadoSuperior;
    return Cint(limiteEstimuloEstadoSuperior);
  endfunction

  function _CalcuraInfluenciaTemperatura(this)
    var temperaturaRegion := getTemperature(this.esteItemMarcador) + PROPAGACAO_TEMPERATURA_BASE;
    var sinal := (temperaturaRegion >= 0) ? 1 : -1;
    var porcentagem := Abs(temperaturaRegion) * 0.4 / 100;
    return (1 - sinal * porcentagem);
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
         retorno := -1;
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

  function PegaCatalogoDano(this)
    var catalogoDano := this.PegaPropPropagacao().catalogoDano;
    if(catalogoDano)
      return catalogoDano;
    else
      return this.catalogoDanoBase;
    endif 
  endfunction

endclass
