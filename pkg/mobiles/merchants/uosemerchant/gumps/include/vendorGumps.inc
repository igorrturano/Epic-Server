use math;

include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";
include ":gumps:/include/requestGump";

CONST GUMP_MONTALOJA_WIDTH := 1045;
CONST GUMP_MONTALOJA_HEIGHT := 600;
CONST LOJA_BOLA_HUE := 1358;

//30543

CONST PAINELPRODUTO_INIPOS_X := 210;
CONST PAINELPRODUTO_INIPOS_Y := 36;
CONST PAINELPRODUTO_LINHA_Y := 165;
CONST PAINELPRODUTO_COLUNA_X := 265;
CONST PAINELPRODUTO_ESPACO_Y := 5;
CONST PAINELPRODUTO_ESPACO_X := 5;
CONST PAINELPRODUTO_COLUNAS_MAX := 3;
CONST PAINELPRODUTO_VAL_LOTEQTD_XOFFSET := 220;
CONST PAINELPRODUTO_POS_CENTRO_X := (PAINELPRODUTO_INIPOS_X + PAINELPRODUTO_ESPACO_X) * PAINELPRODUTO_COLUNAS_MAX;

CONST PAINELBOTOES_TAMANHO_LINHA := 35;
CONST PAINELBOTOES_FUNDO_WIDTH := 205;

CONST PAINELPRODUTO_QUANTIDADE_BOTOES := 2;
CONST ID_BOTAO_PROXIMA_PAGINA := 30;
CONST ID_BOTAO_PAGINA_ANTERIOR := 31;

CONST PRODUTO_FAIXA_INICIAL_BOTOES := 1000;

function MontaLojaGump(session)
	
	var gump := GFECreateGump("teste",GUMP_MONTALOJA_WIDTH,GUMP_MONTALOJA_HEIGHT, {BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});
	
	GFPage( gump, 0 );

	GFHTMLArea( gump, PAINELPRODUTO_POS_CENTRO_X - 100 , 25 ,200, 40,"<BASEFONT size=12 color=#cad7ed><center>Loja de {}".format(session.nomeVendedor) ); 
	GFPicTiled(gump,PAINELPRODUTO_INIPOS_X,20,PAINELPRODUTO_COLUNA_X + 50, 30,30543); //Detalhe Borda esquerda superior
	GFPicTiled(gump,PAINELPRODUTO_INIPOS_X * 3 + 120,20,PAINELPRODUTO_COLUNA_X + 10, 30,30543); //Detalhe Borda direita superior
	GFGumpPic(gump,PAINELPRODUTO_POS_CENTRO_X - 140,15,1648,LOJA_BOLA_HUE); //BolaNome Brilhante Esquerda superior
	GFGumpPic(gump,PAINELPRODUTO_POS_CENTRO_X + 90,15,1648,LOJA_BOLA_HUE); //BolaNome Brilhante direita superior
	GFGumpPic(gump,GUMP_MONTALOJA_WIDTH - 30,15,1648,LOJA_BOLA_HUE); //Bola Brilhante direita superior

	GFPicTiled(gump,PAINELPRODUTO_INIPOS_X,GUMP_MONTALOJA_HEIGHT - 40,PAINELPRODUTO_COLUNA_X + 50, 30,30537); //Detalhe Borda esquerda inferior
	GFPicTiled(gump,PAINELPRODUTO_INIPOS_X * 3 + 120, GUMP_MONTALOJA_HEIGHT - 40,PAINELPRODUTO_COLUNA_X + 10, 30,30537); //Detalhe Borda direita inferior
	GFGumpPic(gump,PAINELPRODUTO_POS_CENTRO_X - 140,GUMP_MONTALOJA_HEIGHT - 37,1648,LOJA_BOLA_HUE); //BolaNome Brilhante Esquerda inferior
	GFGumpPic(gump,PAINELPRODUTO_POS_CENTRO_X + 90,GUMP_MONTALOJA_HEIGHT - 37,1648,LOJA_BOLA_HUE); //BolaNome Brilhante direita inferior
	GFGumpPic(gump,GUMP_MONTALOJA_WIDTH - 30,GUMP_MONTALOJA_HEIGHT - 37,1648,LOJA_BOLA_HUE); //Bola Brilhante direita inferior

	PainelBotoes(gump, session.tiposItemVendedor,session.catalogoTipoItems, session.isAdministrador);
	PainelProduto(gump,
				session,
				PAINELPRODUTO_INIPOS_X,
				PAINELPRODUTO_INIPOS_Y,
				session.isAdministrador
				);
	
	return gump;
