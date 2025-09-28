
use util;
include ":propagacao:include/propagacao";

class MarcadorPropagacaoBase()
	function MarcadorPropagacaoBase( this, esteItemMarcador, propPropagacao, momentoNascimento)
		var propagacaoConfig := ReadConfigFile(":propagacao:config/propagacao");
		if(propPropagacao.geracao == 0)
			
			var posicaoAvaliada := this.AvaliarPosicao(
												esteItemMarcador.x,
												esteItemMarcador.y,
												esteItemMarcador.z,
												esteItemMarcador.realm, 
												propPropagacao.tipoPropagacao);

			this.isMarcadorCoberto := posicaoAvaliada.temTeto;
		else
			this.isMarcadorCoberto := propPropagacao.isMarcadorCoberto;
		endif
		this.geracao := propPropagacao.geracao;
		this.esteItemMarcador := esteItemMarcador;
		this.marcadoresAdjacentes := dictionary;
		this.estado := propPropagacao.estadoInicial;
		this.ciclosDesdeUltimoEstimulo := 0;
		this.isAcumulando := false;
		this.estadoControle := CONTROLE_PROPAGACAO::ATIVO;
		this.fatorMudancaEstado := 1; //divede o medidor de estimulo. limite
		this.medidorEstimulo := 0; // exemplo: vai de -50 a + 50 
		this.descansoControleDormente := cint(propagacaoConfig["sistema"].descansoMinimo) * cint(propagacaoConfig["sistema"].multiplicadorDescansoDormente);
		this.momentoNascimento := momentoNascimento;
	endfunction

	//@abstract 
	function Iniciar() endfunction
	//@abstract 
	function EfetuarCiclo() endfunction
  	//@abstract 
	function RecebeEstimulo( valorEstimulo , isAcumulando ) endfunction
  	//@abstract 
	function EnviaEstimulo(valorEstimulo) endfunction
  	//@abstract 
	function InjetaCombustivel(deveResetarEstado) endfunction

	function MarcarAdjacentes(this)
		for deslocamentoX := -1 to 1
			sleepms(2);
			for deslocamentoY := -1 to 1
				sleepms(2);
				if(deslocamentoX == 0 && deslocamentoY == 0) //pula se for a coordenada onde o marcador esta.
					continue;
				endif
				var adjacenteX := Cint(this.esteItemMarcador.x + deslocamentoX);
				var adjacenteY := Cint(this.esteItemMarcador.y + deslocamentoY);

				var chaveDicionario := CStr(adjacenteX) + CStr(adjacenteY);
				var marcadorPropagacaoEncontrado := this.marcadoresAdjacentes[chaveDicionario];
				
				if(!marcadorPropagacaoEncontrado)
					Sleepms(RandomIntMinMax(25,60));//Marreta pra diminuir a condição de corrida nr 1. kkkk
					var posicaoAvaliada := this.AvaliarPosicao(
													adjacenteX, 
													adjacenteY, 
													this.esteItemMarcador.z, 
													this.esteItemMarcador.realm,
													this.PegaPropPropagacao().tipoPropagacao
												);
					marcadorPropagacaoEncontrado := posicaoAvaliada.marcadorEncontrado;

					if(DeveCriarMarcadorPropagacaoAdjacente(
															marcadorPropagacaoEncontrado,
															this.geracao,
															this.estado
														))

						var propPropagacao := this.PegaPropPropagacao();
						propPropagacao.isMarcadorCoberto := posicaoAvaliada.temTeto;
						
						marcadorPropagacaoEncontrado := CriaMarcadorPropagacaoAdjacente(
																this.esteItemMarcador,
																adjacenteX, 
																adjacenteY, 
																posicaoAvaliada.zMaisProximo,
																this.esteItemMarcador.realm, 
																propPropagacao);
					endif
				else
					continue;
				endif
				
				if(marcadorPropagacaoEncontrado)
					this.marcadoresAdjacentes.insert(chaveDicionario, marcadorPropagacaoEncontrado);
				endif
				
			endfor
		endfor
	endfunction

	function AvaliarPosicao(this,x, y, z, realm, tipoPropagacao)
		
		var posicaoAvaliada := struct {zMaisProximo := z, temTeto := false, marcadorEncontrado := 0};
		var menorDistancia := 4000;
		var alturaMaxima := 19;
		var tileProspectado := TileScanner(x,y,z,realm)
								.AdicionaManipulador(TILE_SCANNER::ITENS, true)
								.AdicionaManipulador(TILE_SCANNER::STATICOS, true)
								.AdicionaManipulador(TILE_SCANNER::TERRENO)
								.Prospectar();
		
		var mapa := tileProspectado.PegaTerrenoInfo();

		foreach item in (tileProspectado.PegaItensEncontrados())
			Sleepms(2);
			if(IsMarcadorPropagacaoValido(item,z,alturaMaxima,tipoPropagacao))
				posicaoAvaliada.marcadorEncontrado := item;
			endif
			this.ComparaAlturaMaisProxima( item,z, posicaoAvaliada.zMaisProximo,menorDistancia);

			var tilesInfo := BuscaConfiguracaoTiles(item);
			if(!posicaoAvaliada.temTeto)
				posicaoAvaliada.temTeto := this._IsMarcadorCoberto(z,item,tilesInfo,alturaMaxima);
			endif
		endforeach

		foreach static in (tileProspectado.PegaEstaticosEncontrados())
			Sleepms(2);
			this.ComparaAlturaMaisProxima( static,z, posicaoAvaliada.zMaisProximo,menorDistancia);

			var tilesInfo := BuscaConfiguracaoTiles(static);
			if(!posicaoAvaliada.temTeto)
				posicaoAvaliada.temTeto := this._IsMarcadorCoberto(z,static,tilesInfo,alturaMaxima);
			endif
		endforeach
		
		var distancia := PegaAbsoluta(mapa.z,z);
		if(menorDistancia > distancia)
			posicaoAvaliada.zMaisProximo := mapa.z;
			if(!posicaoAvaliada.temTeto)
				posicaoAvaliada.temTeto := mapa.z > z && mapa.z > z + alturaMaxima;
			endif
		endif

		return posicaoAvaliada;
	endfunction

	function _IsMarcadorCoberto(this, alturaMarcador, item, tilesInfo, alturaAndar)
		return item.z > alturaAndar + alturaMarcador && (tilesInfo.MoveLand || tilesInfo.BlockSight || tilesInfo.Blocking);
	endfunction

	function ComparaAlturaMaisProxima(this,item, zAlvo, byref zMaisProximo, byref menorDistancia)
		
		var tileInfo := BuscaConfiguracaoTiles(item); 
		var isSuperficie := tileInfo.MoveLand;
		var itemHeight := tileInfo.Height;
		var ztotal;

		if(isSuperficie)
			ztotal :=  item.z + itemHeight;
		else
			ztotal := item.z;
		endif

		var distancia := PegaAbsoluta(item.z,zAlvo);
		if(menorDistancia > distancia)
			zMaisProximo := ztotal;
			menorDistancia := distancia;
		endif
	endfunction

	function DeveCriarMarcadorPropagacaoAdjacente(marcadorPropagacaoEncontrado, geracao, estado)
		return !marcadorPropagacaoEncontrado 
				&& !IsLimiteGeracaoAlcancado(geracao) 
				&& estado != ESTADO_PROPAGACAO::DORMENTE
				&& estado != ESTADO_PROPAGACAO::LATENTE;
		
	endfunction

	function Prospectar(this, opcoes := array)
		var tileScanner := TileScanner(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z,this.esteItemMarcador.realm); 
		if(opcoes && opcoes.size() > 0)
			foreach opcao in (opcoes)
				Sleepms(2);
				tileScanner.AdicionaManipulador(opcao);
			endforeach

			return tileScanner.Prospectar();
		else
			return  tileScanner.AdicionaManipulador(TILE_SCANNER::ITENS)
						.AdicionaManipulador(TILE_SCANNER::STATICOS)
						.AdicionaManipulador(TILE_SCANNER::MOBILES)
						.AdicionaManipulador(TILE_SCANNER::TERRENO)
						.Prospectar();
		endif
	endfunction

	function IncrementaCiclosDesdeUltimoEstimulo(this)
		if(this.estado == ESTADO_PROPAGACAO::DORMENTE)
			this.ciclosDormente++;
		endif
		this.ciclosDesdeUltimoEstimulo++;
	endfunction

	function ZeraCiclosDesdeUltimoEstimuloPositivo(this)
		this.ciclosDesdeUltimoEstimulo := 0;
		this.ciclosDormente := 0;
	endfunction

	function IsEstadoControle(this, estadoControle)
		return this.estadoControle == estadoControle;
	endfunction

	function IsEstadoMarcador(this, estado)
		return this.estado == estado;
	endfunction

	function PegaPropPropagacao(this)
		return GetObjProperty(this.esteItemMarcador,PROPNAME_PROP_PROPAGACAO);
	endfunction

	function AtribuiEstadoControle(this, estadoControle)
		this.estadoControle :=  estadoControle;
	endfunction 

endclass

class TileScanner()
	function TileScanner(this,x,y,z,realm)
		this.x := x;
		this.y := y;
		this.z := z;
		this.realm := realm;
		this.manipuladores := dictionary;
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
		var tileProspectado := TileProspectado();
		foreach manipulador in (this.manipuladores)
			Sleepms(2);
			manipulador.Executar(tileProspectado);
		endforeach
		return tileProspectado;
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