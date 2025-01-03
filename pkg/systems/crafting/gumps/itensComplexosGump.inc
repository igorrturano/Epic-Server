include ":gumps:/include/epicGumps";

const G_LARGURA := 800;
const G_ALTURA := 600;

const G_CELULA_DIVISOR := 12;
const G_MARGEM_ESQUERDA := 10;
const G_MARGEM_SUPERIOR := 10;
const G_ESPACAMENTO := 10;

var alturaCelula :=  G_ALTURA / G_CELULA_DIVISOR;
var larguraCelula :=  G_LARGURA / G_CELULA_DIVISOR;

var espacamentoAplicado := G_ESPACAMENTO * 2;

function MontaItensComplexosGump(quem,byref gump,parametros,byref navegacao)
	gump := GFECreateGump("GumpItensComplexos",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});

	//GFResizePic(gump, 20, 20, 83, 140, 160);//5170, 1755,

	var larguraPainelCategoria := DefineLarguraPainel(3) + G_ESPACAMENTO;
	var alturaPainelCategoria := DefineAlturaPainel(11);

	var larguraPainelReceita := DefineLarguraPainel(9);
	var alturaPainelReceita :=  DefineAlturaPainel(8);

	var larguraPainelEsquema := DefineLarguraPainel(9);
	var alturaPainelEsquema := DefineAlturaPainel(3);

	var larguraPainelRodape := DefineLarguraPainel(12);
	var alturaPainelRodape := DefineAlturaPainel(1);


	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO, G_MARGEM_SUPERIOR + G_ESPACAMENTO, 83, larguraPainelCategoria, alturaPainelCategoria); // Painel Categorias.
	
	PreencheCategorias(gump,larguraPainelCategoria,parametros);

	var painelReceitasX := G_MARGEM_ESQUERDA + espacamentoAplicado + larguraPainelCategoria;
	var painelReceitasY := G_MARGEM_SUPERIOR + G_ESPACAMENTO ;
	GFResizePic(gump, painelReceitasX, painelReceitasY , 1460, larguraPainelReceita, alturaPainelReceita); //Painel Receitas.
	GFResizePic(gump, painelReceitasX,  painelReceitasY + alturaPainelReceita + espacamentoAplicado, 1755, larguraPainelEsquema, alturaPainelEsquema); //Painel Esquemas.
	
	GFHTMLArea(gump,painelReceitasX + larguraPainelEsquema / 2 - 35,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 15,70,20,"<basefont size=55 color=#ffffff><center>Esquemas</center>");
	GFTextLine(gump,painelReceitasX + 110,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,86,"Nome Receita 1");
	GFAddButton(gump,painelReceitasX + 15,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4011,4012);
	GFAddButton(gump,painelReceitasX + 45,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4020,4021);
	GFAddButton(gump,painelReceitasX + 75,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4023,4024);

	PreencheReceitas(gump , painelReceitasX, 45, painelReceitasY, 55,larguraPainelReceita,alturaPainelReceita,parametros.visaoReceitas[navegacao.categoriaSelecionada]);
	
	GFGumpPic(gump,painelReceitasX + larguraPainelReceita / 2 - 102,20,1764); //Placa Titulo Receitas.
	GFHTMLArea(gump,painelReceitasX + larguraPainelReceita / 2 - 102,35,205,20,"<basefont size=55 color=#ffffff><center>Receitas</center>");
	

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,alturaCelula * 11 + 10, 5054, larguraPainelRodape, alturaPainelRodape);// Painel Rodapé

	return gump;
endfunction

function PreencheReceitas(byref gump,x,deslocamentoX,y,deslocamentoY,largura,altura,receitas)

	//TODO Implementar uma forma de lembrar onde estava da ultima vez que abriu o gump. Talvez usando a CProp legado "Craft".
	var page := 1;
	var colunasLinha := 4;
	GFPage(gump,page);
	if(receitas != error && receitas.size() > 0)
		var linha := 0;
		foreach receita in receitas
			MontaJanelaReceita(gump , x + deslocamentoX, y + deslocamentoY,linha, (_receita_iter - 1) % colunasLinha,receita);

			//Controle de Paginação.
			if(_receita_iter % (colunasLinha * 2) == 0)
				page += 1;
				GFAddButton(gump,x + largura - 45, y + altura / 2,4005,4006,GF_PAGE_BTN,gump.cur_page + 1);
				GFPage(gump,page);
				GFAddButton(gump,x + 13, y + altura / 2 ,4014,4015,GF_PAGE_BTN,gump.cur_page - 1);
				linha := 0;
			else
				if(_receita_iter % colunasLinha == 0)
					linha += 1;
				endif
			endif
			Sleepms(2);
		endforeach
	else
		GFHTMLArea(gump,x + largura / 2 - 102,y + altura / 2,205,20,"<basefont size=55 color=#ffffff><center>Vazio</center>"); //TODO alterar para algo mais bonitinho.
	endif
	GFPage(gump,0);
endfunction

function PreencheCategorias(byref gump,larguraPainelCategoria, parametros)
	
	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + larguraPainelCategoria / 2 - 35,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10,70,20,"<basefont size=55 color=#ffffff><center>Categorias</center>");
	foreach categoria in (parametros.visaoReceitas.keys())
		GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10,  G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _categoria_iter) ,4005,4006,GF_CLOSE_BTN, _categoria_iter);
		GFTextLine(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 45,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _categoria_iter),86,categoria);
		Sleepms(2);
	endforeach
endfunction

function EnviaGumpComponentes(quem)
	var gump := GFECreateGump("GumpComponentes",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});
	
	var opcoesTabela := dictionary{ 
							QTD_LINHA_LIMITE -> 15,
							TITULO_TABELA -> "Escolha um material:",
							BOTAO_SOLTO -> 2510,
							BOTAO_APERTADO -> 2511
							};
	var valor := 1; 
	
	var colCfg := dictionary;
	colCfg["nome"] := struct{titulo := "Material",ordem := 1,largura := 5};
	colCfg["tipo"] := struct{titulo := "Tipo",ordem := 2,largura := 5};
	colCfg["marque"] := struct{titulo := "Selecione",ordem := 3,largura := 2};
	//CriaTabelaGumpPaginada();
	GFSendGump(quem,gump);
endfunction

function DefineLarguraPainel(quantidadeCelulas)
	return larguraCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function DefineAlturaPainel(quantidadeCelulas)
	return alturaCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function MontaJanelaReceita(byref gump , x, y,linha, coluna, receita)
	y := y + 150 * linha;
	x := x + 120 * coluna;
	GFResizePic(gump, x, y, 1579, 120, 150);//5170, 1755,
	GFTooltipText(gump,receita.textoDescItem);
	GFAddButton(gump,x + 15, y + 120,4011,4012);
	GFAddButton(gump,x + 45,y + 120,4029,4030);
	GFAddButton(gump,x + 75,y + 120,4023,4024);
	GFGumpPic(gump,x + 60 ,y + 10,9009); // Pino
	GFTilePic(gump, x + 30,y + 50, CInt(receita.graphic));
	//TODO fazer com que os pinos tenham cor diferente de acordo com a dificuldade de criação do item.
endfunction