endfunction

function MontaVenderItemGump(params)
	var gump := GFCreateGump();
	var x := 400;
	var y := 250;
	GFPage( gump, 0 );
	GFResizePic( gump, 14 + x, 22 + y, 9270, 176, 149 + ( params.isLotCMD * 50 ) );
	GFResizePic( gump, 27 + x, 32 + y, 9350, 150, 126 + ( params.isLotCMD * 50 ) );
	GFAddButton( gump, 42 + x, 123 + y + ( params.isLotCMD * 50 ), 0x818, 0x819, GF_CLOSE_BTN,0 );
	GFAddButton( gump, 114 + x, 123 + y + ( params.isLotCMD * 50 ), 0x81A, 0x81C, GF_CLOSE_BTN,1 );
	GFHtmlShadowed( gump, 80 + x, 37 + y, 80, 20, "#FFFFFF", "Vender", 5 );
	GFTextLine( gump, 39 + x, 69 + y, 1891, "Moedas por Unidade" );
	GFResizePic( gump, 39 + x, 89 + y, 30546, 126, 26 );
	GFGumpPic( gump, 43 + x, 94 + y, 40022, 0 );
	GFTextEntry( gump, 62 + x, 92 + y, 89, 20, 1153, "",2 );

	if ( params.isLotCMD )
		GFTextLine( gump, 48 + x, 69 + y + 48, 1891, "Itens por lote" );
		GFResizePic( gump, 75 + x, 89 + y + 50, 30546, 40, 26 );
		GFTextEntry( gump, 68 + 20 + x, 92 + y + 50, 30, 20, 1153, "1",3 );
		GFTooltipText( gump, "Número de items que são vendidos por unidade. Exemplo: 4 ferros por 1 unidade" );
	endif

	if ( params.papelJogador >= 3 )
		GFResizePic( gump, 191 + x, 21 + y, 9270, 138, 149 );
		GFResizePic( gump, 204 + x, 35 + y, 9350, 112, 126 );
		GFHTMLArea( gump, 219 + x, 40 + y, 100, 20, "<BASEFONT size=5 color=#FFFFFF>[GM] Troca" );
		GFAddButton( gump, 223 + x, 73 + y, 239, 248, GF_CLOSE_BTN, 4 );
		GFHTMLArea( gump, 215 + x, 93 + y, 100, 20, "<BASEFONT size=5 color=#FFFFFF>Options" );
		GFCheckBox( gump, 210 + x, 113 + y, 0x9C4E, 0x9C4F,0,5);
		GFTextLine( gump, 240 + x, 112 + y, 1153, "Infinito" );
	endif
	return gump;
endfunction

function GumpEscolhaQuantidade(amount)

	var gump := GFCreateGump();
	var x := 450;
	var y := 250;
	GFPage( gump, 0 );
	GFResizePic( gump, 14 + x, 22 + y, 9270, 176, 149 );
	GFResizePic( gump, 27 + x, 32 + y, 9350, 150, 126 );

	GFAddButton( gump, 42 + x, 123 + y, 0x818, 0x819, GF_CLOSE_BTN, 1 );
	GFAddButton( gump, 114 + x, 123 + y, 0x81A, 0x81C, GF_CLOSE_BTN, 2 );
	GFHtmlShadowed( gump, 80 + x, 37 + y, 80, 20, "#FFFFFF", "Comprar", 5 );
	GFTextLine( gump, 39 + x, 69 + y, 1891, "Quantidade? Max: {}".format( amount ));
	GFResizePic( gump, 49 + x, 89 + y, 30546, 100, 26 );
	GFTextEntry( gump, 62 + x, 92 + y, 89, 20, 1153, "{}".format( amount ), 3);

	return gump;
