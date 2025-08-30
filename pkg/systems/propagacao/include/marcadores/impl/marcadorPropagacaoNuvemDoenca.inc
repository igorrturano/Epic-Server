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

class MarcadorPropagacaoIncendio( MarcadorPropagacaoBase)
  
  function MarcadorPropagacaoIncendio(this, esteItemMarcador, propPropagacao, momentoNascimento)
    super(esteItemMarcador,propPropagacao, momentoNascimento); 

    esteItemMarcador.usescript := ":propagacao:dblClickMarcadorIncendio";

    var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(propPropagacao.origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
    this.resourceConfig := ReadConfigFile(":crafting:config/resourceList");
    this.estimuloLimite := propagacaoConfig["incendio"].estimuloLimite;
    this.ciclosDormenteLimite := propagacaoConfig["incendio"].ciclosDormenteLimite;
    this.ciclosDormente := 0;
    this.tileProspectado := this.Prospectar();
  endfunction

  function EfetuarCiclo(this)
    if(!this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO))
 
    endif
  endfunction

  function Iniciar(this)
    this.MarcarAdjacentes();
    if(!this.tileProspectado)
      this.tileProspectado := this.Prospectar();
    endif
  endfunction

  function InjetaCombustivel(this, deveResetarEstado)
    if(deveResetarEstado)
      this.AtribuiEstado(this.PegaPropPropagacao().estadoInicial);
    endif
    this.mobileOrigemIncendio := SystemFindObjectBySerial( cint(this.PegaPropPropagacao().origem), SYSFIND_SEARCH_OFFLINE_MOBILES);
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
    
    this.medidorEstimulo += (this._DeveInverterEstimulo(valor,isRemetenteAcumulando) ?  valor * -1 : valor);

    this.AtualizaEstado();
  endfunction

  function DevePropagar(this)
    return 
        this.estado > ESTADO_PROPAGACAO::DORMENTE
        && !this.IsEstadoControle(CONTROLE_PROPAGACAO::MORTO);
  endfunction

  function Propagar(this)
    return this.MarcarAdjacentes();
  endfunction

endclass
