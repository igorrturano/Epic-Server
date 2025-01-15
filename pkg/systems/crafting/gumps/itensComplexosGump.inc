include ":gumps:/include/epicGumps";

const G_LARGURA := 800;
const G_ALTURA := 600;

const G_CELULA_DIVISOR := 12;
const G_MARGEM_ESQUERDA := 10;
const G_MARGEM_SUPERIOR := 10;
const G_ESPACAMENTO := 10;

enum FAIXA_BTN_IDS // Valor inicial para os ids de botões de cada painel.
	COMPLEXOS_CATEGORIAS := 20,
	COMPLEXOS_RECEITAS := 100,
	COMPONENTES_DROPDOWN_INICIAL := 100,
	COMPONENTES_DROPDOWN_OPCOES_INICIAL :=	200,
	COMPONENTES_DROPDOWN_MAX := 10,
	COMPONENTES_TABELA_INICIAL := 1000
endenum

enum BTN_IDS // ids, não variaveis, de botões e entradas de cada painel.
	COMPONENTES_BTN_QTD := 2,
	COMPONENTES_BTN_FABRICAR :=	3
endenum

enum CLASSE_RETORNO_GUMP //RT = RETORNO.
	RT_RECEITAS,
	RT_CATEGORIAS,
	RT_ESQUEMAS,
	RT_BOTAO_MENU
endenum

var alturaCelula :=  G_ALTURA / G_CELULA_DIVISOR;
var larguraCelula :=  G_LARGURA / G_CELULA_DIVISOR;

var espacamentoAplicado := G_ESPACAMENTO * 2;

function MontaItensComplexosGump(quem,byref gump,parametros,byref sessao)
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

	PreencheReceitas(gump , painelReceitasX, 45, painelReceitasY, 55,larguraPainelReceita,alturaPainelReceita,parametros.visaoReceitas[sessao.categoriaSelecionada]);
	
	GFGumpPic(gump,painelReceitasX + larguraPainelReceita / 2 - 102,20,1764); //Placa Titulo Receitas.
	GFHTMLArea(gump,painelReceitasX + larguraPainelReceita / 2 - 102,35,205,20,"<basefont size=55 color=#ffffff><center>Receitas</center>");

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,alturaCelula * 11 + 10, 5054, larguraPainelRodape, alturaPainelRodape);// Painel Rodapé

	return gump;
endfunction

//function MontaComponentesGump(quem,byref gump,parametros,byref sessao)
function MontaComponentesGump(byref gump,byref parametros,byref sessao)
	//TODO tabela com todos os componentes.
	gump := GFECreateGump("GumpComponentes",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});

	var larguraPainelEntrada := DefineLarguraPainel(12);
	var alturaPainelEntrada := DefineAlturaPainel(3);

	var larguraPainelComponentes := DefineLarguraPainel(12);
	var alturaPainelComponentes := DefineAlturaPainel(9);
	var componenteSelecionado := sessao.componenteSelecionado;


	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,G_MARGEM_SUPERIOR + G_ESPACAMENTO, 5054, larguraPainelEntrada, alturaPainelEntrada);
	GFTextLine(gump,G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2,86,"Quantidade:");
	GFTextEntry( gump, G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2 + 70, G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 , 25, 17, 1153, sessao.quantidade ? sessao.quantidade : "1", 2,3);
	
	if(componenteSelecionado)
		GFTextLine(gump,G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 20,86,"Selecionado: " + componenteSelecionado.nome);
		GFAddButton(gump,G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2 + larguraCelula * 10,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2,1147,1148,GF_CLOSE_BTN, 3);
		foreach material in (SplitWords(componenteSelecionado.materiais,","))
			DefinirMateriais(gump,
							G_MARGEM_ESQUERDA+ G_ESPACAMENTO + larguraCelula * 3,
							G_MARGEM_SUPERIOR  + G_ESPACAMENTO + 20 * _material_iter,
							material,
							sessao,
							_material_iter);
			Sleepms(2);
		endforeach
	endif

	var opcoesTabela := dictionary{ 
							MAX_PAGINAS -> 30,
							QTD_LINHA_LIMITE -> 20,
							TITULO_TABELA  -> "Componentes",
							GUMPIC_FUNDO -> 3000,
							BOTAO_APERTADO -> 2103,
							BOTAO_SOLTO -> 2104,
							RECUO_EIXO_Y_BOTAO -> 5
							};

	//TODO Lidar com Paginação.
	CriaTabelaGump(G_MARGEM_ESQUERDA + G_ESPACAMENTO,
					alturaPainelEntrada + G_ESPACAMENTO + alturaCelula/2,
					larguraPainelComponentes,
					parametros.componentes,
					gump,
					0,
					opcoesTabela,
					parametros.configColunas,
					COMPONENTES_TABELA_INICIAL);
	return gump;
endfunction

