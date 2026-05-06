

include ":propagacao:include/propagacao";
include ":propagacao:include/combustiveis/combustivelIncendio";

class CombustivelEstaticos( CombustivelIncendio )
  function CombustivelEstaticos(this)
    var config := ReadConfigFile(":propagacao:config/estaticosInflamaveis");
    super(0,COMBUSTIVEIS::ESTATICOS,0,config["geral"]);
  endfunction

  function ProcuraCombustivel(this,itemMarcador, tileProspectado)
    if(!this.isExaurido)
      foreach estatico in (tileProspectado.PegaEstaticosEncontrados())
        Sleepms(2);
        var zAbsoluto := PegaAbsoluta(itemMarcador.z, estatico.z);
        var isAoAlcance :=  zAbsoluto <= 5;

        if(isAoAlcance)
          if(this.EncheCombustivelTile(this.ExtraiInfoTile(this.inflamaveisConfig[cstr(estatico.objtype)])))
            return this.TemCombustivel();
          endif
        endif
      endforeach
    endif
    return false;
  endfunction
endclass