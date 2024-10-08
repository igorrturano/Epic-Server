use math;

include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";
include ":gumps:/include/requestGump";

CONST GUMP_MONTALOJA_WIDTH := 1045;
CONST GUMP_MONTALOJA_HEIGHT := 600;


CONST PAINELPRODUTO_INIPOS_X := 210;
CONST PAINELPRODUTO_INIPOS_Y := 33;
CONST PAINELPRODUTO_LINHA_Y := 165;
CONST PAINELPRODUTO_COLUNA_X := 265;
CONST PAINELPRODUTO_ESPACO_Y := 5;
CONST PAINELPRODUTO_ESPACO_X := 5;
CONST PAINELPRODUTO_COLUNAS_MAX := 3;
CONST PAINELPRODUTO_VAL_LOTEQTD_XOFFSET := 220;
CONST PAINELPRODUTO_POS_CENTRO_X := (PAINELPRODUTO_INIPOS_X + PAINELPRODUTO_ESPACO_X) * PAINELPRODUTO_COLUNAS_MAX;

CONST PAINELBOTOES_TAMANHO_LINHA := 35;
CONST PAINELBOTOES_FUNDO_WIDTH := 205;

function MontaLojaGump(params)
	
	var gump := GFECreateGump("teste",GUMP_MONTALOJA_WIDTH,GUMP_MONTALOJA_HEIGHT, {BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});
	var indiceItem := 1;
	
	GFPage( gump, 0 );

	PainelBotoes(gump, params.tiposItemVendedor,params.catalogoTipoItems, params.isVendorAdm,params.nomeVendedor);
	PainelProduto(gump,
				params.itensVendidos,
				indiceItem ,
				PAINELPRODUTO_INIPOS_X,
				PAINELPRODUTO_INIPOS_Y,
				9,
				params.isVendorAdm
				);
	
	return gump;
endfunction

function PainelBotoes(byref gump,tiposItemVendedor, catalogoTipoItems,isVendorAdm, nomeVendedor)

	var y := 90;
	var x := 40;
	var indice := 1;

	GFHTMLArea( gump, PAINELPRODUTO_POS_CENTRO_X - 100 , 25 ,200, 40,"<BASEFONT size=12 color=#cad7ed><center>Loja de {}".format(nomeVendedor) ); 
	GFResizePic(gump, 20, 20, 39925, PAINELBOTOES_FUNDO_WIDTH, 570); // fundo Botoes
	GFTextLine( gump, PAINELBOTOES_FUNDO_WIDTH / 2 , 40 , 1051, "Filtros" ); 
	if(isVendorAdm)
		GFAddButton(gump, 60 ,35,22124,22125,GF_CLOSE_BTN, 3); // Botão regra de negócio
		GFTooltipText( gump, "Regras de Negócio");
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

