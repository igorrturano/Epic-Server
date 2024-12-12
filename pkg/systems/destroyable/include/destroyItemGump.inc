include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";

const ALTURA_LINHA := 20;
const QUALTIDADE_COLUNAS := 12;

enum OPCOES_TABELA
	COR_TITULO_COLUNA := "corTituloId",
	COR_TITULO_TABELA := "corTituloTabId",
	COR_DADO := "corDadoId",
	QTD_LINHA_LIMITE := "qtdeLimiteLinha",
	GUMPIC_FUNDO := "fundoId",
	TITULO_TABELA := "tituloTabela",
	BOTAO_APERTADO := "botaoApertadoId",
	BOTAO_SOLTO := "botaoSoltoId",
	CHECKBOX_APERTADO := "checkBoxApertadoId",
	CHECKBOX_SOLTO := "checkBoxSoltoId",
	RECUO_EIXO_X_BOTAO := "recuoExidoXBtn",
	MOSTRAR_FUNDO := "isMostrarFundo",
	MAX_PAGINAS := "numeroMaxPaginas"
endenum

function CriaTabelaGump(x,y,larguraTotal,dados,byref gump,paginaTabela := 0,opcoesTabela := dictionary,opcoesColunas := dictionary, indiceBtnInicial := 0)

	if(dados.size() > 0 )
		//Customizando Tabela
		var opt_qtd_limite_linha :=  DeterminaOpcaoTabela(opcoesTabela,QTD_LINHA_LIMITE,20);
		var opt_gumpic_fundo :=  DeterminaOpcaoTabela(opcoesTabela,GUMPIC_FUNDO,9300);
		var opt_cor_titulo_coluna := DeterminaOpcaoTabela(opcoesTabela,COR_TITULO_COLUNA,0);
		var opt_cor_titulo_tabela := DeterminaOpcaoTabela(opcoesTabela,COR_TITULO_TABELA, "#593803");
		var opt_cor_dado := DeterminaOpcaoTabela(opcoesTabela,COR_DADO,89);
		var opt_botao_apertado := DeterminaOpcaoTabela(opcoesTabela,BOTAO_APERTADO,2511);
		var opt_botao_solto := DeterminaOpcaoTabela(opcoesTabela,BOTAO_SOLTO,2510);
		var opt_checkbox_apertado := DeterminaOpcaoTabela(opcoesTabela,CHECKBOX_APERTADO,2511);
		var opt_checkbox_solto := DeterminaOpcaoTabela(opcoesTabela,CHECKBOX_SOLTO,2510);
		var opt_recuo_eixox_btn := DeterminaOpcaoTabela(opcoesTabela,RECUO_EIXO_X_BOTAO,5);
		var opt_is_mostrar_fundo := DeterminaOpcaoTabela(opcoesTabela,MOSTRAR_FUNDO,true);

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

		if(opcoesTabela[TITULO_TABELA])
			if(opt_is_mostrar_fundo)
				GFResizePic(gump, x, y,opt_gumpic_fundo, larguraTotal, ALTURA_LINHA);
			endif

			GFHTMLArea( 
			gump,
			x + CentralizaEixoX(larguraTotal,opcoesTabela[TITULO_TABELA]),
			y,
			larguraTotal,
			ALTURA_LINHA,
			"<{}size={} color={}>{}</{}>".format(
					"BASEFONT",
					5,
					opt_cor_titulo_tabela,
					opcoesTabela[TITULO_TABELA],
					"BASEFONT"));

			y := y + ALTURA_LINHA;
		endif

		foreach coluna in colunas
			if(!coluna.largura)// Não tem largura definada nas opções da coluna? vamos pegar uma parte da largura restante.
				var qtdeColNaoConfiguradas := colunas.size() - qtdeColConfiguradas;
				var largura := CDbl(qtdeColRestantes) / CDbl(qtdeColNaoConfiguradas);
				coluna.+largura := largura; 
			endif
			if(opt_is_mostrar_fundo)
				GFResizePic(gump, x + larguraUtilizada, y,opt_gumpic_fundo, larguraGrid * coluna.largura, ALTURA_LINHA);
			endif

			GFTextLine(gump, x + larguraUtilizada + CentralizaEixoX(larguraGrid,coluna.titulo,coluna.largura), y, opt_cor_titulo_coluna, coluna.titulo);
			larguraUtilizada := larguraUtilizada + larguraGrid * coluna.largura;
			Sleepms(2);
		endforeach

		//tabela em si
		var indice := paginaTabela * opt_qtd_limite_linha + 1;
		var indiceLinha := 1;

		for linha := indice to  indice + (opt_qtd_limite_linha - 1 )
			if(linha > dados.size())
				break;
			endif
			larguraUtilizada := 0;
			foreach coluna in colunas
				if(opt_is_mostrar_fundo)
					GFResizePic(gump, x + larguraUtilizada, y + ALTURA_LINHA * indiceLinha, opt_gumpic_fundo, larguraGrid * coluna.largura, ALTURA_LINHA);
				endif
				
				var valorCelula :=  dados[linha][coluna.chaveReal];
				var valorBotao := valorCelula.valor ? valorCelula.valor: indiceLinha + indiceBtnInicial;
				if(valorCelula.isColunaBotao)
					GFAddButton( gump,
								x + larguraUtilizada + CentralizaEixoX(larguraGrid,0,coluna.largura) - opt_recuo_eixox_btn, 
								y + ALTURA_LINHA * indiceLinha, opt_botao_solto, opt_botao_apertado, 
								GF_CLOSE_BTN, 
								valorBotao);

				elseif(valorCelula.isColunaCheckBox)
					GFCheckBox( gump,
							x + larguraUtilizada + CentralizaEixoX(larguraGrid,0,coluna.largura) - opt_recuo_eixox_btn, 
							y + ALTURA_LINHA * indiceLinha, 
							opt_checkbox_solto, 
							opt_checkbox_apertado,
							valorCelula.valor ? true:false,
							valorBotao);
				else
					GFTextLine(gump, x + larguraUtilizada + 5, y + ALTURA_LINHA * indiceLinha,opt_cor_dado, valorCelula);
				endif;
				larguraUtilizada := larguraUtilizada + larguraGrid * coluna.largura;
				Sleepms(2);
			endforeach
			indiceLinha += 1; 
			Sleepms(2);
		endfor
	endif

	return gump;
