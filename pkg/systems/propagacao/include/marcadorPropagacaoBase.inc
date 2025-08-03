include ":propagacao:include/propagacao";


class MarcadorPropagacaoBase()
	function MarcadorPropagacaoBase( this, esteItemMarcador, propPropagacao)
		this.esteItemMarcador := esteItemMarcador;
		this.propPropagacao := propPropagacao;
		this.marcadoresAdjacentes := dictionary;
		this.estado := propPropagacao.estadoInicial;
		this.ciclosDesdeUltimoEstimulo := 0;
		this.isMarcadorMorto := false;
		this.isAcumulando := false;
		this.fatorMudancaEstado := 1; //divede o medidor de estimulo. limite
		this.medidorEstimulo := 0; // exemplo vai de -50 a + 50 
	endfunction

  	//@abstract void EfetuarCiclo()
  	//@abstract void RecebeEstimulo( valor )
  	//@abstract void EnviaEstimulo( valor )

	function Iniciar()
	endfunction

	function MarcarAdjacentes(this)
		//Print("*PROPAGACAO* Marcando adjacentes a {} ".format(this.esteItemMarcador.serial));
		for deslocamentoX := -1 to 1
			sleepms(2);
			for deslocamentoY := -1 to 1
				sleepms(2);
				if(deslocamentoX == 0 && deslocamentoY == 0) //pula se for a coordenada onde o marcador esta.
					continue;
				endif
				var adjacenteX := this.esteItemMarcador.x + deslocamentoX;
				var adjacenteY := this.esteItemMarcador.y + deslocamentoY;
				var adjacenteZ := GetStandingHeight( adjacenteX,
					adjacenteY,
					this.esteItemMarcador.z,
					this.esteItemMarcador.realm).z;


				var chaveDicionario := CStr(adjacenteX) + CStr(adjacenteY) + CStr(adjacenteZ);
				var  marcadorPropagacaoEncontrado := this.marcadoresAdjacentes[chaveDicionario];
				
				if(!marcadorPropagacaoEncontrado)
					marcadorPropagacaoEncontrado := EncontraMarcadorPropagacao(cint(adjacenteX), cint(adjacenteY), cint(adjacenteZ), this.esteItemMarcador.realm);
					if(DeveCriarMarcadorPropagacaoAdjacente(marcadorPropagacaoEncontrado,this.propPropagacao.geracao,this.estado))
						
						//Print("*PROPAGACAO* Marcador de Propagação adjacente criado com chave: {} ".format(chaveDicionario));
						marcadorPropagacaoEncontrado := CriaMarcadorPropagacaoAdjacente(
																this.esteItemMarcador,
																cint(adjacenteX), 
																cint(adjacenteY), 
																cint(adjacenteZ),
																this.esteItemMarcador.realm, 
																this.propPropagacao);
					else
						//Print("*PROPAGACAO* Marcador de Propagação adjacente localizado ou não propagado com chave: {} ".format(chaveDicionario));
					endif
				else
					//Print("*PROPAGACAO* Marcador já registrado na chave: {}".format(chaveDicionario));
					continue;
				endif
				
				if(marcadorPropagacaoEncontrado)
					this.marcadoresAdjacentes.insert(chaveDicionario, marcadorPropagacaoEncontrado);
					//Print("*PROPAGACAO* Marcador de Propagação adjacente criado com chave: {} - Serial: {} ".format(chaveDicionario, marcadorPropagacaoEncontrado.serial));
				else
					//Print("*PROPAGACAO* Marcador de Propagação adjacente não propagado na chave: {}".format(chaveDicionario));
				endif
				
			endfor
		endfor
	endfunction

	function DeveCriarMarcadorPropagacaoAdjacente(marcadorPropagacaoEncontrado, geracao, estado)
		//Print( " ZXXAAX --- " + marcadorPropagacaoEncontrado + " - " + geracao +" - " +estado +  " - " + ESTADO_PROPAGACAO_DORMENTE + " - " + ESTADO_PROPAGACAO_LATENTE);
		return !marcadorPropagacaoEncontrado 
				&& !IsLimiteGeracaoAlcancado(geracao) 
				&& estado != ESTADO_PROPAGACAO_DORMENTE
				&& estado != ESTADO_PROPAGACAO_LATENTE;
		
	endfunction

	function Prospectar(this)
		//Print("*PROPAGACAO* Marcador de Propagação: {}  ** Prospectando.".format(this.esteItemMarcador.serial));
		return TileProspectado(
			ListItemsAtLocation( this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z, this.esteItemMarcador.realm),
			ListStaticsNearLocation(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z,0,0, this.esteItemMarcador.realm ),
			ListMobilesNearLocationEx(this.esteItemMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.z,LISTEX_FLAG_GHOST, this.esteItemMarcador.realm),
			GetMapInfo( this.esteesteItemMarcadorMarcador.x, this.esteItemMarcador.y, this.esteItemMarcador.realm )
		);
	endfunction

	function IncrementaCiclosDesdeUltimoEstimulo(this)
		this.ciclosDesdeUltimoEstimulo++;
	endfunction

	function ZeraCiclosDesdeUltimoEstimuloPositivo(this)
		this.ciclosDesdeUltimoEstimulo := 0;
	endfunction

	function IsMarcadorMorto(this)
		return this.isMarcadorMorto;
	endfunction

endclass

class TileProspectado()
	function TileProspectado( this, itensEncontrados, estaticosEncontrados, mobilesEncontrados, chaoInfo)
		this.itensEncontrados := itensEncontrados;
		this.estaticosEncontrados := estaticosEncontrados;
		this.mobilesEncontrados := mobilesEncontrados;
		this.chaoInfo := chaoInfo;
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

	function PegaChaoInfo(this)
		return this.chaoInfo;
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

	function AtribuiChaoInfo(this, chaoInfo)
		this.chaoInfo := chaoInfo;
	endfunction

endclass