function PainelProduto(byref gump,itemsVendidos ,indiceItem ,x,y,maxItemPagina,isJogadorAdm)
	
	var linha := 0;
	var paginaAtual := 1;

	if ( itemsVendidos.size() < 1 )
		GFTextLine( gump, PAINELPRODUTO_POS_CENTRO_X - 40, 295, 1051, "Nenhum item a venda!" );
	endif

	foreach infoItem in itemsVendidos

		if ( indiceItem % PAINELPRODUTO_COLUNAS_MAX == 1 ) //chegou ao limite de colunas, vai rpa proxima linha.
			x := PAINELPRODUTO_INIPOS_X;
			linha := linha + 1;
		endif

		y := PAINELPRODUTO_INIPOS_Y + ( linha * PAINELPRODUTO_LINHA_Y ); //incrementa linha

		if(indiceItem % maxItemPagina == 1)//chegou a limite da pagina, muda pra proxima.
			linha := 0;
			y := PAINELPRODUTO_INIPOS_Y;
			ControlarPagina(gump,indiceItem,maxItemPagina,itemsVendidos.size());
		endif

		x += PAINELPRODUTO_ESPACO_X; //espaçamento.
		y += (PAINELPRODUTO_ESPACO_Y * linha); //espaçamento.

		GFResizePic(gump, x + 20, y + 20, 5054, 260, 160); //brackgroud iteminfo  5054 | 3500
		GFResizePic(gump, x + 30, y + 30, 9250, 100, 110); //foto 9250 | 94
		if(isJogadorAdm)
			GFHTMLArea( gump, x + 45, y + 42, 80, 20, "<BASEFONT size=4 color=#FFFFFF><center>{}".format(infoItem.nomeVendedor));
			GFTooltipText( gump, "Nome do vendedor deste prouto");
		endif
		GFGumpPic(gump,x + 152,y + 149,40022); // moeda de ouro
		GFGumpPic(gump,x + 139,y + 148,95);//detalhe esquerdo gold
		GFGumpPic(gump,x + 260,y + 148,97);//detalhe direito gold
		GFResizePic(gump, x + 141, y + 30,9350 , 125, 110); //sellerinfo

		GFHTMLArea( gump, x + 146, y + 35, 110, 40, "<BASEFONT size=8 color=#13518c><center>{}".format(infoItem.item.desc) );
		if(infoItem.infoCusto.lote > 1)
			GFGumpPic(gump,x + 178,y + 95,5601);//seta 
			GFTextLine( gump, x + 149, y + 94, 1051, "Lote");
			GFTextLine( gump, x + 197, y + 94, 1153, "{}".format(infoItem.infoCusto.lote));
		endif;
		GFGumpPic(gump,x + 200,y + 117,5601); //seta
		GFTextLine( gump, x + 149, y + 115, 1051, "A Venda" );
		GFTextLine( gump, x + 220, y + 115, 1153, "{}".format( infoItem.item.amount / infoItem.infoCusto.lote));
		GFTextLine( gump, x + 172, y + 148, 1153, "{}".format( infoItem.infoCusto.cost_amt));
		GFTooltipText( gump, "{} unidades por {} moedas de {}".format( infoItem.infoCusto.lote,
																		infoItem.infoCusto.cost_amt,
																		infoItem.infoCusto.cost_name ) );
		GFTilePic( gump, x + 58, y + 75, infoItem.item.graphic, cint( infoItem.item.color) );
		GFTooltip(gump, infoItem.descVenda);

		GFAddButton(gump, x + 26,y + 141,4037,4036,GF_CLOSE_BTN, infoItem.serial);//comprar
		GFTooltipText( gump, "Comprar" );
		if(isJogadorAdm || infoItem.isVendaJogador)
			GFAddButton(gump, x + 66,y + 147,2639,2637, 90000 + cint( infoItem.serial));//deletar
		endif

		x += PAINELPRODUTO_COLUNA_X;
		indiceItem += 1;
		sleepms( 2 );
	endforeach
	return gump;
endfunction

function ControlarPagina(byref gump,indiceItem,maxItemPagina,maxItems)

	GFPage( gump, gump.cur_page + 1 );
	var dePaginas := maxItems / maxItemPagina;
	var resto := maxItems % maxItemPagina;

	dePaginas := dePaginas <= 0 ? 1 : dePaginas;
	dePaginas := resto >= 0? dePaginas + 1 : dePaginas;

	GFTextLine( gump, PAINELPRODUTO_POS_CENTRO_X - 50, 568, 1153, "Página {} de {}".format( gump.cur_page,  dePaginas));
	
	if ( maxItems > maxItemPagina && indiceItem < (maxItems - maxItemPagina))
		GFAddButton( gump, PAINELPRODUTO_POS_CENTRO_X + 40, 571, 2224, 2224, GF_PAGE_BTN, gump.cur_page + 1 );
	endif

	if(indiceItem > maxItemPagina)
		GFAddButton( gump, PAINELPRODUTO_POS_CENTRO_X - 75, 571, 2223, 2223, GF_PAGE_BTN, gump.cur_page - 1 );
	endif

endfunction