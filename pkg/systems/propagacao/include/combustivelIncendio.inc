

include ":propagacao:include/propagacao";

class CombustivelIncendio()
  function CombustivelIncendio(this, combustivel, tipo, fatorInflamavel, inflamaveisConfig)
    this.propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.tipo := tipo;
    this.inflamaveisConfig := inflamaveisConfig;
    this.combustivel := combustivel;
    this.isExaurido := false;
    this.fatorInflamavel := fatorInflamavel;
  endfunction

  //@abstract bool ProcuraCombustivel(ItemRef itemMarcador,TileProspectado tileProspectado)

  function ConsomeCombustivel(this, valor)
    this.combustivel -= valor;
    if(!this.TemCombustivel())
      this._MarcaExaurido();
    endif
  endfunction

  function TemCombustivel(this)
    return this.combustivel > 0;
  endfunction

  function _MarcaExaurido(this)
    this.isExaurido := true;
  endfunction

  function ExtraiInfoTile(this, tile)
      if(tile)
        var splited := SplitWords(tile);
        return struct {
                      combustivel := this.PegaTier(splited[1]), 
                      fator := this.PegaTier(splited[2])
                    };
      else
       return 0;
      endif
  endfunction

  function PegaTier(this, valor)
    var valorConfig := GetConfigInt( this.propagacaoConfig["incendio"], valor );

    return valorConfig ? valorConfig : valor;
  endfunction


  function EncheCombustivelTile(this,infoTile)
    
    if(infoTile)
      this.EncheCombustivel(infoTile.combustivel,infoTile.fator);
      return true;
    else
      return false;
    endif
  endfunction

   function EncheCombustivel(this,combustivel, fator)
    this.combustivel := cint(combustivel);
    this.fatorInflamavel := cint(fator ? fator : 1);
  endfunction

endclass

class CombustivelEstaticos( CombustivelIncendio )
  function CombustivelEstaticos(this)
    var config := ReadConfigFile("config/estaticosInflamaveis");
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

class CombustivelTerreno( CombustivelIncendio )
  function CombustivelTerreno(this)
    var config := ReadConfigFile("config/terrenosInflamaveis");
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

class CombustivelMagico( CombustivelIncendio )
  function CombustivelMagico(this)
    super(0,COMBUSTIVEIS::MAGICO,0, 0);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    tileProspectado == 0;
    if(!this.isExaurido)
      var magico := GetObjProperty(itemMarcador,"propPropagacao").combustivelMagico;
      if(magico && magico != 0 && TypeOfInt(magico) != OT_STRUCT)
      endif
      if(magico)
        this.EncheCombustivel( magico.combustivel,magico.fator ? magico.fator : 1);
        return this.TemCombustivel();
      endif
    endif
    return false;
  endfunction

endclass