function DefinirMateriais(byref gump,x,y,material,byref sessao,indice)
	if(indice < 1 || indice > 10 )
		return 0;
	endif
	var gerenciador := sessao.gerenciadorDropDown;
	gerenciador.dropDowns[indice].x := x;
	gerenciador.dropDowns[indice].y := y + 20;
	Print("nomeeee " + gerenciador.dropDowns[indice].valorEscolhido.nome);
	var nome := gerenciador.dropDowns[indice].valorEscolhido.nome ? gerenciador.dropDowns[indice].valorEscolhido.nome : "";

	GFAddButton(gump,x,y,2648,2649,GF_CLOSE_BTN, gerenciador.dropDowns[indice].idBottao);
	GFTextLine(gump,x + 20,y,86, material + ": " + nome);
endfunction

function ControleGump(byref gump,controleX,controleY,gumpX,gumpY,idButaoInicial := 0,descIncremento := "Baixo")
	//TODO Implementar o controle de gump de forma a ser um script autosuficiente.
	GFResizePic(gump, controleX,controleY, 5054, 140, 70);

	var centroCruzX := controleX + 25;
	var centroCruzY := controleY + 25;
	GFAddButton(gump,centroCruzX,centroCruzY - 15,5600,5600,GF_CLOSE_BTN,idButaoInicial);
	GFAddButton(gump,centroCruzX + 15,centroCruzY ,5601,5601,GF_CLOSE_BTN,idButaoInicial + 1);
	GFAddButton(gump,centroCruzX,centroCruzY + 15,5602,5602,GF_CLOSE_BTN, idButaoInicial + 2);
	GFAddButton(gump,centroCruzX - 15,centroCruzY ,5603,5603,GF_CLOSE_BTN, idButaoInicial + 3);

	GFTextLine(gump,centroCruzX + 40,centroCruzY,1258,descIncremento);

	GFAddButton(gump,centroCruzX + 95,6 + centroCruzY - 15 ,2435,2436,GF_CLOSE_BTN, idButaoInicial + 4);
	GFAddButton(gump,centroCruzX + 95,6 + centroCruzY + 15,2437,2438,GF_CLOSE_BTN, idButaoInicial + 5);
	
endfunction

function ClassificaRetornoComplexos(valorRetorno)
	if(valorRetorno > COMPLEXOS_CATEGORIAS && valorRetorno <= COMPLEXOS_RECEITAS)
		return RT_CATEGORIAS;
	elseif(valorRetorno > COMPLEXOS_RECEITAS)
		return RT_RECEITAS;
	else
		return RT_BOTAO_MENU;
	endif
endfunction

function PreencheReceitas(byref gump,x,deslocamentoX,y,deslocamentoY,largura,altura,receitas)

	//TODO Implementar uma forma de lembrar onde estava da ultima vez que abriu o gump. Talvez usando a CProp legado "Craft".
	var page := 1;
	var colunasLinha := 4;
	GFPage(gump,page);
	if(receitas != error && receitas.size() > 0)
		var linha := 0;
		foreach receita in receitas
			MontaJanelaReceita(gump , x + deslocamentoX, y + deslocamentoY,linha, (_receita_iter - 1) % colunasLinha,receita,_receita_iter);

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
		GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10,  G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _categoria_iter) ,4005,4006,GF_CLOSE_BTN, COMPLEXOS_CATEGORIAS + _categoria_iter);
		GFTextLine(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 45,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10 + (30 * _categoria_iter),86,categoria);
		Sleepms(2);
	endforeach
endfunction

function DefineLarguraPainel(quantidadeCelulas)
	return larguraCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function DefineAlturaPainel(quantidadeCelulas)
	return alturaCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function MontaJanelaReceita(byref gump , x, y,linha, coluna, receita, indice)
	y := y + 150 * linha;
	x := x + 120 * coluna;

	var numeroBotoes := 3;
	var baseIdBotao :=  (indice - 1) * numeroBotoes + COMPLEXOS_RECEITAS;

	GFResizePic(gump, x, y, 1579, 120, 150);//5170, 1755,
	GFTooltipText(gump,receita.textoDescItem);
	GFAddButton(gump,x + 15, y + 120,4011,4012,GF_CLOSE_BTN,baseIdBotao + 1);
	GFAddButton(gump,x + 45,y + 120,4029,4030,GF_CLOSE_BTN,baseIdBotao + 2);
	GFAddButton(gump,x + 75,y + 120,4023,4024,GF_CLOSE_BTN,baseIdBotao + 3);
	GFGumpPic(gump,x + 60 ,y + 10,9009); // Pino

	case (receita.uso)
		"One_Handed":
			GFGumpPic(gump,x + 5 ,y + 5,137,0x6c1);
			break;
		"Two_Handed":
		"Dual_Wield":
			GFGumpPic(gump,x + 5 ,y + 5,137,0x6c1);
			GFGumpPic(gump,x + 28 ,y + 5,136,0x6c1);
			break;
		"Ranged":
			GFGumpPic(gump,x + 5 ,y + 5,138,0x6c1);
			break;
		default:
	endcase
	
	GFTilePic(gump, x + 30,y + 50, CInt(receita.graphic));
	//TODO fazer com que os pinos tenham cor diferente de acordo com a dificuldade de criação do item.
endfunction
