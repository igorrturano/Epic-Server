
use util;
include ":propagacao:include/propagacao";
include ":propagacao:include/marcadores/IMarcadorPropagacao";
include ":propagacao:include/tiles/TileScanner";
include "/include/epicPropNames";

class MarcadorPropagacaoBase(IMarcadorPropagacao)
	function MarcadorPropagacaoBase( this, esteItemMarcador, propPropagacao, momentoNascimento, descansoMinimo := 0)
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
		this.marcadorOrigem := propPropagacao.marcadorOrigem;
		this.esteItemMarcador := esteItemMarcador;
		this.marcadoresAdjacentes := dictionary;
		this.estado := propPropagacao.estadoInicial;
		this.ciclosDesdeUltimoEstimulo := 0;
		this.ciclos := 0;
		this.isAcumulando := false;
		this.estadoControle := CONTROLE_PROPAGACAO::ATIVO;
		this.fatorMudancaEstado := 1; //divede o medidor de estimulo. limite
		this.medidorEstimulo := 0; // exemplo: vai de -50 a + 50
		this.descasoMinimo := descansoMinimo ? descansoMinimo : cint(propagacaoConfig["sistema"].descansoMinimo);
		this.descansoControleDormente := this.descasoMinimo * cint(propagacaoConfig["sistema"].multiplicadorDescansoDormente);
		this.limitePorOrigem := cint(propagacaoConfig["sistema"].limitePorOrigem);
		this.momentoNascimento := momentoNascimento;
	endfunction

	uninit function Iniciar(this);
	uninit function EfetuarCiclo(this);
	uninit function EnviaEstimulo(this, valorEstimulo);
	uninit function RecebeEstimulo(this, valorEstimulo, isAcumulando);
	uninit function InjetaCombustivel(this, deveResetarEstado);
	uninit function GanchoAntesAtualizaEstado(this, estado);
	uninit function AtualizaComportamentoMarcador(this);

	function Propagar( this )
	  return this.MarcarAdjacentes();
	endfunction

	function AtualizaEstado(this, estado, forcaAtualizar := false)
	  var novoEstado := this.GanchoAntesAtualizaEstado(estado);
	  var valorAntigo := this.estado;
	  var prop := this.PegaPropPropagacao();

	  novoEstado := TypeOfInt(novoEstado) == OT_LONG? novoEstado : estado;
	  
      prop.lerEstadoAtual := novoEstado;
      SetObjProperty(this.esteItemMarcador,PROPNAME_PROP_PROPAGACAO,prop);
	  this.estado := novoEstado;

	  if(valorAntigo != this.estado || forcaAtualizar)
	  	this.AtualizaComportamentoMarcador();
	  endif
	endfunction

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
					Sleepms(RandomIntMinMax(25,90));//Marreta pra diminuir a condição de corrida nr 1. kkkk
					var posicaoMarcador := this.BuscaPosicaoMarcador(
													adjacenteX, 
													adjacenteY, 
													this.esteItemMarcador.z, 
													this.esteItemMarcador.realm,
													this.PegaPropPropagacao().tipoPropagacao
												);
					marcadorPropagacaoEncontrado := posicaoMarcador.marcadorEncontrado;

					if(this.DeveCriarMarcadorPropagacaoAdjacente(
															marcadorPropagacaoEncontrado,
															this.geracao,
															this.estado
														))

						var propPropagacao := this.PegaPropPropagacao();
						propPropagacao.isMarcadorCoberto := posicaoMarcador.temTeto;
						
						marcadorPropagacaoEncontrado := CriaMarcadorPropagacaoAdjacente(
																this.esteItemMarcador,
																adjacenteX, 
																adjacenteY, 
																posicaoMarcador.zMaisProximo,
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

	function BuscaPosicaoMarcador(this,x, y, z, realm, tipoPropagacao)
		
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

	function DeveCriarMarcadorPropagacaoAdjacente(this, marcadorPropagacaoEncontrado, geracao, estado)
		return !marcadorPropagacaoEncontrado 
				&& !IsLimiteGeracaoAlcancado(geracao) 
				&& estado != ESTADO_PROPAGACAO::DORMENTE
				&& estado != ESTADO_PROPAGACAO::LATENTE
				&& !this.IsLimiteGlobalAlcancado();
		
	endfunction

	function IsLimiteGlobalAlcancado(this)
		return this.limitePorOrigem <= LerLimitadoraGlobal(this.PegaPropPropagacao().origem);
	endfunction

	function Prospectar(this, opcoes := array, tileProspectado := 0)
		var tileScanner := TileScanner(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z,this.esteItemMarcador.realm); 
		if(opcoes && opcoes.size() > 0)
			foreach opcao in (opcoes)
				Sleepms(2);
				tileScanner.AdicionaManipulador(opcao);
			endforeach

			if(tileProspectado)
				tileScanner.AtribuiTileProspectado(tileProspectado);
			endif

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

