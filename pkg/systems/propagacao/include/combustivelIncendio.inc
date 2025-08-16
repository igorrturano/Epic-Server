

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
     
     return struct {
                  objtype := splited[1], 
                  combustivel := this.PegaTier(splited[2]), 
                  fator := this.PegaTier(splited[3])
                };
  endfunction

  function PegaTier(this, valor)
    var valorConfig := GetConfigInt( this.propagacaoConfig["incendio"], valor );

    return valorConfig ? valorConfig : valor;
  endfunction


  function EncheCombustivelTile(this,objtypeItemInflamavel, infoTile)
    
    if(cstr(objtypeItemInflamavel) == infoTile.objtype)
      this.EncheCombustivel(infoTile.combustivel,infoTile.fator);
      return true;
    else
      return false;
    endif
  endfunction

   function EncheCombustivel(this,combustivel, fator)
    this.combustivel := cint(combustivel);
    this.fatorInflamavel := cint(fator);;
  endfunction

endclass

class CombustivelEstaticos( CombustivelIncendio )
  function CombustivelEstaticos(this)
    super(0,PROPAGACAO_COMBUSTIVEL_ESTATICOS,0);
  endfunction

  function ProcuraCombustivel(this,itemMarcador, tileProspectado)
    if(!this.isExaurido)
      foreach estatico in (tileProspectado.PegaEstaticosEncontrados())
        Sleepms(2);
        var zAbsoluto := PegaAbsoluta(itemMarcador.z, estatico.z);
        var isAoAlcance :=  zAbsoluto <= 5;
        if(isAoAlcance)
          foreach staticoInflamavel in (this.listaInflamaveis)
            Sleepms(2);
            if(this.EncheCombustivelTile(estatico.objtype,this.ExtraiInfoTile(staticoInflamavel)))
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
    super(0,PROPAGACAO_COMBUSTIVEL_TERRENO,0);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    if(!this.isExaurido)
      var infoTerreno := tileProspectado.PegaTerrenoInfo();
      if(itemMarcador.z != infoTerreno.z)
        return false;
      endif
      
      foreach terrenoInflamavel in (this.listaInflamaveis)
        Sleepms(2);
        if(this.EncheCombustivelTile(infoTerreno.landtile,this.ExtraiInfoTile(terrenoInflamavel)))
          return true;
        endif
      endforeach
    endif
    return false;
  endfunction

endclass

class CombustivelMagico( CombustivelIncendio )
  function CombustivelMagico(this)
    super(0,PROPAGACAO_COMBUSTIVEL_MAGICO,0);
  endfunction

  function ProcuraCombustivel(this, itemMarcador , tileProspectado)
    if(!this.isExaurido)
      var magico := GetObjProperty(itemMarcador,"propPropagacao").combustivelMagico;
      if(magico)
        this.EncheCombustivel( magico.combustivel,magico.fator ? magico.fator : 1);
        return this.TemCombustivel();
      endif
    endif
    return false;
  endfunction

endclass

