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

function EnviaGumpItensComplexos(quem)
	var gump := GFECreateGump("GumpItensComplexos",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});

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
	
	var dummyCategorias := array {"Espadas","Machados","Martelos","Armas de Haste","Armaduras de Aneis","Armadura de Placas","Armadura de Malha"};
	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + larguraPainelCategoria / 2 - 35,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10,70,20,"<basefont size=55 color=#ffffff><center>Categorias</center>");
	foreach dummy in dummyCategorias
		GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10,  G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _dummy_iter) ,4005,4006);
		GFTextLine(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 45,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _dummy_iter),86,dummy);
	endforeach

	var painelReceitasX := G_MARGEM_ESQUERDA + espacamentoAplicado + larguraPainelCategoria;
	var painelReceitasY := G_MARGEM_SUPERIOR + G_ESPACAMENTO ;
	GFResizePic(gump, painelReceitasX, painelReceitasY , 1460, larguraPainelReceita, alturaPainelReceita); //Painel Receitas.
	GFResizePic(gump, painelReceitasX,  painelReceitasY + alturaPainelReceita + espacamentoAplicado, 1755, larguraPainelEsquema, alturaPainelEsquema); //Painel Esquemas.
	
	GFHTMLArea(gump,painelReceitasX + larguraPainelEsquema / 2 - 35,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 15,70,20,"<basefont size=55 color=#ffffff><center>Esquemas</center>");
	GFTextLine(gump,painelReceitasX + 110,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,86,"Nome Receita 1");
	GFAddButton(gump,painelReceitasX + 15,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4011,4012);
	GFAddButton(gump,painelReceitasX + 45,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4020,4021);
	GFAddButton(gump,painelReceitasX + 75,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,4023,4024);

	var dummyReceitas := array {1,2,3,4,5,6,7,8};

	var linhas := 0;
	foreach dummy in dummyReceitas
		MontaJanelaReceita(gump , painelReceitasX + 45, painelReceitasY + 55,linhas, (_dummy_iter - 1) % 4);

		if(_dummy_iter % 4 == 0)
			linhas += 1;
		endif
		Sleepms(2);
	endforeach

	GFGumpPic(gump,painelReceitasX + larguraPainelReceita / 2 - 102,20,1764); //Placa Titulo Receitas.
	GFHTMLArea(gump,painelReceitasX + larguraPainelReceita / 2 - 102,35,205,20,"<basefont size=55 color=#ffffff><center>Receitas</center>");
	GFAddButton(gump,painelReceitasX + 13, painelReceitasY + alturaPainelReceita / 2 ,4014,4015);
	GFAddButton(gump,painelReceitasX + larguraPainelReceita - 45, painelReceitasY + alturaPainelReceita / 2,4005,4006);

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,alturaCelula * 11 + 10, 5054, larguraPainelRodape, alturaPainelRodape);// Painel Rodapé


	GFSendGump(quem,gump);
endfunction

function DefineLarguraPainel(quantidadeCelulas)
	return larguraCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function DefineAlturaPainel(quantidadeCelulas)
	return alturaCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function MontaJanelaReceita(byref gump , x, y,linha, indice)
	y := y + 150 * linha;
	x := x + 120 * indice;
	GFResizePic(gump, x, y, 1579, 120, 150);//5170, 1755,
	GFAddButton(gump,x + 15, y + 120,4011,4012);
	GFAddButton(gump,x + 45,y + 120,4029,4030);
	GFAddButton(gump,x + 75,y + 120,4023,4024);
	GFGumpPic(gump,x + 60 ,y + 10,9009); // Pino
	//TODO fazer com que os pinos tenham cor diferente de acordo com a dificuldade de criação do item.
endfunction
