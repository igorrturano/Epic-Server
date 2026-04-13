
include ":propagacao:include/propagacao";
include ":propagacao:include/combustiveis/combustivelIncendio";

class CombustivelTerreno( CombustivelIncendio )
  function CombustivelTerreno(this)
    var config := ReadConfigFile(":propagacao:config/terrenosInflamaveis");
    super(0,COMBUSTIVEIS::TERRENO,0,config["geral"]);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    if(!this.isExaurido)
      var infoTerreno := tileProspectado.PegaTerrenoInfo();

      if(itemMarcador.z != infoTerreno.z)
        return false;
      endif
      
      if(this.EncheCombustivelTile(this.ExtraiInfoTile(this.inflamaveisConfig[cstr(infoTerreno.landtile)])))
        return this.TemCombustivel();
      endif
      return false;
    endif
    return false;
  endfunction

endclass