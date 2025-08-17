
use util;
include ":propagacao:include/propagacao";

enum tiposProspeccao
	PROPAGACAO_PROSPECCAO_INICIAL,
	PROPAGACAO_PROSPECCAO_CICLO,
endenum

const PROPAGACAO_TEMPERATURA_BASE := 20;
class MarcadorPropagacaoBase()
	function MarcadorPropagacaoBase( this, esteItemMarcador, propPropagacao)
		
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
		this.esteItemMarcador := esteItemMarcador;
		this.marcadoresAdjacentes := dictionary;
		this.estado := propPropagacao.estadoInicial;
		this.ciclosDesdeUltimoEstimulo := 0;
		this.isMarcadorMorto := false;
		this.isAcumulando := false;
		this.fatorMudancaEstado := 1; //divede o medidor de estimulo. limite
		this.medidorEstimulo := 0; // exemplo: vai de -50 a + 50 
	endfunction

  	//@abstract void EfetuarCiclo()
  	//@abstract void RecebeEstimulo( valor )
  	//@abstract void EnviaEstimulo( valor )

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
					Sleepms(RandomIntMinMax(25,120));//Marreta pra diminuir a condição de corrida nr 1. kkkk
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
															this.PegaPropPropagacao().geracao,
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
		var tileProspectado := TileProspectado(
									ListItemsNearLocation( x, y, LIST_IGNORE_Z , 0,realm),
									ListStaticsNearLocation(x, y, LIST_IGNORE_Z ,0,0,realm ),
									0,
									GetMapInfo( x, y, realm )
								);
		
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
				&& estado != ESTADO_PROPAGACAO_DORMENTE
				&& estado != ESTADO_PROPAGACAO_LATENTE;
		
	endfunction

	function Prospectar(this)

		return TileProspectado(
			ListItemsNearLocation( this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z, 0,this.esteItemMarcador.realm),
			ListStaticsNearLocation(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z,0,0, this.esteItemMarcador.realm ),
			ListMobilesNearLocationEx(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z, 0,LISTEX_FLAG_NORMAL + LISTEX_FLAG_HIDDEN , this.esteItemMarcador.realm),
			GetMapInfo( this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.realm )
		);
	endfunction

	function IncrementaCiclosDesdeUltimoEstimulo(this)
		if(this.estado == ESTADO_PROPAGACAO_DORMENTE)
			this.ciclosDormente++;
		else
			this.ciclosDesdeUltimoEstimulo++;
		endif
	endfunction

	function ZeraCiclosDesdeUltimoEstimuloPositivo(this)
		this.ciclosDesdeUltimoEstimulo := 0;
		this.ciclosDormente := 0;
	endfunction

	function IsMarcadorMorto(this)
		return this.isMarcadorMorto;
	endfunction

	function PegaPropPropagacao(this)
		return GetObjProperty(this.esteItemMarcador,PROPNAME_PROP_PROPAGACAO);
	endfunction

endclass

class TileProspectado()
	function TileProspectado( this, itensEncontrados, estaticosEncontrados, mobilesEncontrados, terrenoInfo)
		this.itensEncontrados := itensEncontrados;
		this.estaticosEncontrados := estaticosEncontrados;
		this.mobilesEncontrados := mobilesEncontrados;
		this.terrenoInfo := terrenoInfo;
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