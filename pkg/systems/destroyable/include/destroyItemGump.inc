include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";

const ALTURA_LINHA := 20;
const QUALTIDADE_COLUNAS := 12;

function CriaTabelaGump(x,y,larguraTotal,dados,byref gump,paginaArray := 0,opcoesTabela := struct,opcoesColunas := dictionary)

	if(dados.size() > 0 )
		var exemplo := dados[1];
		var divisorLargura := 0;
		var nomesColunas := array;
		
		if(TypeOf(exemplo) == "Struct")
			nomesColunas := exemplo.keys();
		endif

		var larguraGrid := larguraTotal / QUALTIDADE_COLUNAS;
		var colunas := array;
		var colunasVerificadas := dictionary;
		var larguraContabilizada := 0;
		var qtdeColConfiguradas := 0;

		foreach nome in nomesColunas
			var coluna := struct;
			var chaveColunaAtual := nome;
			var chaveColunaTrocada := nome;
			var menorValorOrdem := nomesColunas.size() * 2;

			foreach ordenado in nomesColunas // pega o proximo de menor ordem nas opções informadas.
				if(colunasVerificadas[ordenado])
					continue;
				endif
				if(opcoesColunas[ordenado])
					if(!opcoesColunas[ordenado].ordem); //ordem não é obrigatória pra ter opcoes de coluna.
						continue;
					endif
					if(opcoesColunas[ordenado].ordem < menorValorOrdem)
						menorValorOrdem := opcoesColunas[ordenado].ordem;
						chaveColunaTrocada := ordenado;
					endif
				endif
				Sleepms(1);
			endforeach

			if(colunasVerificadas[chaveColunaTrocada])
				continue;
			endif

			if(opcoesColunas[chaveColunaTrocada])
				qtdeColConfiguradas += 1;
				coluna.+titulo := opcoesColunas[chaveColunaTrocada].titulo;
				coluna.+chaveReal := chaveColunaTrocada;
				coluna.+largura := opcoesColunas[chaveColunaTrocada].largura;
				coluna.+filtro := opcoesColunas[chaveColunaTrocada].filtro;

				larguraContabilizada := larguraContabilizada + coluna.largura;
			else
				coluna.+titulo := chaveColunaTrocada;
				coluna.+chaveReal := chaveColunaTrocada;
			endif

			colunasVerificadas.insert(chaveColunaTrocada,_nome_iter);
			colunas.append(coluna);

			if(chaveColunaTrocada != chaveColunaAtual) //Move para o final caso tenha perdido seu lugar para outra chave.
				nomesColunas.append(chaveColunaAtual);
			endif
			
			Sleepms(2);
		endforeach

		var qtdeColRestantes := QUALTIDADE_COLUNAS - larguraContabilizada;
		
		//cabeçalho
		var larguraUtilizada := 0;
		foreach coluna in colunas
			if(!coluna.largura)// Não tem largura definada nas opções da coluna? vamos pegar uma parte da largura restante.
				var qtdeColNaoConfiguradas := colunas.size() - qtdeColConfiguradas;
				var largura := CDbl(qtdeColRestantes) / CDbl(qtdeColNaoConfiguradas);
				coluna.+largura := largura; 
			endif
			GFResizePic(gump, x + larguraUtilizada, y,9300, larguraGrid * coluna.largura, ALTURA_LINHA);
			GFTextLine(gump, x + larguraUtilizada + (larguraGrid * coluna.largura / 2) - (Len(coluna.titulo) * 3), y, opcoesTabela.corTituloId? opcoesTabela.corTituloId: 0, coluna.titulo);
			larguraUtilizada := larguraUtilizada + larguraGrid * coluna.largura;
			Sleepms(2);
		endforeach

		//tabela em si
		var indice := paginaArray * opcoesTabela.qtdeLimiteLinha + 1;
		var indiceLinha := 1;
		for i := indice to  indice + (opcoesTabela.qtdeLimiteLinha - 1 )
			if(i > dados.size())
				break;
			endif
			larguraUtilizada := 0;
			foreach coluna in colunas
				GFResizePic(gump, x + larguraUtilizada, y + ALTURA_LINHA * indiceLinha,opcoesTabela.fundoId? opcoesTabela.fundoId:9300, larguraGrid * coluna.largura, ALTURA_LINHA);
				GFTextLine(gump, x + larguraUtilizada + 5, y + ALTURA_LINHA * indiceLinha, opcoesTabela.corDadoId? opcoesTabela.corDadoId: 89, dados[i][coluna.chaveReal]);
				larguraUtilizada := larguraUtilizada + larguraGrid * coluna.largura;
				Sleepms(2);
			endforeach
			indiceLinha += 1; 
			Sleepms(2);
		endfor
	endif

	return gump;
endfunction;