endfunction


function PainelBotoes(byref gump,tiposItemVendedor, catalogoTipoItems,isAdministrador)

	var y := 90;
	var x := 40;
	var indice := 1;

	GFResizePic(gump, 20, 20, 39925, PAINELBOTOES_FUNDO_WIDTH, 570); // fundo Botoes
	GFTextLine( gump, PAINELBOTOES_FUNDO_WIDTH / 2 , 40 , 1051, "Filtros" ); 
	if(isAdministrador)
		GFAddButton(gump, 60 ,35,22124,22125,GF_CLOSE_BTN, 3); // Botão regra de negócio
		GFTooltipText( gump, "Regras de Negócio");
		GFAddButton(gump, 155 ,35,22124,22125,GF_CLOSE_BTN, 6); // Botão regra de negócio
		GFTooltipText( gump, "histórico");
	endif
	GFTextEntry(gump,105,82,90,30,1051,"",4, 50); // Caixa de pesquisa
	GFAddButton(gump, 70,80,4011,4013,GF_CLOSE_BTN,2);
	GFTooltipText( gump, "Pesquisar por texto");
	GFAddButton(gump, 40,80,4020,4022,GF_CLOSE_BTN,5); 
	GFTooltipText( gump, "Limpar todos os filtros");

	foreach tipo in tiposItemVendedor
		GFAddButton(gump, x ,y  +  PAINELBOTOES_TAMANHO_LINHA * indice,9720,9724,GF_CLOSE_BTN, catalogoTipoItems[tipo]);
		GFTextLine( gump, x + 35, y + 5 + PAINELBOTOES_TAMANHO_LINHA * indice , 1051,  tipo);

		indice += 1;
	endforeach

endfunction

