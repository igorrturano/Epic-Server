use util;
include ":propagacao:include/propagacao";
include "/include/epicPropNames";

class TileScanner()
	function TileScanner(this,x,y,z,realm)
		this.x := x;
		this.y := y;
		this.z := z;
		this.realm := realm;
		this.manipuladores := dictionary;
		this.tileProspectado := 0;
	endfunction

	function AtribuiTileProspectado(this, tileProspectado)
		this.tileProspectado := tileProspectado;

		return this;
	endfunction

	function AdicionaManipulador(this,  tipo, deveIgnorarZ := false)
		case(tipo)
			TILE_SCANNER::ITENS:
				this.manipuladores.insert(tipo,ProspectoItensManipulador(this.x,this.y,this.z,this.realm, deveIgnorarZ));
			TILE_SCANNER::STATICOS:
				this.manipuladores.insert(tipo,ProspectoStaticosManipulador(this.x,this.y,this.z,this.realm, deveIgnorarZ));
			TILE_SCANNER::MOBILES:
				this.manipuladores.insert(tipo,ProspectoMobilesManipulador(this.x,this.y,this.z,this.realm, deveIgnorarZ));
			TILE_SCANNER::TERRENO:
				this.manipuladores.insert(tipo,ProspectoTerrenoManipulador(this.x,this.y,this.z,this.realm));
		endcase

		return this;
	endfunction

	function RemoveManipulador(this,  tipo)
		this.manipuladores.erase(tipo);
		return this;
	endfunction

	function Prospectar(this)
		if(!this.tileProspectado)
			this.tileProspectado := TileProspectado();
		endif

		foreach manipulador in (this.manipuladores)
			Sleepms(2);
			manipulador.Executar(this.tileProspectado);
		endforeach

		return this.tileProspectado;
	endfunction

endclass

class IManipuladorTileScanner()
	uninit function Executar(this, byref tileProspectado);
endclass

class ProspectoItensManipulador(IManipuladorTileScanner)
	function ProspectoItensManipulador(this,x,y,z,realm, deveIgnorarZ)
		this.x := x;
		this.y := y;
		this.z := deveIgnorarZ ? LIST_IGNORE_Z : z;
		this.realm := realm;
		this.deveIgnorarZ := deveIgnorarZ;
	endfunction

	function Executar(this,byref tileProspectado)
		tileProspectado.AtribuiItensEncontrados(ListItemsNearLocation( this.x, this.y, this.z, 0,this.realm));
	endfunction
endclass

class ProspectoStaticosManipulador(IManipuladorTileScanner)
	function ProspectoStaticosManipulador(this,x,y,z,realm, deveIgnorarZ)
		this.x := x;
		this.y := y;
		this.z := deveIgnorarZ ? LIST_IGNORE_Z : z;
		this.realm := realm;
	endfunction

	function Executar(this,byref tileProspectado)
		tileProspectado.AtribuiEstaticosEncontrados(ListStaticsNearLocation(this.x, this.y, this.z,0,0, this.realm ));
	endfunction
endclass

class ProspectoMobilesManipulador(IManipuladorTileScanner)
	function ProspectoMobilesManipulador(this,x,y,z,realm, deveIgnorarZ)
		this.x := x;
		this.y := y;
		this.z := deveIgnorarZ ? LIST_IGNORE_Z : z;
		this.realm := realm;
		this.deveIgnorarZ := deveIgnorarZ;
	endfunction

	function Executar(this,byref tileProspectado)
		tileProspectado.AtribuiMobilesEncontrados(ListMobilesNearLocationEx(this.x, this.y, this.z, 0,LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN , this.realm));
	endfunction
endclass

class ProspectoTerrenoManipulador(IManipuladorTileScanner)
	function ProspectoTerrenoManipulador(this,x,y,z,realm)
		this.x := x;
		this.y := y;
		this.z := z;
		this.realm := realm;
	endfunction

	function Executar(this,byref tileProspectado)
		tileProspectado.AtribuiTerrenoInfo(GetMapInfo( this.x, this.y, this.realm ));
	endfunction
endclass

class TileProspectado()
	function TileProspectado( this)
		this.itensEncontrados := 0;
		this.estaticosEncontrados := 0;
		this.mobilesEncontrados := 0;
		this.terrenoInfo := 0;
	endfunction

	function PegaItensEncontrados(this)
		return this.itensEncontrados;
	endfunction

	function PegaEstaticosEncontrados(this)
		return this.estaticosEncontrados;
	endfunction

	function PegaMobilesEncontrados(this)
		return this.mobilesEncontrados;
	endfunction

	function PegaTerrenoInfo(this)
		return this.terrenoInfo;
	endfunction

	function AtribuiItensEncontrados(this, itensEncontrados)
		this.itensEncontrados := itensEncontrados;
	endfunction

	function AtribuiEstaticosEncontrados(this, estaticosEncontrados)
		this.estaticosEncontrados := estaticosEncontrados;
	endfunction

	function AtribuiMobilesEncontrados(this, mobilesEncontrados)
		this.mobilesEncontrados := mobilesEncontrados;
	endfunction

	function AtribuiTerrenoInfo(this, terrenoInfo)
		this.terrenoInfo := terrenoInfo;
	endfunction

endclass