endfunction;

function CriaTabelaGumpPaginada(x,y,larguraTotal,dados,byref gump,opcoesTabela := dictionary,opcoesColunas := dictionary, indiceBtnInicial := 0)

	var opt_max_pagina := DeterminaOpcaoTabela(opcoesTabela,MAX_PAGINAS, 5);
	var opt_qtd_limite_linha :=  DeterminaOpcaoTabela(opcoesTabela,QTD_LINHA_LIMITE,20);

	var pagina;
	var algo := CDbl(dados.size()) / opt_qtd_limite_linha;
	for( pagina := 0; (pagina + 1) - algo < 1; pagina := pagina + 1)
		if(pagina + 1 > opt_max_pagina)
			break;
		endif
		GFPage( gump, pagina + 1);
		gump := CriaTabelaGump(x,y,larguraTotal,dados,gump,pagina,opcoesTabela,opcoesColunas,indiceBtnInicial);

		//Controle de Paginação.
		if(!pagina == 0)
			GFAddButton( gump, x + larguraTotal / 2 - 40, y + (opt_qtd_limite_linha + 3) * ALTURA_LINHA, 2223, 2223, GF_PAGE_BTN, gump.cur_page - 1 );
		endif
		if((pagina + 2) - algo < 1)
			GFAddButton( gump, x + larguraTotal / 2  + 40, y + (opt_qtd_limite_linha + 3) * ALTURA_LINHA, 2224, 2224, GF_PAGE_BTN, gump.cur_page + 1 );
		endif
	endfor

	return gump;
endfunction

function CentralizaEixoX(larguraGrid,conteudo,gridsUtilizadas := 1)
	if(TypeOf(conteudo) == "String")
		return (larguraGrid * gridsUtilizadas / 2) - (Len(conteudo) * 3);
	else
		return (larguraGrid * gridsUtilizadas / 2);
	endif
endfunction

function DeterminaOpcaoTabela(opcoesTabela,opcao,padrao)
	return opcoesTabela[opcao]? opcoesTabela[opcao]:padrao;
endfunction