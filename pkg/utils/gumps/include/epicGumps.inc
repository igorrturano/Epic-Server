///////////////////////////////////
// Adicionado para o Epic! Shard //
///////////////////////////////////

use uo;
use os;
use basic;

//include "include/client";
include "gumps";
include "gumps_ex";
include ":destroyable:include/destroyItem";

const ALTURA_LINHA := 20;
const QUALTIDADE_COLUNAS := 12;

const GUMP_LARGURA := 800;
const GUMP_ALTURA := 600;
const TEMPO_DIA := 86400;

enum GUMP_PARAMS
	HIDE_TITLE, HIDE_PAGE, HIDE_BORDER,
	BORDER_GOLDEN, BORDER_BLUE, BORDER_MARBLE,
	NO_CLOSABLE
endenum

enum OPCOES_TABELA
	COR_TITULO_COLUNA := "corTituloId", // customiza a cor do titulo da COLUNA na tabela
	COR_TITULO_TABELA := "corTituloTabId", // customiza a cor do titulo da tabela
	COR_DADO := "corDadoId", // customiza a cor do valor nas celulas
	QTD_LINHA_LIMITE := "qtdeLimiteLinha", // Customiza O máximo de linhas de cada página da tábela.
	GUMPIC_FUNDO := "fundoId", // GUMPIC id do fundo da tabela.
	TITULO_TABELA := "tituloTabela", // Customiza o titulo da tabela
	BOTAO_APERTADO := "botaoApertadoId",
	BOTAO_SOLTO := "botaoSoltoId",
	CHECKBOX_APERTADO := "checkBoxApertadoId",
	CHECKBOX_SOLTO := "checkBoxSoltoId",
	RECUO_EIXO_X_BOTAO := "recuoExidoXBtn", // recua o botao no eixo x caso a centralização não tenha funcionado.
	RECUO_EIXO_Y_BOTAO := "recuoExidoXBtn", // recua o botao no eixo y caso a centralização não tenha funcionado.
	MOSTRAR_FUNDO := "isMostrarFundo", // customiza se deve mostrar o background da tabela.
	MAX_PAGINAS := "numeroMaxPaginas" // Numero máximo de páginas na sua versão páginada.
endenum

enum IMPORTANCIA_ITEM
	DISPENSAVEL := "dispensavel",
	EMPERRADA := "emperrada",
	INDESTRUTIVEL := "Indestrutível"
endenum 


/**
 * Purpose 
 * Criar um gump padronizado com os demais do Shard.
 *
 * Parameters
 * title: O gump que será adicionado a moldura
 * width: Largura
 * height: Altura
 * background: Imagem de fundo (caso queira alterar)
 * params: Array com demais parâmetros (usando as consts acima) para modificar a forma como o gump é criado, se terá barra para título, scroll, etc.
 *
 * Return gump
 * 
 */
function GFECreateGump(title, width, height, params := array{})
	var gump := GFCreateGump(0, 0, width, height);
	var x, y;
	
	if (NO_CLOSABLE in params)
		GFClosable(gump, 0);
	else
		GFClosable(gump, 1);
	endif
    GFDisposable(gump, 1);

	if (!(HIDE_BORDER in params))
		var border;
		if (BORDER_BLUE in params)
			border := BORDER_BLUE;
		elseif (BORDER_MARBLE in params)
			border := BORDER_MARBLE;
		else 
			border := BORDER_GOLDEN;
		endif
		GFEAddMoldura(gump, width, height, border);
	endif

	x := 20;
	y := 27;
	if (!(HIDE_TITLE in params))
		GFResizePic(gump, x, y, 9350, width - 27, 35);
		GFESetTitle(gump, width/2, title, 53);
		y += 40;
	endif

	if (!(HIDE_PAGE in params))
		GFResizePic(gump, x, y, 9350, width - 27, height - y - 8);
	endif

	return gump;
endfunction

/**
 * Purpose 
 * Adicionar moldura ao Gump
 *
 * Parameters
 * gump: O gump que será adicionado a moldura
 * width: Largura
 * height: Altura
 */
