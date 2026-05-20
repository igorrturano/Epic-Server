use cfgfile;
use uo;
use util;

include ":propagacao:include/marcadores/marcadorPropagacaoBase";
include ":propagacao:include/propagacao";
include ":propagacao:include/afetaMobilePropagacao";
include ":propagacao:include/combustiveis/combustivelIncendio";
include ":propagacao:include/combustiveis/impl/combustivelEstaticos";
include ":propagacao:include/combustiveis/impl/combustivelTerreno";
include ":propagacao:include/combustiveis/impl/combustivelMagico";
include "/include/epicPropNames";
include "/include/epicConstants";
include "include/client";
include "/include/math";
include ":destroyable:include/destroyItem";
include ":combat:Model/Damage";
include ":nature:include/nature";

class MarcadorPropagacaoIncendio( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoIncendio(this, esteItemMarcador, propPropagacao, momentoNascimento)
    super(esteItemMarcador,propPropagacao, momentoNascimento); 

    esteItemMarcador.usescript := ":propagacao:dblClickMarcadorIncendio";

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(propPropagacao.origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
    this.resourceConfig := ReadConfigFile(":crafting:config/resourceList");
    this.combustiveis := dictionary{
                            COMBUSTIVEIS::ESTATICOS -> CombustivelEstaticos(), 
                            COMBUSTIVEIS::TERRENO -> CombustivelTerreno(), 
                            COMBUSTIVEIS::MAGICO -> CombustivelMagico()
                          };
    this.estimuloLimite := propagacaoConfig["incendio"].estimuloLimite;
    this.ciclosDormenteLimite := propagacaoConfig["incendio"].ciclosDormenteLimite;
    this.ciclosDormente := 0;
    this.limiteEstimuloEstadoSuperior := this._CalculaLimiteEstimuloSuperior();
    this.limiteEstimuloEstadoInferior := this._CalculaLimiteEstimuloInferior();
    this.catalogoDanoBase := MontaCatalogoDano();
    this.aceleradorContraLatente := cint(propagacaoConfig["incendio"].contraLatente);
    this.multiplicadorDescansoDormente := cint(propagacaoConfig["sistema"].multiplicadorDescansoDormente);
    this.tileProspectado := this.Prospectar();
  endfunction

  function EfetuarCiclo(this)
    if(!this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO))
      this.tileProspectado := this.Prospectar(array {TILE_SCANNER::ITENS, TILE_SCANNER::MOBILES}, this.tileProspectado);
      
      this.isAcumulando := false;

      if(this.IsEstadoControle(CONTROLE_PROPAGACAO::ATIVO))
        this._EfetuarCicloAtivo(this.tileProspectado);
      else
        this._EfetuarCicloDormente(this.tileProspectado);
      endif
      this.ciclos++;
    endif
  endfunction

  function Iniciar(this)
    this.AtualizaEstado(this.estado, true);
    this.MarcarAdjacentes();
    if(!this.tileProspectado)
      this.tileProspectado := this.Prospectar();
    endif
  endfunction

  function _EfetuarCicloAtivo(this,tileProspectado)
    var isAlvoEncontrado := this._ProcessaAlvos(tileProspectado);
    if(!isAlvoEncontrado)
      this.estadoControle := CONTROLE_PROPAGACAO::DORMENTE;
    endif
    this._ExecutaRotinaPropagacao(tileProspectado);

    return isAlvoEncontrado;
  endfunction

  function _EfetuarCicloDormente(this, tileProspectado)
    var isAlvoEncontrado := this._ProcessaAlvos(tileProspectado);

    if(isAlvoEncontrado)
      this.estadoControle := CONTROLE_PROPAGACAO::ATIVO;
    endif

    this._ExecutaRotinaPropagacao(tileProspectado);
  endfunction

  function _ExecutaRotinaPropagacao(this, tileProspectado)
    Sleepms(40);
    if(!this.isAcumulando)
      this._ProcessaCombustivel(tileProspectado);
    endif
    this._ProcessaClima();
    Sleepms(40);
    this._ProcessaEstimulos();
    this.TocaSFXCaracteristico();
  endfunction

  function _ProcessaEstimulos(this)
    this._ProcessaAutoEstimulo();
    
    if(this.DevePropagar())
      this.Propagar();
      this.EnviaEstimulo(this.estado);
    endif
  endfunction

  function _ProcessaAutoEstimulo(this)
    if(this.isAcumulando && this.estado >= ESTADO_PROPAGACAO::LATENTE)
        this.RecebeEstimulo(this.estado);
    else
        var valorEstimulo := -this.estado;
        if(this.ciclosDesdeUltimoEstimulo > 0 && this.estado == ESTADO_PROPAGACAO::LATENTE && !this.isAcumulando)
          valorEstimulo -= this.ciclosDesdeUltimoEstimulo * this.aceleradorContraLatente;
        endif 
        this.RecebeEstimulo(valorEstimulo);
        this.IncrementaCiclosDesdeUltimoEstimulo();
    endif
  endfunction

  function _ProcessaAlvos(this, tileProspectado)
    var danoDado := this.PegaCatalogoDano()[cstr(this.estado)]; 
    var rolagemDano := RandomDiceRoll(danoDado);
    var isAlvoEncontrado := false;
    foreach item in (tileProspectado.PegaItensEncontrados())
      Sleepms(2);
      if(!item) continue; endif//Estanhamente tá vindo um unintialized as vezes. maluquice

      if( item.serial == this.esteItemMarcador.serial) continue; endif

      if(item.IsMarcadorTipo(PROPAGACAO_TIPO::INCENDIO) 
        && item.serial != this.esteItemMarcador.serial 
        && this.estado > ESTADO_PROPAGACAO::DORMENTE)
        DestroyItem(item); // Marreta contra condição de corrida nr 2;
        continue;
      endif

      this.isAcumulando := this.isAcumulando ? this.isAcumulando : this._AvaliaInflamabilidade(item);
      isAlvoEncontrado := true;

      if(this.estado > ESTADO_PROPAGACAO::DORMENTE)
        DanificaItem(this.mobileOrigemIncendio,item,struct {desc := "Incêndio", dano := rolagemDano, tipo := DAMAGE_TYPE::FIRE});
      endif
    endforeach

    if(this.estado > ESTADO_PROPAGACAO::DORMENTE)
      foreach mobile in (tileProspectado.PegaMobilesEncontrados())
        Sleepms(2);
        if(!mobile) continue; endif//Estanhamente tá vindo um unintialized as vezes. maluquice
        isAlvoEncontrado := true;
        AfetaMobileIncendiado(mobile,this.estado,this.PegaPropPropagacao(),rolagemDano);
      endforeach
    endif

    return isAlvoEncontrado;
  endfunction

  function _ProcessaClima(this)
    var clima := GetGlobalProperty("weather");
    var chuvaLeve := 0;
    if(!this.isMarcadorCoberto && TypeOfInt(clima) != OT_ERROR)
        if(cint(clima) == chuvaLeve && this.estado <= ESTADO_PROPAGACAO::PRINCIPIO)
          this.isAcumulando := false;
        elseif(cint(clima) > chuvaLeve )
          this.isAcumulando := false;
        endif
    endif
  endfunction

  function _ProcessaCombustivel(this,tileProspectado)
      foreach tipoCombustivel in (this.combustiveis)
        Sleepms(2);
        
        if(tipoCombustivel.TemCombustivel())
          this._ConsomeCombustivel(tipoCombustivel);
          return 1;
          break;
        else
          var temCombustivel := tipoCombustivel.ProcuraCombustivel(this.esteItemMarcador,tileProspectado);
          if(temCombustivel)
            this._ConsomeCombustivel(tipoCombustivel);
            return 1;
          endif
        endif
      endforeach

      return 0;
  endfunction

  function _ConsomeCombustivel(this, tipoCombustivel)
    this.fatorMudancaEstado := tipoCombustivel.fatorInflamavel;

    tipoCombustivel.ConsomeCombustivel(this.estado);
    this.isAcumulando := true;
  endfunction

  function InjetaCombustivel(this, deveResetarEstado)
    this.combustiveis.insert(COMBUSTIVEIS::MAGICO,CombustivelMagico());
    if(deveResetarEstado)
      this.AtualizaEstado(this.PegaPropPropagacao().estadoInicial);
    endif
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(this.PegaPropPropagacao().origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
  endfunction

  function _AvaliaInflamabilidade(this, item)

    var material :=  this.resourceConfig[GetObjProperty(item, PROPNAME_MATERIAL)];
    //TODO [futuro] Ver se não seria melhor tranformar os recursos como logs e minerio bruto em resources no ResourceList tbm.
    //Pra poder remover esse catalogoPraTabua daqui.
    material := material ? material : this.resourceConfig[CATALOGO_LOG_PRA_TABUA[Lower(HEX(item.objtype))]];
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
      if(this.ciclosDormente >= cint(this.ciclosDormenteLimite) && this.estado == ESTADO_PROPAGACAO::DORMENTE)
        this.estadoControle := CONTROLE_PROPAGACAO::MORTO;
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
    
    /* // [Debug]
    Print("processo {}; ciclo {}: Recebeu estímulo: {} - medidor: {} - acumulando ?: {} - estado: {}".format(this.esteItemMarcador.serial, this.ciclos, valor, this.medidorEstimulo, this.isAcumulando, this.estado));
    */

    this.medidorEstimulo += (this._DeveInverterEstimulo(valor,isRemetenteAcumulando) ?  valor * -1 : valor);

    this.ProcessaMudancaEstado();
  endfunction

  function _DeveInverterEstimulo(this, valorEstimulo, isRemetenteAcumulando)
    if(isRemetenteAcumulando && !this.isAcumulando && this.estado > ESTADO_PROPAGACAO::LATENTE && valorEstimulo > 0)
      return true;
    elseif(!isRemetenteAcumulando && !this.isAcumulando && valorEstimulo > 0)
      if(this.estado > ESTADO_PROPAGACAO::LATENTE)
        return true;
      endif
    endif
    return false;
  endfunction

  function EnviaEstimulo(this, valor)
    foreach adjacente in (this.marcadoresAdjacentes)
      Sleepms(RandomIntMinMax(20, 80));
      var propAdjacente := GetObjProperty(adjacente, PROPNAME_PROP_PROPAGACAO);
      var pidAdjacente := GetProcess(propAdjacente.esteMarcadorPid);

      if(pidAdjacente)
        pidAdjacente.SendEvent(struct{
                      id := EV_PROPAGACAO::ESTIMULO, 
                      valor := valor, 
                      isAcumulando := this.isAcumulando, 
                      estado := this.estado});
      endif
      
    endforeach
  endfunction

  function GanchoAntesAtualizaEstado(this, valor)
    var valorAntigo := this.estado;
    var isMaximo := valor > ESTADO_PROPAGACAO::INTENSO;
    var isMinimo := valor < ESTADO_PROPAGACAO::DORMENTE;

    if(valor > ESTADO_PROPAGACAO::INTENSO)
      valor :=  ESTADO_PROPAGACAO::INTENSO;
    else
      if( valor < ESTADO_PROPAGACAO::DORMENTE)
        valor := ESTADO_PROPAGACAO::DORMENTE;
      endif
    endif

    return valor;
  endfunction

  function ProcessaMudancaEstado(this);
    this.limiteEstimuloEstadoSuperior := _CalculaLimiteEstimuloSuperior(this);
    this.limiteEstimuloEstadoInferior := _CalculaLimiteEstimuloInferior(this);
    /* // [Debug]
    Print("processo {}; ciclo {};  - medidor: {} - acumulando ?: {} - estado: {} - limites: {}|{}"
      .format(
      this.esteItemMarcador.serial, 
      this.ciclos,
      this.medidorEstimulo,
      this.isAcumulando,
      this.estado,
      this.limiteEstimuloEstadoSuperior,
      this.limiteEstimuloEstadoInferior)); 
    */

    if(this._isMedidorAcimaLimiteEstado())  
      var valorAvanco := this._ObtemQuantidadeAvancoEstado();
    
      if(this._DeveAvancarEstado(valorAvanco))

        var novoValorEstado := this.estado + valorAvanco;
        this.AtualizaEstado(novoValorEstado);

        this.medidorEstimulo := 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoSuperior;
      endif
    elseif(this._isMedidorAbaixoLimiteEstado())

      var valorRegresso := this._ObtemQuantidadeRegressaoEstado();

      if(this._DeveRegredirEstado(valorRegresso))

        var novoValorEstado := this.estado + valorRegresso;

        this.AtualizaEstado(novoValorEstado);

        this.medidorEstimulo := 0;
      else
        this.medidorEstimulo := this.limiteEstimuloEstadoInferior;
      endif
    endif
  endfunction

  function AtualizaComportamentoMarcador(this)
    Sleepms(40);
    var efeitosPersonalizados := this.PegaPropPropagacao().efeitosPersonalizados[cstr(this.estado)];
    var efeitoFumacinha := 0x3728;
    var objtypeNuvemFumaca := 0x3CFF;
    var efeitoFogo := 0xB46F;

    var fogoPequeno := 0x19AB;
    var brasas := 0x0DE9;
    var fogoMedio := 0xB007;
    var fogoIntenso := 0x6E26;

    case(this.estado)
      ESTADO_PROPAGACAO::DORMENTE:
        this.AtualizaComportamento(efeitosPersonalizados,fogoPequeno, 900);
        break;
      ESTADO_PROPAGACAO::LATENTE:
        this.AtualizaComportamento(efeitosPersonalizados,brasas, 0, 0);
        break;
      ESTADO_PROPAGACAO::PRINCIPIO:
        this.AtualizaComportamento(efeitosPersonalizados,fogoPequeno, 0,0,efeitoFumacinha,objtypeNuvemFumaca);
        break;
      ESTADO_PROPAGACAO::REGULAR:
        this.AtualizaComportamento(efeitosPersonalizados,fogoMedio, 0,0,efeitoFumacinha,10,objtypeNuvemFumaca);
        break;
      ESTADO_PROPAGACAO::INTENSO:
        this.AtualizaComportamento(efeitosPersonalizados,fogoIntenso, 0,0,efeitoFogo,100,objtypeNuvemFumaca);
        break;
    endcase
  endfunction

  function AtualizaComportamento(this,efeitosPersonalizados, graphic, cor :=0, isInvisibel := 1,efx := -1, duracao := 10, effectItemObjtype := -1, chanceRandom := 2 , minRandom := 1, maxRandom := 8)
     
    this.esteItemMarcador.graphic := efeitosPersonalizados.graphic ? efeitosPersonalizados.graphic : graphic;
    this.esteItemMarcador.color :=  efeitosPersonalizados.cor ? efeitosPersonalizados.cor : cor ;
    this.esteItemMarcador.invisible := efeitosPersonalizados.isInvisibel ? efeitosPersonalizados.isInvisibel : isInvisibel;
    this.TocaSFXCaracteristico();

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
    if(resultadoAvanco > ESTADO_PROPAGACAO::LATENTE)
      return  this.isAcumulando;
    else
      return true;
    endif
  endfunction

  function _DeveRegredirEstado(this, valorRegresso)
    var isEstadoDormente := 
      this.IsEstadoMarcador(ESTADO_PROPAGACAO::DORMENTE) && 
      (this.medidorEstimulo - valorRegresso) <= this.limiteEstimuloEstadoInferior;
    return !isEstadoDormente;
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
    //var temperaturaRegion := getTemperature(this.esteItemMarcador) + PROPAGACAO_TEMPERATURA_BASE; pegar  a temperatura eestava deixando tudo 1000x mais pesado.
    var temperaturaRegion := PROPAGACAO_TEMPERATURA_BASE;
    var sinal := (temperaturaRegion >= 0) ? 1 : -1;
    var porcentagem := Abs(temperaturaRegion) * 0.4 / 100;
    return (1 - sinal * porcentagem);
  endfunction

  function _CalculaLimiteEstimuloInferior(this)

    var calculo := -this.estimuloLimite / this.fatorMudancaEstado;

    if(this.estado <= ESTADO_PROPAGACAO::LATENTE)
      return Cint(calculo / 2);
    endif
      return Cint(calculo);
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
        this.estado > ESTADO_PROPAGACAO::LATENTE
        && !this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO);
  endfunction

  function PegaCatalogoDano(this)
    var catalogoDano := this.PegaPropPropagacao().marcadorProps.catalogoDano;
    if(catalogoDano)
      return catalogoDano;
    else
      return this.catalogoDanoBase;
    endif 
  endfunction
  
  function TocaSFXCaracteristico(this)
    if(this.estado >= ESTADO_PROPAGACAO::PRINCIPIO)
      var efeitosPersonalizados := this.PegaPropPropagacao().efeitosPersonalizados[cstr(this.estado)];
      if(efeitosPersonalizados.sfx != -1)
        PlaySoundEffect( this.esteItemMarcador,efeitosPersonalizados.sfx ? efeitosPersonalizados.sfx : SFX_1DE);
      endif
    endif
  endfunction

endclass