function PainelProduto(byref gump,session ,x,y,isAdministrador)
	
	var linha := 0;
	var paginaAtual := 1;
	var quantidadeItensTotal := session.itensVendidos.size();
	var paginaMaxima :=  ceil(quantidadeItensTotal / session.paginador.itensPorPagina);
	if ( quantidadeItensTotal < 1 )
		GFTextLine( gump, PAINELPRODUTO_POS_CENTRO_X - 40, 295, 1051, "Nenhum item a venda!" );
	endif

	session.paginador.pagina := 
					session.paginador.pagina > paginaMaxima
					? paginaMaxima
					: session.paginador.pagina;

	var paginador := MontaPaginador(session.paginador.pagina, session.paginador.itensPorPagina);
	
	GFTextLine( gump, PAINELPRODUTO_POS_CENTRO_X - 50, 568, 1153, "Página {} de {}"
		.format( 
			session.paginador.pagina + 1,  
			Ceil((quantidadeItensTotal - 1) / session.paginador.itensPorPagina) + 1)
		);

	if(quantidadeItensTotal >= paginador.indicePaginaSeguinte)
		GFAddButton( gump, PAINELPRODUTO_POS_CENTRO_X + 40, 571, 2224, 2224, GF_CLOSE_BTN, ID_BOTAO_PROXIMA_PAGINA );
	endif
	if(1 >= paginador.indicePaginaAnterior)
		GFAddButton( gump, PAINELPRODUTO_POS_CENTRO_X - 75, 571, 2223, 2223, GF_CLOSE_BTN, ID_BOTAO_PAGINA_ANTERIOR );
	endif

	for  indice := paginador.indicePaginaAtual to (paginador.indicePaginaSeguinte - 1)
		sleepms( 2 );
		if ( indice > quantidadeItensTotal)
			break;
		endif
		var itemNoIndice := session.itensVendidos[indice];
		var baseIdBotao :=  indice * PAINELPRODUTO_QUANTIDADE_BOTOES + PRODUTO_FAIXA_INICIAL_BOTOES;

		if ( !itemNoIndice )
			break;
		endif

		if ( (indice - 1) % PAINELPRODUTO_COLUNAS_MAX == 0 && paginador.indicePaginaAtual != indice ) //chegou ao limite de colunas, vai rpa proxima linha.
			x := PAINELPRODUTO_INIPOS_X;
			linha := linha + 1;
		endif

		y := PAINELPRODUTO_INIPOS_Y + ( linha * PAINELPRODUTO_LINHA_Y ); //incrementa linha

		/*if(indiceItem % session.paginador.itensPorPagina == 1)//chegou a limite da pagina, muda pra proxima.
			linha := 0;
			y := PAINELPRODUTO_INIPOS_Y;
		endif*/

		x += PAINELPRODUTO_ESPACO_X; //espaçamento.
		y += (PAINELPRODUTO_ESPACO_Y * linha); //espaçamento.
		GFResizePic(gump, x + 20, y + 20, 5054, 260, 160); //brackgroud iteminfo  5054 | 3500
		GFResizePic(gump, x + 30, y + 30, 9250, 100, 110); //foto 9250 | 94
		GFTooltipText(gump, itemNoIndice.descVenda);
		if(isAdministrador)
			GFHTMLArea( gump, x + 45, y + 42, 80, 20, "<BASEFONT size=4 color=#FFFFFF><center>{}".format(itemNoIndice.nomeVendedor));
			GFTooltipText( gump, "Nome do vendedor deste prouto");
		endif
		GFGumpPic(gump,x + 152,y + 149,40022); // moeda de ouro
		GFGumpPic(gump,x + 139,y + 148,95);//detalhe esquerdo gold
		GFGumpPic(gump,x + 260,y + 148,97);//detalhe direito gold
		GFResizePic(gump, x + 141, y + 30,9350 , 125, 110); //sellerinfo

		GFHTMLArea( gump, x + 146, y + 35, 110, 40, "<BASEFONT size=8 color=#13518c><center>{}".format(itemNoIndice.item.desc) );
		if(itemNoIndice.infoCusto.lote > 1)
			GFGumpPic(gump,x + 178,y + 95,5601);//seta 
			GFTextLine( gump, x + 149, y + 94, 1051, "Lote");
			GFTextLine( gump, x + 197, y + 94, 1153, "{}".format(itemNoIndice.infoCusto.lote));
		endif;
		GFGumpPic(gump,x + 200,y + 117,5601); //seta
		GFTextLine( gump, x + 149, y + 115, 1051, "A Venda" );
		GFTextLine( gump, x + 220, y + 115, 1153, "{}".format( itemNoIndice.item.amount / itemNoIndice.infoCusto.lote));
		GFTextLine( gump, x + 172, y + 148, 1153, "{}".format( itemNoIndice.infoCusto.cost_amt));
		GFTooltipText( gump, "{} unidades por {} moedas de {}".format( itemNoIndice.infoCusto.lote,
																		itemNoIndice.infoCusto.cost_amt,
																		itemNoIndice.infoCusto.cost_name ) );
		GFTilePic( gump, x + 58, y + 75, itemNoIndice.item.graphic, cint( itemNoIndice.item.color) );

		GFAddButton(gump, x + 26,y + 141,4037,4036,GF_CLOSE_BTN, baseIdBotao);//comprar
		GFTooltipText( gump, "Comprar" );
		if(isAdministrador || itemNoIndice.isVendaJogador)
			GFAddButton(gump, x + 66,y + 147,2639,2637,GF_CLOSE_BTN, baseIdBotao + 1);//deletar
		endif
		x += PAINELPRODUTO_COLUNA_X;
	endfor
	return gump;
endfunction