function GFEAddMoldura(byref gump, width, height, border)
	var background, molduras;
	if (border == BORDER_GOLDEN)
		background := 39929;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 39928 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 39930 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 35, img := 39926 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 35, img := 39925 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 35, img := 39927 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 39932 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 39931 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 39933 } // BOTTOMRIGHT
		};
	elseif (border == BORDER_BLUE)
		background := 40004;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 40003 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 40005 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 15, img := 40001 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 15, img := 40000 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 15, img := 40002 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 40007 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 40006 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 40008 } // BOTTOMRIGHT
		};
	elseif (border == BORDER_MARBLE)
		background := 3504;
		molduras := array{
			// SIDES
			struct{ x := 0, y := 15, width := 35, height := height - 15, img := 3503 }, // LEFT
			struct{ x := width - 25, y := 15, width := 35, height := height - 15, img := 3505 }, // RIGHT

			// TOP
			struct{ x := 25, y := 0, width := width - 41, height := 15, img := 3501 }, // TOPMIDDLE
			struct{ x := 0, y := 0, width := 35, height := 15, img := 3500 }, // TOPLEFT
			struct{ x := width - 25, y := 0, width := 35, height := 15, img := 3502 }, // TOPRIGHT

			// BOTTOM
			struct{ x := 25, y := height - 25, width := width - 25, height := 35, img := 3507 }, // BOTTOMMIDDLE
			struct{ x := 0, y := height - 25, width := 35, height := 35, img := 3506 }, // BOTTOMLEFT
			struct{ x := width - 25, y := height - 25, width := 35, height := 35, img := 3508 } // BOTTOMRIGHT
		};
	endif

	GFESetBackground(gump, width - 20, height - 20, background);
    foreach moldura in molduras
        GFGumpPicTiled(gump, moldura.x, moldura.y, moldura.width, moldura.height, moldura.img);
    endforeach
endfunction

function GFESetTitle(byref gump, x, title, color := 0)
	GFTextMid(gump, x, 35, Len(title)*2, color, title);
endfunction

/**
 * Parameters
 * gump: Gump para adicionar as linhas
 * arrTextos: Array de frases para adicionar
 * incrY: Valor a ser incrementado em y a cada linha
 * color: Cor do texto
 */

function GFESetBackground(byref gump, width, height, imgID)
	GFGumpPicTiled(gump, 10, 10, width, height, imgID);
endfunction

function GFEAddTextBox(byref gump, x, y, width, height := 16, text_color := COLOR_YELLOW, default_text := 0, id := 0, maxlenght := 0)
	GFResizePic(gump, x, y, GFGetCfgConst("BackGrounds", "GOLDTRIM"), width + 10, height + 15);
	var val := GFTextEntry(gump, x + 6, y + 6, width, height, text_color, default_text, id, maxlenght);
	return val;
endfunction

function GFEAdd(byref gump)

endfunction


