

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

  function PegaFatorInflamavel(this)
    return this.fatorInflamavel;
  endfunction

endclass


