
include ":propagacao:include/propagacao";
include ":propagacao:include/combustiveis/combustivelIncendio";

class CombustivelMagico( CombustivelIncendio )
  function CombustivelMagico(this)
    super(0,COMBUSTIVEIS::MAGICO,0, 0);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    tileProspectado == 0;
    if(!this.isExaurido)
      var magico := GetObjProperty(itemMarcador,"propPropagacao").combustivelExterno;
      if(magico)
        this.EncheCombustivel( magico.combustivel,magico.fator ? magico.fator : 1);
        return this.TemCombustivel();
      endif
    endif
    return false;
  endfunction

endclass