/* 
 * CriaTabelaGump(x,y,larguraTotal,dados,byref gump,paginaTabela := 0,opcoesTabela := dictionary,opcoesColunas := dictionary, indiceBtnInicial := 0)
 *
 * Purpose
 *	Componente de tabela altamente customizavel. Para usar, basta mandar um array contendo structs que ele
 *  automaticamente irá criar as colunas baseadas nos membros desses structs. No entanto, é possivel customizar
 *  as colunas usando o parâmetro opcoesColunas e a própria tabela usando o parâmetro opcoesTabela.
 * 
 *  Nota importante. se o struct i
 * 
 * Parameters
 * x: posição x inicial na tela
 * y: posição y inicial na tela
 * larguraTotal: largura total reservada para a tabela.
 * dados:  um array contendo structs com as informaões da tabela. 
		Atenção, se o valor de um dos membros for um struct com membros:
			.+isColunaBotao
			.+valor
		a coluna terá um unico botão na célula
		ou 
			.+isColunaCheckBox
			.+valor
		a coluna teráum unico checkbox na celula
 * gump:  o gump do qual a tabela fará parte..
 * paginaTabela:  a pagina atual da tabela. [será usada como indice inicial. Começa em 0]
 * opcoesTabela:  um dictionary com as configurações da tabela, sendo:
 * 
 * 		dictionary[chave<OPCOES_TABELA>] -> <valor>.
 * 
 * opcoesColunas: um dictionary com as configurações das colunas, sendo:
 * 
 * 		dictionary[chave<nome equivalente ao membro da struct que vai se configurar>] 
 							->  value<struct{titulo := <titulo da coluna> ,
											[ordem := <ordem numerica>],
											[largura := <um valor maior que zero e menor que 12>}]> //12 colunas na Grid.

		exemplo:
			colCFG := dictionary;
			colCfg["nome"] := struct{titulo := "Nome Do personagem",ordem := 1,largura := 5};
			ou
			colCfg["nome"] := struct{titulo := "Nome Do personagem"}; // A ordem será definida automaticamente e a largura dividida igualmente.
 * indiceBtnInicial: um indice para iniciar os botoes da tabela, caso precise.
 *
 * Return value
 *   o gump
 *
 */
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
		var opt_recuo_eixox_btn := DeterminaOpcaoTabela(opcoesTabela,RECUO_EIXO_X_BOTAO,-5);
		var opt_recuo_eixoy_btn := DeterminaOpcaoTabela(opcoesTabela,RECUO_EIXO_Y_BOTAO,0);
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
				GFResizePic(gump, cInt(x), cInt(y),opt_gumpic_fundo, larguraTotal, ALTURA_LINHA);
			endif

			GFHTMLArea( 
			gump,
			cInt(x + CentralizaEixoX(larguraTotal,opcoesTabela[TITULO_TABELA])),
			cInt(y),
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
				GFResizePic(gump, cInt(x + larguraUtilizada), cInt(y),opt_gumpic_fundo, larguraGrid * coluna.largura, ALTURA_LINHA);
			endif

			GFTextLine(gump,cInt( x + larguraUtilizada + CentralizaEixoX(larguraGrid,coluna.titulo,coluna.largura)),cInt( y), opt_cor_titulo_coluna, coluna.titulo);
			larguraUtilizada := larguraUtilizada + larguraGrid * coluna.largura;
			Sleepms(2);
		endforeach

		//aqui começa a tabela em si
		var indice := paginaTabela * opt_qtd_limite_linha + 1;
		var indiceLinha := 1;

		for linha := indice to  indice + (opt_qtd_limite_linha - 1 )
			if(linha > dados.size())
				break;
			endif
			larguraUtilizada := 0;
			foreach coluna in colunas
				if(opt_is_mostrar_fundo)
					GFResizePic(gump,cInt( x + larguraUtilizada),cInt( y + ALTURA_LINHA * indiceLinha), opt_gumpic_fundo, larguraGrid * coluna.largura, ALTURA_LINHA);
				endif
				var valorCelula :=  dados[linha][coluna.chaveReal];
				var valorBotao := valorCelula.valor ? valorCelula.valor: indiceLinha + indiceBtnInicial;
				if(valorCelula.isColunaBotao)
					GFAddButton( gump,
								cInt(x + larguraUtilizada + CentralizaEixoX(larguraGrid,0,coluna.largura) + opt_recuo_eixox_btn), 
								cInt(y + ALTURA_LINHA * indiceLinha + opt_recuo_eixoy_btn), opt_botao_solto, opt_botao_apertado, 
								GF_CLOSE_BTN, 
								valorBotao);

				elseif(valorCelula.isColunaCheckBox)
					GFCheckBox( gump,
							cInt(x + larguraUtilizada + CentralizaEixoX(larguraGrid,0,coluna.largura) + opt_recuo_eixox_btn), 
							cInt(y + ALTURA_LINHA * indiceLinha + opt_recuo_eixoy_btn), 
							opt_checkbox_solto, 
							opt_checkbox_apertado,
							valorCelula.valor,
							valorBotao);
				else
					GFTextLine(gump, cInt(x + larguraUtilizada + 5), cInt(y + ALTURA_LINHA * indiceLinha),opt_cor_dado, valorCelula);
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

/* 
 * CriaTabelaGumpPaginada(x,y,larguraTotal,dados,byref gump,opcoesTabela := dictionary,opcoesColunas := dictionary, indiceBtnInicial := 0)
 *
 * Purpose
 *	 Versão da CriaTabelaGump com um controle de paginação já imbutido. ideal para tabelas com poucos registros.
 * 
 * 
 * Parameters
 * x: posição x inicial na tela
 * y: posição y inicial na tela
 * larguraTotal: largura total reservada para a tabela.
 * dados:  um array contendo structs com as informaões da tabela. 
		Atenção, se o valor de um dos membros for um struct com membros:
			.+isColunaBotao
			.+valor
		a coluna terá um unico botão na célula
		ou 
			.+isColunaCheckBox
			.+valor
		a coluna teráum unico checkbox na celula
 * gump:  o gump do qual a tabela fará parte..
 * opcoesTabela:  um dictionary com as configurações da tabela, sendo:
 * 
 * 		dictionary[chave<OPCOES_TABELA>] -> <valor>.
 * 
 * opcoesColunas: um dictionary com as configurações das colunas, sendo:
 * 
 * 		dictionary[chave<nome equivalente ao membro da struct que vai se configurar>] 
 							->  value<struct{titulo := <titulo da coluna> ,
											[ordem := <ordem numerica>],
											[largura := <um valor maior que zero e menor que 12>}]> //12 colunas na Grid.

		exemplo:
			colCFG := dictionary;
			colCfg["nome"] := struct{titulo := "Nome Do personagem",ordem := 1,largura := 5};
			ou
			colCfg["nome"] := struct{titulo := "Nome Do personagem"}; // A ordem será definida automaticamente e a largura dividida igualmente.
 * indiceBtnInicial: um indice para iniciar os botoes da tabela, caso precise.
 *
 * Return value
 *   o gump
 *
 */
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
		var linhaAdicional := 2;
		if(opcoesTabela[TITULO_TABELA])
			linhaAdicional :=  3;
		endif

		if(!pagina == 0)
			GFAddButton( gump, (x + larguraTotal) / 2 - 40, y + (opt_qtd_limite_linha + linhaAdicional) * ALTURA_LINHA, 2223, 2223, GF_PAGE_BTN, gump.cur_page - 1 );
		endif
		if((pagina + 2) - algo < 1)
			GFAddButton( gump, (x + larguraTotal) / 2  + 40, y + (opt_qtd_limite_linha + linhaAdicional) * ALTURA_LINHA, 2224, 2224, GF_PAGE_BTN, gump.cur_page + 1 );
		endif
	endfor

	return gump;
endfunction

/* 
 * CentralizaEixoX(larguraGrid,conteudo,gridsUtilizadas := 1)
 *
 * Purpose
 *	 Centraliza o conteúdo dentro de uma célula, usando o conceito de Grids utilizado no CriaTabelaGump.
 * 
 * 
 * Parameters
 * larguraGrid: largura de uma unidade da grid
 * conteudo: conteudo a ser centralizado.
 * gridsUtilizadas: quantidade de grids que o conteudo ocupa.

 *
 * Return value
 *   valor no eixo x.
 *
 */
function CentralizaEixoX(larguraGrid,conteudo,gridsUtilizadas := 1)
	if(TypeOf(conteudo) == "String")
		return CInt((larguraGrid * gridsUtilizadas / 2) - (Len(conteudo) * 3));
	else
		return CInt((larguraGrid * gridsUtilizadas / 2));
	endif
endfunction

//Função "privada" da criação de tabela.
function DeterminaOpcaoTabela(opcoesTabela,opcao,padrao)
	return opcoesTabela[opcao]? opcoesTabela[opcao]:padrao;
endfunction
