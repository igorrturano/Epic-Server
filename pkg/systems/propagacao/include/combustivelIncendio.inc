

include ":propagacao:include/propagacao";

class CombustivelIncendio()
  function CombustivelIncendio(this, combustivel, tipo, fatorInflamavel)
    this.propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
    this.tipo := tipo;
    this.listaInflamaveis := GetConfigStringArray(this.propagacaoConfig["incendio"],tipo);
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

  function ConfiguraCombustivel(this, valor)
    this.combustivel := valor;
  endfunction

  function ExtraiInfoTile(this, tile)
     var splited := SplitWords(tile);
      
     return struct {objtype := splited[1], combustivel := splited[2], fator := splited[3]};
  endfunction

  function EncheCombustivel(this,objtypeItemInflamavel, infoTile)
    
    if(cstr(objtypeItemInflamavel) == infoTile.objtype)
      this.combustivel := cint(infoTile.combustivel);
      this.fatorInflamavel := cint(infoTile.fator);
      return true;
    else
      return false;
    endif
  endfunction

endclass

class CombustivelEstaticos( CombustivelIncendio )
  function CombustivelEstaticos(this)
    super(0,"staticInflamavel",0);
  endfunction

  function ProcuraCombustivel(this,itemMarcador, tileProspectado)
    if(!this.isExaurido)
      foreach estatico in (tileProspectado.PegaEstaticosEncontrados())
        Sleepms(2);
        var zAbsoluto := PegaAbsoluta(itemMarcador.z, estatico.z);
        var isAoAlcance :=  zAbsoluto <= 19;
        if(isAoAlcance)
          foreach staticoInflamavel in (this.listaInflamaveis)
            Sleepms(2);
            if(this.EncheCombustivel(estatico.objtype,this.ExtraiInfoTile(staticoInflamavel)))
              return true;
            endif
          endforeach
        endif
      endforeach
    endif
    return false;
  endfunction
endclass

class CombustivelTerreno( CombustivelIncendio )
  function CombustivelTerreno(this)
    super(0,"terrenoInflamavel",0);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    if(!this.isExaurido)
      var infoTerreno := tileProspectado.PegaTerrenoInfo();
      if(itemMarcador.z != infoTerreno.z)
        return false;
      endif
      
      foreach terrenoInflamavel in (this.listaInflamaveis)
        Sleepms(2);
        if(this.EncheCombustivel(infoTerreno.landtile,this.ExtraiInfoTile(terrenoInflamavel)))
          return true;
        endif
      endforeach
    endif
    return false;
  endfunction

endclass

