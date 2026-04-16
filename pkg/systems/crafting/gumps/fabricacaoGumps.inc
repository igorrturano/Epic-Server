//Author: Deiro || Rafael Santos

include ":gumps:/include/epicGumps";
include ":crafting:include/util/fabricacaoConstants";
include "include/string";
include "/include/epicConstants";

const G_LARGURA := 800;
const G_ALTURA := 600;

const G_CELULA_DIVISOR := 12;
const G_MARGEM_ESQUERDA := 10;
const G_MARGEM_SUPERIOR := 10;
const G_ESPACAMENTO := 10;

enum FAIXA_BTN_IDS // Valor inicial para os ids de botões de cada painel.
	COMPLEXOS_CATEGORIAS := 20,
	COMPLEXOS_RECEITAS := 100,
	COMPLEXOS_ESQUEMAS := 600,
	COMPLEXOS_ESQUEMAS_ESCOLHA_GM_CPNT := 660,
	COMPLEXOS_DROPDOWN_INICIAL := 3000,
	COMPLEXOS_DROPDOWN_OPCOES_INICIAL :=	5000,
	COMPONENTES_DROPDOWN_INICIAL := 100,
	COMPONENTES_DROPDOWN_OPCOES_INICIAL :=	200,
	COMPONENTES_TABELA_INICIAL := 1000
endenum

enum MAXIMO_BTN_IDS // Valores máixmos dos ids.
	COMPLEXOS_DROPDOWN_MAX := 80,
	COMPONENTES_DROPDOWN_MAX := 10,
endenum

enum BTN_IDS // ids, não variaveis, de botões e entradas de cada painel.
	COMPONENTES_INPT_QTD := 2,
	COMPONENTES_BTN_FABRICAR :=	3,
	COMPONENTES_BTN_ESFORCO_MAIS := 4,
	COMPONENTES_BTN_ESFORCO_MENOS := 5,
	COMPONENTES_BTN_AVALIAR := 6,
	COMPONENTES_INPT_FILTRO_NOME := 7,
	COMPONENTES_INPT_FILTRO_ENCAIXE := 8,
	COMPONENTES_BTN_FILTRAR := 9,
	COMPONENTES_BTN_LIMPA_FILTRO := 10,
	COMPONENTES_BTN_LIGA_DESL_FILTRO_MEMORIA := 11,
	COMPONENTES_BTN_LIMPAR_MEMORIA := 12,
	COMPONENTES_CHK_BOX_RECICLAR := 13,
	COMPLEXOS_ESQUEMAS_INPT_NOME := 2,
	COMPLEXOS_ESQUEMAS_BTN_SALVAR:= 3,
	COMPLEXOS_BTN_AJUDA := 4,
	COMPLEXOS_INPT_QTD := 5,
	COMPLEXOS_BTN_RCT_PROX_PAGINA := 6,
	COMPLEXOS_BTN_RCT_ANT_PAGINA := 7,
	COMPLEXOS_BTN_MEMORIZAR_COMPONENTES := 8,
	COMPLEXOS_BTN_LIMPAR_MEMORIA := 9,
	COMPLEXOS_BTN_CANCELAR := 10,
	COMPLEXOS_BTN_OKAY_GM_CMPNT := 11,
	COMPLEXOS_BTN_FABRICAR_GM_CMPNT := 12,
	COMPLEXOS_BTN_ESQ_PROX_PAGINA := 13,
	COMPLEXOS_BTN_ESQ_ANT_PAGINA := 14,
	COMPLEXOS_BTN_REPARAR := 15,
endenum

enum CLASSE_RETORNO_GUMP_COMPLX // (RT = RETORNO) Classificação do retorno do Gump de intes complexos.
	RT_RECEITAS,
	RT_CATEGORIAS,
	RT_ESQUEMAS_CPNT,
	RT_ESQUEMAS,
	RT_ESQUEMAS_SALVAR,
	RT_CANCELADO_ESQUEMA_COMPLX,
	RT_CANCELADO_GUMP_GM_COMPONENTE,
	RT_CANCELADO_COMPLX,
	RT_ESCOLHA_DROPDOWN_CPX,
	RT_ESCOLHA_DD_OPCOES_CPX,
	RT_GM_COMPONENTE_SELECIONADO,
	RT_GM_ESQUEMA_FABRICAR,
	RT_ABRE_GM_COMPONENT,
	RT_RCT_PROX_PAGINA,
	RT_RCT_ANT_PAGINA,
	RT_ESQ_PROX_PAGINA,
	RT_ESQ_ANT_PAGINA,
	RT_MEMORIZAR_COMPONENTES,
	RT_LIMPAR_MEMORIA,
	RT_AJUDA_CPX,
	RT_REPARAR
endenum

enum CLASSE_RETORNO_GUMP_COMPNT //(RT = RETORNO) Classificação do retorno do Gump de Componentes.
	RT_ESCOLHA_DROPDOWN,
	RT_ESCOLHA_DD_OPCOES,
	RT_ESCOLHA_COMPONENTE,
	RT_BTN_FABRICAR,
	RT_BTN_ESFORCO_MAIS,
	RT_BTN_ESFORCO_MENOS,
	RT_BTN_AVALIAR,
	RT_BTN_FILTRAR,
	RT_BTN_LIMPA_FILTRO,
	RT_CHK_BOX_RECICLAR,
	RT_LIMPAR_MEMORIA_COMPNT,
	RT_FILTRO_MEMORIA_LIG_DES,
	RT_CANCELADO
endenum

enum GUMP_ATUAL_COMPLEXOS //Definição do Gump atual da sessão no gump de itens complexos.
	COMPLEXOS,
	ESQUEMAS,
	GM_COMPONENTE
endenum

enum ACAO_COMPLEXOS //Definição do Gump atual da sessão no gump de itens complexos.
	ADICIONAR,
	EDITAR
endenum

enum ABA_AJUDA
	FABRICACAO_AJUDA_ABA_GERAL := 1,
	FABRICACAO_AJUDA_ABA_COMPONENTES := 2,
	FABRICACAO_AJUDA_ABA_MONTAGEM := 3,
	FABRICACAO_AJUDA_ABA_TRATAMENTO := 4,
endenum

var alturaCelula :=  G_ALTURA / G_CELULA_DIVISOR;
var larguraCelula :=  G_LARGURA / G_CELULA_DIVISOR;
var espacamentoAplicado := G_ESPACAMENTO * 2;

var coresClassesReceitas := array{0,737,727,717,707,797};
const GUMPIC_ID_PROFISSOES_DEFAULT := 5581;
var gumpicIdProfissoes := dictionary {
							"Alchemy" -> 5557,
							"Blacksmithy" -> 5555,
							"Carpentry" -> 5559,
							"BowcraftFletching" -> 5551,
							"Tailoring" -> 5579,
							"Tinkering" -> 5571
						};

function MontaItensComplexosGump(byref gump,parametros,byref sessao)
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
	
	PreencheCategorias(gump,larguraPainelCategoria,parametros, sessao);

	var painelReceitasX := G_MARGEM_ESQUERDA + espacamentoAplicado + larguraPainelCategoria;
	var painelReceitasY := G_MARGEM_SUPERIOR + G_ESPACAMENTO ;
	GFResizePic(gump, painelReceitasX, painelReceitasY , 1460, larguraPainelReceita, alturaPainelReceita); //Painel Receitas.
	
	if(sessao.receitaSelecionada.receita != "")
		GFResizePic(gump, painelReceitasX,  painelReceitasY + alturaPainelReceita + espacamentoAplicado, 1755, larguraPainelEsquema, alturaPainelEsquema); //Painel Esquemas.
		GFHTMLArea(gump,painelReceitasX + larguraPainelEsquema / 2 - 35,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 15,70,20,"<basefont size=55 color=#ffffff><center>Esquemas</center>");
	endif
	
	PreencheReceitas(gump , painelReceitasX, 45, painelReceitasY, 55,larguraPainelReceita,alturaPainelReceita,parametros.visaoReceitas[sessao.categoriaSelecionada], sessao, parametros.receitasPorPagina);
	PreencheEsquemas (gump,painelReceitasX + 15,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,sessao);

	GFGumpPic(gump,painelReceitasX + larguraPainelReceita / 2 - 102,20,1764); //Placa Titulo Receitas.
	GFHTMLArea(gump,painelReceitasX + larguraPainelReceita / 2 - 102,35,205,20,"<basefont size=55 color=#ffffff><center>Receitas</center>");

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,alturaCelula * 11 + 10, 5054, larguraPainelCategoria, alturaPainelRodape);// Rodapé Categorias
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + (larguraPainelCategoria / 2) - 31,alturaCelula * 11 + 14,241,242, GF_CLOSE_BTN, COMPLEXOS_BTN_CANCELAR);
	if(sessao.modoGm)
		GFTextLine(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO +  cint(larguraCelula * 3),alturaCelula * 11 + 14,149,"*Modo GM*");
	endif

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + cint(larguraCelula * 8.4),alturaCelula * 11 + 10, 5054, cint(larguraCelula * 1.3), alturaPainelRodape);	
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + cint(larguraCelula * 8.5) - 2, alturaCelula * 11 + 17,1537,1538, GF_CLOSE_BTN, COMPLEXOS_BTN_REPARAR);// Botão reparar
	GFTooltipText(gump,"Reparar objeto.");
	
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + cint(larguraCelula * 8.9) - 2, alturaCelula * 11 + 17,1533,1534, GF_CLOSE_BTN, COMPLEXOS_BTN_MEMORIZAR_COMPONENTES);// Botão memoriza componentes
	GFTooltipText(gump,"Memorizar encaixes de componente.");
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 23 + cint(larguraCelula * 8.9),alturaCelula * 11 + 17,1541,1542, GF_CLOSE_BTN, COMPLEXOS_BTN_LIMPAR_MEMORIA);// Botão limpar memória.
	GFTooltipText(gump,"Esquecer encaixes memorizados.");

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + cint(larguraCelula * 9.8) ,alturaCelula * 11 + 10, 5054, cint(larguraCelula * 1.9), alturaPainelRodape);// Rodapé Quantidade
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO  + 11 + cint(larguraCelula * 10.8) ,alturaCelula * 11 + 13, 9350, cint(larguraCelula * 0.6), alturaPainelRodape - 6);// Rodapé Quantidade Input
	GFTextLine(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO - 4  + larguraCelula * 10,alturaCelula * 11 + 17,86,"Quantidade:");
	GFTextEntry( gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 3 + larguraCelula * 11,alturaCelula * 11 + 17, 25, 17, 1153, sessao.quantidade ? sessao.quantidade : "1", COMPLEXOS_INPT_QTD,3);
	return gump;
endfunction

function PreencheEsquemas(byref gump, x,y, sessao)
	var deslocamentoY := 20;
	var deslocamentoX := 280;
	var coluna := 0;
	var indice := 0;

	foreach esquema in (sessao.receitaSelecionada.esquemas)
		var numeroBotoes := 3;
		var baseIdBotao :=  indice * numeroBotoes + COMPLEXOS_ESQUEMAS;

		var linha := indice % 3;
		if(indice != 0 && linha == 0)
			++coluna;
		endif
		GFTextLine(gump,x + 95 + deslocamentoX * coluna,y + deslocamentoY * linha,86,esquema.nome);
		GFAddButton(gump,x + deslocamentoX * coluna,y + deslocamentoY * linha,4020,4021, GF_CLOSE_BTN, baseIdBotao + 2);
		GFTooltipText(gump,"Excluir.");
		GFAddButton(gump,x + 30 + deslocamentoX * coluna,y + deslocamentoY * linha,4011,4012, GF_CLOSE_BTN, baseIdBotao + 1);
		GFTooltipText(gump,"Editar.");
		GFAddButton(gump,x + 60 + deslocamentoX * coluna,y + deslocamentoY * linha,4023,4024, GF_CLOSE_BTN, baseIdBotao + 3);
		GFTooltipText(gump,"Montar.");

		++indice;
	endforeach
endfunction

function MontaEsquemaGump(byref gump,byref sessao,byref parametros)
	gump := GFECreateGump("GumpEsquema",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});
	var margemEsquerdaLocal := G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10;
	var margemSuperiorLocal := G_MARGEM_ESQUERDA + G_ESPACAMENTO;

	var larguraEncaixeHeader := DefineLarguraPainel(12);
	var alturaEncaixeHeader := DefineAlturaPainel(1);

	var larguraPainelInfo := DefineLarguraPainel(12);
	var alturaPainelInfo := DefineAlturaPainel(4.8);
	
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO, G_MARGEM_SUPERIOR + G_ESPACAMENTO, 83, larguraPainelInfo, alturaPainelInfo);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10, G_MARGEM_SUPERIOR + G_ESPACAMENTO + 15, 3500, larguraPainelInfo - 20, cint(alturaCelula *  1.4));

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,CInt(alturaCelula * 4.9), 5054, larguraEncaixeHeader, alturaEncaixeHeader);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + (larguraCelula * 4.4) - 5,CInt(alturaCelula * 4.9) + 4, 3000, larguraCelula * 3, alturaEncaixeHeader - 7);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 13,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 15 + cint(alturaCelula * 1.5), 30546, larguraCelula * 3, cint(alturaCelula * 2.2) + 6);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 8 + cint(larguraCelula * 3.2),G_MARGEM_SUPERIOR + G_ESPACAMENTO + 15 + cint(alturaCelula * 1.5), 3000, cint(larguraCelula * 8.1) + 5, cint(alturaCelula * 2.2) + 6);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 18 + cint(larguraCelula * 3.2),G_MARGEM_SUPERIOR + G_ESPACAMENTO + 40 + cint(alturaCelula * 1.5), 3000, cint(larguraCelula * 6.8), cint(alturaCelula * 1.6) + 6);

	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO,CInt(alturaCelula * 4.9) + 7,larguraEncaixeHeader,alturaEncaixeHeader,"<basefont color=#024d95><center>Encaixes de Componente</center>");
	GFHTMLArea(gump,margemEsquerdaLocal + 15, margemSuperiorLocal + 5 + Cint(alturaCelula * 0.5),larguraEncaixeHeader,alturaEncaixeHeader,"<basefont color=#423E3D>Receita:</basefont><basefont color=#907201> {}".format(sessao.receitaSelecionada.receita.desc));
	GFHTMLArea(gump,margemEsquerdaLocal + 15, margemSuperiorLocal + Cint(alturaCelula),larguraEncaixeHeader,alturaEncaixeHeader,"<basefont color=#423E3D>Nome do Esquema:");
	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 8 + cint(larguraCelula * 3.2),G_MARGEM_SUPERIOR + G_ESPACAMENTO + 20 + cint(alturaCelula * 1.5), cint(larguraCelula * 6.8),alturaEncaixeHeader,"<basefont color=#423E3D><center>Descrição Excepcional (Não dispponível [WIP])</center>");
	//GFTextLine(gump,margemEsquerdaLocal + 15, margemSuperiorLocal + 5 + Cint(alturaCelula * 0.5),1153,"Receita: {}".format(sessao.receitaSelecionada.receita.desc));

	var margemEsquerdaValoresModificadores := margemEsquerdaLocal + 80;
	var margemSuperiorModificadores := margemSuperiorLocal + Cint(alturaCelula * 2.5);

	GFTextLine(gump,margemEsquerdaLocal + 40,margemSuperiorModificadores -25,89,"Modificadores Gerais".format(sessao.receitaSelecionada.esquemaSelecionado.modificadorPesoTotal));

	var pesoTotalGeral := sessao.receitaSelecionada.esquemaSelecionado.modificadorPesoTotal;
	if(pesoTotalGeral  != 0)
		GFTextLine(gump,margemEsquerdaLocal + 10,margemSuperiorModificadores,1153,"Peso Total:".format(sessao.receitaSelecionada.esquemaSelecionado.modificadorPesoTotal));
		var corSinalPeso := CriaCorSinal(pesoTotalGeral,true);
		GFTextLine(
			gump,
			margemEsquerdaValoresModificadores,
			margemSuperiorModificadores,
			corSinalPeso.cor,
			"{}%".format(pesoTotalGeral));
	endif

	var hpTotalGeral := sessao.receitaSelecionada.esquemaSelecionado.modificadorHpTotal;
	if(hpTotalGeral != 0)
		GFTextLine(gump,margemEsquerdaLocal + 10, margemSuperiorModificadores + 20,1153,"Hp Total:".format(sessao.receitaSelecionada.esquemaSelecionado.modificadorHpTotal));
		var corSinalHp := CriaCorSinal(hpTotalGeral);
		GFTextLine(gump,
		margemEsquerdaValoresModificadores,
		margemSuperiorModificadores + 20,
		corSinalHp.cor,
		"{}{}".format(corSinalHp.sinal,hpTotalGeral));
	endif
	 
	if(sessao.acao == ADICIONAR)
		GFTextEntry(gump,margemEsquerdaLocal  + (larguraCelula * 2), margemSuperiorLocal + Cint(alturaCelula), 120, 17, 1153, sessao.receitaSelecionada.esquemaSelecionado.nome, 2,40);
		if(sessao.modoGm)
			GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + Cint(larguraCelula * 10.4),Cint(alturaCelula * 3),2311,2312,GF_CLOSE_BTN,COMPLEXOS_BTN_FABRICAR_GM_CMPNT);
			GFTooltipText(gump,"Fabricar item Pronto sem salvar.");
		endif
	elseif(sessao.acao == EDITAR)
		GFTextLine(gump,margemEsquerdaLocal  + (larguraCelula * 2), margemSuperiorLocal + Cint(alturaCelula),1153,sessao.receitaSelecionada.esquemaSelecionado.nome);
	endif
	
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + Cint(larguraCelula * 10.4),Cint(alturaCelula * 4),2460,2461,GF_CLOSE_BTN,COMPLEXOS_ESQUEMAS_BTN_SALVAR);
	GFTooltipText(gump,"Salvar Receita.");
	//Print("Componentes: " + sessao);
	var posx := 1;
	var paginador := MontaPaginador( sessao.paginador.esquemas.pagina,parametros.encaixesPorPagina);
	var componentesNecessarios := sessao.receitaSelecionada.receita.componentesNecessarios;

	for indice := paginador.indicePaginaAtual to (paginador.indicePaginaSeguinte - 1)
		if(indice > componentesNecessarios.size())
			break;
		endif
		
		var componente :=  componentesNecessarios[indice];
		
		PreencheEncaixes(gump, margemEsquerdaLocal + (larguraCelula * 4) * ((indice - 1) % parametros.encaixesPorPagina), margemSuperiorLocal + Cint(alturaCelula * 5.3), indice,componente,sessao);
		
		if(componentesNecessarios.size() >= paginador.indicePaginaSeguinte)
			GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + larguraCelula * 11 + 10,Cint(alturaCelula * 4.9) + 5,4005,4006,GF_CLOSE_BTN,COMPLEXOS_BTN_ESQ_PROX_PAGINA);
		endif
		if(1 >= paginador.indicePaginaAnterior)
			GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 5,CInt(alturaCelula * 4.9) + 5,4014,4015,GF_CLOSE_BTN,COMPLEXOS_BTN_ESQ_ANT_PAGINA);
		endif
		Sleepms(2);
	endfor
endfunction

function MontaComponentesGump(byref gump,byref parametros,byref sessao)
	gump := GFECreateGump("GumpComponentes",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});

	var larguraPainelEntrada := DefineLarguraPainel(12);
	var alturaPainelEntrada := DefineAlturaPainel(3);

	var larguraPainelComponentes := DefineLarguraPainel(12);
	var alturaPainelComponentes := DefineAlturaPainel(9);
	var componenteSelecionado := sessao.componenteSelecionado;
	var margemBaseEsquerda := G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2;

	GFResizePic(gump, margemBaseEsquerda - 10,G_MARGEM_SUPERIOR + G_ESPACAMENTO , 9270 , larguraPainelEntrada, alturaPainelEntrada);
	GFResizePic(gump, margemBaseEsquerda - 2,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 5, 9250, CInt(larguraCelula * 4.7), Cint(alturaCelula * 2.3) + 2);//input 30546
	GFResizePic(gump, margemBaseEsquerda + CInt(larguraCelula * 4.8),G_MARGEM_SUPERIOR + G_ESPACAMENTO + 5, 9250, CInt(larguraCelula * 3.8), Cint(alturaCelula * 2.3) + 2);
	GFGumpPic(gump, margemBaseEsquerda + CInt(larguraCelula * 8.7) - 3,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 10, 2070);
	GFGumpPic(gump, margemBaseEsquerda + CInt(larguraCelula * 9) + 12 ,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 4 + CINT(alturaCelula/1.6), 2445);
	GFGumpPic(gump, margemBaseEsquerda + CInt(larguraCelula * 9) + 12 ,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 24 + CINT(alturaCelula/1.6), 2445);

	GFTextEntry( gump,  margemBaseEsquerda + CInt(larguraCelula * 9) + 17 , G_MARGEM_SUPERIOR + G_ESPACAMENTO + 5 + CINT(alturaCelula/1.6), 100, 17, 1153, sessao.filtros.nome, COMPONENTES_INPT_FILTRO_NOME); // input de filtro nome
	GFTooltipText(gump,"Filtro por nome.");
	GFTextEntry( gump,  margemBaseEsquerda + CInt(larguraCelula * 9) + 17 ,G_MARGEM_SUPERIOR + G_ESPACAMENTO + 25 + CINT(alturaCelula/1.6), 100, 17, 1153, sessao.filtros.encaixe , COMPONENTES_INPT_FILTRO_ENCAIXE); // input de filtro nome
	GFTooltipText(gump,"Filtro por encaixe.");

	var margemEsquerdaBotoesFiltro := margemBaseEsquerda + CInt(larguraCelula * 9.1);
	var margemTopoBotoesFiltro := G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 73;

	GFAddButton(gump,margemEsquerdaBotoesFiltro ,margemTopoBotoesFiltro,1535,1536,GF_CLOSE_BTN, COMPONENTES_BTN_LIMPA_FILTRO);
	GFTooltipText(gump,"Limpa as caixas de filtro da tabela.");
	GFAddButton(gump,margemEsquerdaBotoesFiltro + 25,margemTopoBotoesFiltro,1537,1538,GF_CLOSE_BTN, COMPONENTES_BTN_FILTRAR);
	GFTooltipText(gump,"Filtrar Tabela de Componentes.");
	GFAddButton(gump, margemEsquerdaBotoesFiltro + 50, margemTopoBotoesFiltro,1541,1542, GF_CLOSE_BTN, COMPONENTES_BTN_LIMPAR_MEMORIA);// Botão memoriza componentes
	GFTooltipText(gump,"Esquecer encaixes memorizados.");

	if(sessao.isFiltroMemoria)
		GFAddButton(gump, margemEsquerdaBotoesFiltro + 75,margemTopoBotoesFiltro,1534,1533, GF_CLOSE_BTN, COMPONENTES_BTN_LIGA_DESL_FILTRO_MEMORIA);// Botão limpar memória. marcado
		GFTooltipText(gump,"Filtrar por encaixes memorizados. {Ligado}");
	else
		GFAddButton(gump,margemEsquerdaBotoesFiltro + 75,margemTopoBotoesFiltro,1533,1534, GF_CLOSE_BTN, COMPONENTES_BTN_LIGA_DESL_FILTRO_MEMORIA);// Botão limpar memória. desmarcado
		GFTooltipText(gump,"Filtrar por encaixes memorizados. {Desligado}");
	endif
	
	GFTextLine(gump, margemBaseEsquerda + 15,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 24,1254,"Quantidade:");
	GFTextEntry( gump, margemBaseEsquerda + 85, G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 24 , 25, 17, 1254, sessao.quantidade ? sessao.quantidade : "1", COMPONENTES_INPT_QTD,3);
	GFPicTiled( gump, margemBaseEsquerda + 85, G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 40 , 49, 3, 9101);
	GFTextLine(gump,margemBaseEsquerda + 15,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2  + 6,1254,"Selecionado: " + (componenteSelecionado.nome ? componenteSelecionado.nome : "Nenhum"));

	if(componenteSelecionado)
		var deveReciclarCheckBoxMarcado := sessao.deveReciclar ? 6440 : 6439;
		var deveReciclarCheckBoxDesmarcado := sessao.deveReciclar ? 6439 : 6440;
		GFAddButton(gump,margemBaseEsquerda +  10 + cint(larguraCelula * 3.5),G_MARGEM_SUPERIOR + 10 + G_ESPACAMENTO + CInt(alturaCelula * 1.5) ,2311,2312,GF_CLOSE_BTN, COMPONENTES_BTN_FABRICAR);
		GFTextLine(gump,margemBaseEsquerda + 15,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 44,687,"Esforço:  " + AvaliaEsforço(sessao));
		GFAddButton(gump,margemBaseEsquerda + 64,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 44,2435,2436,GF_CLOSE_BTN, COMPONENTES_BTN_ESFORCO_MAIS);
		GFTooltipText(gump,"Aumentar o esforço produz componentes de melhor qualidade.");
		GFAddButton(gump,margemBaseEsquerda + 64 ,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 54,2437,2438,GF_CLOSE_BTN, COMPONENTES_BTN_ESFORCO_MENOS);
		GFTooltipText(gump,"Diminuir o esforço produz componentes de menor qualidade.");
		GFTextLine(gump,margemBaseEsquerda + 20 + larguraCelula * 3,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 44,687,"Reciclar");
		GFAddButton(gump,margemBaseEsquerda + 20 +  cint(larguraCelula * 3.8), G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 44,deveReciclarCheckBoxMarcado,deveReciclarCheckBoxDesmarcado,GF_CLOSE_BTN, COMPONENTES_CHK_BOX_RECICLAR);
		GFTooltipText(gump,"Quando marcado, apenas o melhor componente será produzido e o resto será reciclado.");
		GFAddButton(gump, margemEsquerdaBotoesFiltro + 100,margemTopoBotoesFiltro,1531,1532,GF_CLOSE_BTN, COMPONENTES_BTN_AVALIAR);
		GFTooltipText(gump,"Avaliar Material.");
		GFTextLine(gump,margemBaseEsquerda + 15,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 64,687,"Perícia Usada: " + componenteSelecionado.pericia);
		GFTextLine(gump,margemBaseEsquerda + 15,G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 + 83,687,"Principal Material: " + componenteSelecionado.selecione.materialPrincipal);

		var dictMateriais := componenteSelecionado.selecione.materiais;
		
		var margemXMateriais := G_MARGEM_ESQUERDA+ G_ESPACAMENTO + CINT(larguraCelula * 5.2) - 1;
		var margemYMateriais := G_MARGEM_SUPERIOR  + G_ESPACAMENTO + 16;

		GFTextLine(gump,margemXMateriais + 17,margemYMateriais,904,"Qtd");
		GFGumpPic(gump,margemXMateriais + 10,margemYMateriais + 18,96);
		GFTextLine(gump,margemXMateriais + 57,margemYMateriais,904,"Tipo");
		GFGumpPic(gump,margemXMateriais + 27,margemYMateriais + 18,96);
		
		foreach material in (dictMateriais.keys())

			DefinirMateriais(gump,
							margemXMateriais,
							G_MARGEM_SUPERIOR  + G_ESPACAMENTO + 20 * _material_iter + 20,
							material,
							componenteSelecionado.selecione.materialPrincipal,
							dictMateriais[material],
							sessao,
							_material_iter);
			Sleepms(2);
		endforeach
	endif

	var opcoesTabela := dictionary{ 
							MAX_PAGINAS -> 30,
							QTD_LINHA_LIMITE -> 17,
							TITULO_TABELA  -> "Componentes",
							GUMPIC_FUNDO -> 3000,
							BOTAO_APERTADO -> 2103,
							BOTAO_SOLTO -> 2104,
							RECUO_EIXO_Y_BOTAO -> 5
							};

	//TODO Lidar com Paginação.
	CriaTabelaGumpPaginada(G_MARGEM_ESQUERDA + G_ESPACAMENTO,
					alturaPainelEntrada + G_ESPACAMENTO + alturaCelula/2,
					larguraPainelComponentes,
					sessao.componentes,
					gump,
					opcoesTabela,
					parametros.configColunas,
					COMPONENTES_TABELA_INICIAL);
	return gump;
endfunction

function DefinirMateriais(byref gump,x,y,material, materialPrincipal, infoOpcao,byref sessao,indice)
	if(indice < 1 || indice > 10 )
		return 0;
	endif
	var gerenciador := sessao.gerenciadorDropDown;
	var dropDownAtual := gerenciador.BuscaDropDownPorPosicao(indice);
	dropDownAtual.AtribuiX(x);
	dropDownAtual.AtribuiY(y + 20);
	var nome := dropDownAtual.PegaValorEscolhido().nome ? dropDownAtual.PegaValorEscolhido().nome : "";
	var isPrincipal := material == materialPrincipal;
	var corTexto := isPrincipal ? "<basefont size=55 color=#FFD336>" : "<basefont size=55 color=#B7B7B8>";
	var corValor := "<basefont size=55 color=#83C542>";
	var textoPossiveis := "" ;
	
	foreach possivel in (infoOpcao.possiveis)
		Sleepms(2);
		textoPossiveis += "<br>{}{}".format(WHITE,possivel.name);
	endforeach
	
	GFAddButton(gump,x,y,2648,2649,GF_CLOSE_BTN, dropDownAtual.PegaIdBottao());
	GFHTMLArea(gump,x + 25,y,300,20,"{}{}".format(corTexto,infoOpcao.quantidade));
	GFHTMLArea(gump,x + 50,y,300,20,"{}{}: {}{}".format(corTexto,material,corValor,nome));
	GFTooltipText(gump,"{}Possíveis: {}".format(AMARELO_CLARO,textoPossiveis));
endfunction

function MontaTratamentoGump(x, y, byref sessao)
	var largura := 480;
	var altura := 200;

	var alturaCelula := altura / G_CELULA_DIVISOR;
	var larguraCelula := largura / G_CELULA_DIVISOR;

	var gump := GFCreateGump(0,0,largura,altura);
	GFMovable(gump, 0);

	GFResizePic(gump, x, y, 39925, largura, altura );
	GFResizePic(gump, x+ 20, y + 20, 3500, 437, 160 );
	GFResizePic(gump, x + larguraCelula - 5, y - 3 + alturaCelula * 2, 302, 105, 140 ); // Painel ImagemPericia
	GFGumpPic(gump,x - 32 + larguraCelula * 2,y - 20 + alturaCelula * 5,1417);//Borda Imagem Pericia
	GFResizePic(gump, x - 40 + larguraCelula* 5, y + 5 + alturaCelula * 4, 302, 279, 70 ); // Painel Barras
	GFHTMLArea(gump,x - 40 + larguraCelula* 5,y + 5 + alturaCelula * 2,300,20,"<basefont size=55 color=#0667e3><center>{}</center>".format(sessao.nomeItem));
	GFGumpPic(gump,x + 15 + larguraCelula* 5,y + 5 + alturaCelula * 3,96); // Underline nome item
	if(sessao.ciclos)
		if(sessao.ultimoSucesso)
			GFGumpPic(gump,x - 20 + larguraCelula* 5,y + 20 + alturaCelula * 4,6425);// Seta Maior
		else
			GFGumpPic(gump,x - 20 + larguraCelula* 5,y + 20 + alturaCelula * 4,6426);// Seta Maior vermelha
		endif
	endif

	if(cint(sessao.ciclos) % 2 == 0)
		GFGumpPic(gump,x + 25 + larguraCelula* 5,y + 30 + alturaCelula * 4,1608);// Sinalizador
	else
		GFGumpPic(gump,x + 25 + larguraCelula* 5,y + 30 + alturaCelula * 4,1606);// Sinalizador preto
	endif

	GFGumpPic(gump,x - 20 + larguraCelula* 7,y + 10 + alturaCelula * 5,9750); // barra vazia cima
	GFTooltipText(gump,"Sucessos: {}/{}".format(sessao.sucessos, sessao.sucessosNecessarios));
	var larguraBarraSucesso:=  CInt(96 * (CDbl(sessao.sucessos) / CDbl(sessao.sucessosNecessarios)));
	GFPicTiled(gump, x - 20 + larguraCelula* 7,y + 10 + alturaCelula * 5,larguraBarraSucesso != 0 ? larguraBarraSucesso : 1, 8,9751);

	GFGumpPic(gump,x - 20 + larguraCelula* 7,y + 30 + alturaCelula * 5,9750); // barra vazia baixo
	GFTooltipText(gump,"Falhas: {}/{}".format(sessao.falhas, sessao.maximoFalhas));
	var larguraBarraFalha:=  CInt(96 * (CDbl(sessao.falhas) / CDbl(sessao.maximoFalhas)));
	GFPicTiled(gump, x - 20 + larguraCelula* 7,y + 30 + alturaCelula * 5,larguraBarraFalha != 0 ? larguraBarraFalha : 1 , 8,9753);

	GFGumpPic(gump,cint(x - 28 + larguraCelula* 10),y + 12 + alturaCelula * 5,1594); // espaço porcentagem
	var tooltipChance := "Chance Base: {}<br>".format(cint(sessao.dificuldade.chanceBase));
	
	foreach modificador in (sessao.dificuldade.modificadoresChance)
		Sleepms(2);
		tooltipChance += modificador.origem.format(modificador.valor);
	endforeach
	GFTooltipText(gump,tooltipChance);
	var gumpIdProfissaoPressed := sessao.gumpIdProfissaoPressed ? sessao.gumpIdProfissaoPressed : GUMPIC_ID_PROFISSOES_DEFAULT;
	var gumpIdProfissaoUnpressed := sessao.gumpIdProfissaoUnpressed ? sessao.gumpIdProfissaoUnpressed : GUMPIC_ID_PROFISSOES_DEFAULT;

	GFHTMLArea(gump,cint(x - 24 + larguraCelula* 10),y + 15 + alturaCelula * 5,40,30,"<basefont size=55 color=#016210><center>{}%</center>".format(cint(sessao.dificuldade.chanceTotal)));
	GFAddButton(gump,x - 24 + larguraCelula * 2, y - 10 + alturaCelula * 5,gumpIdProfissaoPressed,gumpIdProfissaoUnpressed,GF_CLOSE_BTN, 6); // Botao ajuda pericia.
	GFAddButton(gump,x + larguraCelula * 4, cint(y - 10 + alturaCelula * 9.9) ,2119,2120,GF_CLOSE_BTN, 3);
	if(sessao.fazerTodos)
		GFHTMLArea(gump,x + cint(larguraCelula) * 6, cint(y - 10 + alturaCelula * 9.9),180,30,"<basefont size=55 color=#000000><right>Produzindo Semelhantes.</right>");
		GFAddButton(gump,x + cint(larguraCelula * 10.3), cint(y - 10 + alturaCelula * 9.5) , 9793 , 9792,GF_CLOSE_BTN, 4); //Checkbox de fazer todos
	else
		GFHTMLArea(gump,x + cint(larguraCelula) * 6.5, cint(y - 10 + alturaCelula * 9.9),160,30,"<basefont size=55 color=#000000><right>Produzindo Este.</right>");
		GFAddButton(gump,x + cint(larguraCelula * 10.3), cint(y - 10 + alturaCelula * 9.5) ,9792, 9793,GF_CLOSE_BTN, 4); //Checkbox de fazer todos
	endif
	GFAddButton(gump,cint(x + larguraCelula * 10.6), y + alturaCelula * 2,9780,9781,GF_CLOSE_BTN, 2); // botao de reposicionamento do gump
	return gump;
endfunction

function PreencheEncaixes(byref gump, x, y, indice,encaixeComponente,sessao)
	var larguraLocal := larguraCelula * 3.7;
	var alturaPainel := alturaCelula * 3;
	var alturaLinha := 20;
	var componenteEscolhido := sessao.receitaSelecionada.esquemaSelecionado.componentesEscolhidos[indice];
	var componenteNecessario := sessao.receitaSelecionada.receita.componentesNecessarios[indice];
	var isHpModificado := componenteEscolhido.propriedades.modificadorHp != 0 && TypeOfInt(componenteEscolhido.propriedades.modificadorHp) != OT_UNINIT;
	var isPesoModificado := componenteEscolhido.propriedades.modificadorPeso != 0 && TypeOfInt(componenteEscolhido.propriedades.modificadorPeso) != OT_UNINIT;
	
	var quantidadeEfeitosAdicionais := componenteEscolhido.propriedades.efeitosAdicionais.size();

	if(quantidadeEfeitosAdicionais > 0 || isHpModificado || isPesoModificado)
		alturaPainel := alturaPainel + alturaLinha * (quantidadeEfeitosAdicionais ? quantidadeEfeitosAdicionais : 0);
		alturaPainel := AumentaPainelSe(isHpModificado,alturaPainel,alturaLinha);
		alturaPainel := AumentaPainelSe(isPesoModificado,alturaPainel,alturaLinha);
	endif
	alturaPainel := AumentaPainelSe(componenteEscolhido,alturaPainel,alturaLinha);

	GFResizePic(gump, x - 10,y, 3600, larguraLocal, alturaPainel - 10);
	GFPicTiled(gump, x,y + 10, larguraLocal - 20, alturaPainel - 30,40012);
	GFPicTiled(gump, x, y + 50, larguraLocal - 20, 2, 30002); // 2700 
	if(componenteEscolhido)
		GFPicTiled(gump, x, y + 110, larguraLocal - 20, 2, 30002); // 2700 
	else
		GFPicTiled(gump, x, y + 90, larguraLocal - 20, 2, 30002); // 2700 
	endif

	GFTextLine(gump,x + CentralizaEixoX(larguraCelula,encaixeComponente.encaixeDesc,3.7),y + 20,1153,encaixeComponente.encaixeDesc);
	if(componenteNecessario.isPrincipal)
		GFGumpPic( gump, x + cint(larguraCelula/ 6), y + 22, 1655);
		GFTooltipText(gump,"Componente principal.");
	else
		if(componenteNecessario.isOpcional)
			GFGumpPic( gump, x + cint(larguraCelula/ 6), y + 22, 1653);
			GFTooltipText(gump,"Componente opcional.");
		endif
	endif

	var xBotao := x + 5;
	var yBotao := y + 60;

	if(!sessao.modoGm)
		var gerenciador := sessao.gerenciadorDropDown;
		var dropDownAtual := gerenciador.BuscaDropDownPorPosicao(indice);
		
		dropDownAtual.AtribuiX(xBotao);
		dropDownAtual.AtribuiY(yBotao + 20);
		
		GFAddButton(gump,xBotao, yBotao ,252,253,GF_CLOSE_BTN,dropDownAtual.PegaIdBottao());
	else
		GFAddButton(gump,xBotao, yBotao ,252,253,GF_CLOSE_BTN,COMPLEXOS_ESQUEMAS_ESCOLHA_GM_CPNT + indice);
	endif
	GFTooltipText(gump,"Escolher Componente.");

	var textoCaracteristica;

	if(Find(UPPER(encaixeComponente.caracteristica),"VAZIO",1) > 0)
		textoCaracteristica := "Vazio";
	else
		textoCaracteristica := UCFirst(encaixeComponente.caracteristica);;
	endif

	if(componenteEscolhido)
		GFTextLine(gump,x + 22 ,y + 60,1153,"{}".format(
		componenteEscolhido.item.nome,
		componenteEscolhido.material.nome,
		componenteEscolhido.tier.desc));

		var materialQualidadeFormatado := "[{}] [{}]".format( componenteEscolhido.material.nome, componenteEscolhido.tier.desc);
		GFTextLine(gump,x + CentralizaEixoX(larguraCelula,materialQualidadeFormatado,3.7) ,y + 85,89,materialQualidadeFormatado);
	else
		var caracteristica := "*" + textoCaracteristica + "*";
		GFTextLine(gump,x + CentralizaEixoX(larguraCelula,caracteristica,3.7) ,y + 103,915,caracteristica);
		GFTextLine(gump,x + 22 ,y + 60,1153,"Escolha um componente.");
	endif

	var posYcaracteristicas := y + (componenteEscolhido ? 120 : 100);
	var posYSetasDeslocamento := 2;
	
	if(TypeOfInt(componenteEscolhido.propriedades.valorPrincipal) != OT_UNINIT
	   && TypeOfInt(componenteEscolhido.propriedades.valorPrincipal) != OT_ERROR)
		GFTextLine(gump,x + 5,posYcaracteristicas,1153,textoCaracteristica); // caracteristica;
		GeraSetas(gump, x + 205,posYcaracteristicas + posYSetasDeslocamento,componenteEscolhido.propriedades.valorPrincipal);
	endif
	
	if(quantidadeEfeitosAdicionais > 0)
		var efeitosAdicionais := componenteEscolhido.propriedades.efeitosAdicionais; 
		
		foreach efeitoAdicional in (efeitosAdicionais.keys())
			GFTextLine(gump,x + 5,cint(posYcaracteristicas +  alturaLinha * _efeitoAdicional_iter),1153," + {}".format(efeitoAdicional)); // caracteristica;
			GeraSetas(gump, x + 205,cint(posYcaracteristicas + posYSetasDeslocamento + alturaLinha * _efeitoAdicional_iter),efeitosAdicionais[efeitoAdicional]);
			Sleepms(2);
		endforeach
	endif

	posYcaracteristicas += alturaLinha * (componenteEscolhido.propriedades.efeitosAdicionais.size() + 1);
	
	if(isHpModificado)
		var info := CriaCorSinal(componenteEscolhido.propriedades.modificadorHp);
		GFTextLine(gump,x + 5,posYcaracteristicas,1153,"Hp");
		GFTextLine(gump,x + 195,posYcaracteristicas,info.cor,"{}{}".format(info.sinal,componenteEscolhido.propriedades.modificadorHp));
		posYcaracteristicas += alturaLinha;
	endif

	if(isPesoModificado)
		var info := CriaCorSinal(componenteEscolhido.propriedades.modificadorPeso, true);
		GFTextLine(gump,x + 5,posYcaracteristicas,1153,"Peso");
		GFTextLine(gump,x + 195,posYcaracteristicas,info.cor,"{}{}%".format(info.sinal,componenteEscolhido.propriedades.modificadorPeso));
	endif

endfunction

function CriaCorSinal(valor, isInvertido := false)
	var retorno := struct;
	if(valor > 0)
		retorno.+cor := isInvertido ? 38 : 62;
		retorno.+sinal := "+";
	else
		retorno.+cor := isInvertido ? 62 : 38;
		retorno.+sinal := "";
	endif
	return retorno;
endfunction

function AumentaPainelSe(isTrue, valor, acrescimo)
	if(isTrue)
		valor += acrescimo;
	endif 
	return valor;
endfunction

function GeraSetas(byref gump,x,y, valor)
	var deslocamentoX := 16;
	var isPositivo := true;

	if(valor < 0)
		isPositivo := false;
		valor := valor * -1;
	else
		if(valor == 0)
			GFGumpPic( gump, x + 5, y +  5 , 1672);
		endif
	endif

	for i:=1 to valor
		if(isPositivo)
			GFGumpPic( gump, x - deslocamentoX * (i - 1), y , 5600,0x1d2);
		else
			GFGumpPic( gump, x - deslocamentoX * (i - 1), y , 5602,0x1af);
		endif
		Sleepms(2);
	endfor
endfunction

function PreencheReceitas(byref gump,x,deslocamentoX,y,deslocamentoY,largura,altura,receitas,sessao, receitasPorPagina)
	//TODO Implementar uma forma de lembrar onde estava da ultima vez que abriu o gump. Talvez usando a CProp legado "Craft".
	var page := 1;
	var colunasLinha := 4;

	//GFPage(gump,page);
	if(receitas != error && receitas.size() > 0)

		var paginador := MontaPaginador( sessao.paginador.complexos.pagina,receitasPorPagina);
		var linha := 0;

		for indice := paginador.indicePaginaAtual to (paginador.indicePaginaSeguinte - 1)
			if(indice > receitas.size())
				break;
			endif
			
			var receita :=  receitas[indice];
			
			MontaJanelaReceita(gump , x + deslocamentoX , y + deslocamentoY,linha, (indice - 1) % colunasLinha,receita,indice,sessao);

			if(receitas.size() >= paginador.indicePaginaSeguinte)
				GFAddButton(gump,x + largura - 45, y + altura / 2,4005,4006,GF_CLOSE_BTN,COMPLEXOS_BTN_RCT_PROX_PAGINA);
			endif
			if(1 >= paginador.indicePaginaAnterior)
				GFAddButton(gump,x + 13, y + altura / 2 ,4014,4015,GF_CLOSE_BTN,COMPLEXOS_BTN_RCT_ANT_PAGINA);
			endif

			if(indice % colunasLinha == 0)
					linha += 1;
			endif
			Sleepms(2);
		endfor
	else
		GFGumpPic(gump, x - 75 + largura / 4 ,y - 24 + altura / 4, 1596);
		GFGumpPic(gump, x - 75 + largura / 4 ,y + 118 + altura / 4, 1599);
		GFGumpPic(gump, x + largura / 2 - 43,y + altura / 2 -51, 337);
		//GFHTMLArea(gump,x + largura / 2 - 102,y + altura / 2,205,20,"<basefont size=55 color=#ffffff><center>Vazio</center>"); //TODO alterar para algo mais bonitinho.
	endif
endfunction

function PreencheCategorias(byref gump,larguraPainelCategoria, parametros, sessao)
	var margemEsquerdaLocal := G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10;
	var margemSuperiorLocal := G_MARGEM_SUPERIOR + G_ESPACAMENTO + 25;
	var alturaIniciocategorias := margemSuperiorLocal + 93;
	var gumpIdProfissaoPressed := parametros.gumpIdProfissaoPressed ? parametros.gumpIdProfissaoPressed : GUMPIC_ID_PROFISSOES_DEFAULT;
	var gumpIdProfissaoUnpressed := parametros.gumpIdProfissaoUnpressed ? parametros.gumpIdProfissaoUnpressed : GUMPIC_ID_PROFISSOES_DEFAULT;

	GFResizePic(gump, margemEsquerdaLocal, margemSuperiorLocal -14, 9250, larguraPainelCategoria - 18, 130);
	GFGumpPic(gump,margemEsquerdaLocal + larguraPainelCategoria / 2 - 50,margemSuperiorLocal + 5,1417); // Fundo Imagem Profissão
	GFAddButton(
			gump,
			margemEsquerdaLocal + larguraPainelCategoria / 2 - 50 + 8,
			margemSuperiorLocal + 15,gumpIdProfissaoPressed,
			gumpIdProfissaoUnpressed,GF_CLOSE_BTN,
			COMPLEXOS_BTN_AJUDA); // botao ajuda profissão

	GFTooltipText(gump,"Clique aqui para mais informações sobre como o sistema funciona.");
	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + larguraPainelCategoria / 2 - 35,margemSuperiorLocal + 85,70,20,"<basefont size=55 color=#ffffff><center>Categorias</center>");
	
	foreach categoria in (parametros.visaoReceitas.keys())
		var posX := G_MARGEM_ESQUERDA + G_ESPACAMENTO + 50;
		VAR alturaBaseLocal := 35;
		GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10,alturaIniciocategorias + (alturaBaseLocal * _categoria_iter) - 5, 3500, larguraPainelCategoria - 19, 30);
		GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 15,  alturaIniciocategorias - 2 + (alturaBaseLocal * _categoria_iter) ,4005,4006,GF_CLOSE_BTN, COMPLEXOS_CATEGORIAS + _categoria_iter);
		if(!(categoria in (sessao.visualizadas.categorias) || parametros.visaoReceitas[categoria].size() <= 0))
			GFGumpPic(gump, posX, alturaIniciocategorias + (alturaBaseLocal * _categoria_iter) + 3, 1655);
			posX += 20; 
		endif
		
		var cortexto := sessao.categoriaSelecionada == categoria ? 149 : 87;

		GFTextLine(gump,posX,alturaIniciocategorias + (alturaBaseLocal * _categoria_iter),cortexto,categoria);
		Sleepms(2);
	endforeach
endfunction

function DefineLarguraPainel(quantidadeCelulas)
	return larguraCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function DefineAlturaPainel(quantidadeCelulas)
	return alturaCelula * quantidadeCelulas - espacamentoAplicado;
endfunction

function MontaJanelaReceita(byref gump , x, y,linha, coluna, receita, indice, sessao)
	y := y + 150 * linha;
	x := x + 120 * coluna;

	var numeroBotoes := 3;
	var baseIdBotao :=  (indice - 1) * numeroBotoes + COMPLEXOS_RECEITAS;
	var deslocamentoGraphicY;

	if(receita.profissao == "Carpentry")
		deslocamentoGraphicY := 15; //Fix - alguns graphics estão saindo da janela.
	else
		deslocamentoGraphicY := 0;
	endif

	GFResizePic(gump, x, y, 1579, 120, 150);//5170, 1755,
	GFTooltipText(gump,receita.textoDescItem);

	if(receita.receitaClassificada.classe != CLASSE_PADRAO)
		GFGumpPic(gump,x + 30 ,y + 10,1592,coresClassesReceitas[cint(receita.receitaClassificada.classe)]); //
	endif
	var corPino := 0;
	if(receita.dificuldade.chanceBase > 45)
		corPino := 78; // verde
	elseif(receita.dificuldade.chanceBase <= 45 && receita.dificuldade.chanceBase > 30)
		corPino := 2212; //amarelo
	elseif(receita.dificuldade.chanceBase <= 30 && receita.dificuldade.chanceBase > 10)
		corPino:= 43;//Laranja
	else
		corPino := 0; // vermelho
	endif
	GFGumpPic(gump,x + 60 ,y + 10,9009, corPino); // Pino
	if(receita.dificuldade.chanceBase)
		GFTooltipText(gump,"Chance de obter sucessos: {}%".format(receita.dificuldade.chanceBase));
	endif

	if(sessao.receitaSelecionada.receita.receitaClassificada.nome == receita.receitaClassificada.nome &&
		sessao.receitaSelecionada.receita.receitaClassificada.classe == receita.receitaClassificada.classe)
		GFGumpPic(gump,x + 17 ,y + 95,30076,0x57); // seletor
	endif

	case (receita.uso)
		"One_Handed":
			GFGumpPic(gump,x + 10 ,y + 10,28319,46);
			break;
		"Two_Handed":
		"Dual_Wield":
			GFGumpPic(gump,x + 10 ,y + 10,28319,46);
			GFGumpPic(gump,x + 33 ,y + 10,28318,46);
			break;
		"Ranged":
			GFGumpPic(gump,x + 10 ,y + 10,28320,46);
			break;
		default:
	endcase
	
	if (!isReceitaVisualizada(receita))//icone de visualizacao
		GFGumpPic(gump, x + 75, y + 15, 1655);
	endif

	GFTilePic(gump, x + 30,y + 50 - deslocamentoGraphicY, CInt(receita.graphic), receita.corObrigatoria ? receita.corObrigatoria : 0);//Graphic

	GFAddButton(gump,x + 15, y + 120,4011,4012,GF_CLOSE_BTN,baseIdBotao + 1);
	GFTooltipText(gump,"Selecionar Receita.");
	GFAddButton(gump,x + 45,y + 120,4029,4030,GF_CLOSE_BTN,baseIdBotao + 2);
	GFTooltipText(gump,"Criar novo esquema.");
	if(!sessao.modoGm)
		GFAddButton(gump,x + 75,y + 120,4023,4024,GF_CLOSE_BTN,baseIdBotao + 3);
		GFTooltipText(gump,"Montar usando melhores opções de componentes.");
	endif
endfunction

function isReceitaVisualizada( receita)
	return receita.isVisualizada;
endfunction

function AvaliaEsforço(sessao)
	var diferenca := sessao.picoMaximoPericia - sessao.picoPericia;
	case (cint(diferenca))

		0: return "Máximo.";
		1: return "Muito Grande.";
		2: return "Grande.";
		3: return "Médio.";
		4: return "Minimo.";
		5: return "Nenhum.";
		default:
			return "Erro!";
	endcase
endfunction

function MontaFabricacaoAjudaGump(byref gump, parametros)
	gump := GFECreateGump("Manual de fabricação de {}".format(parametros.profissao.label),G_LARGURA,G_ALTURA,array{BORDER_BLUE});

	var margemEsquerda := espacamentoAplicado;
	var alturaBarraMenu := 80;

	GFAddButton(gump, larguraCelula * 1, alturaBarraMenu,2117,2118,GF_PAGE_BTN,FABRICACAO_AJUDA_ABA_GERAL);
	GFTextLine(gump, larguraCelula * 1 + 20, alturaBarraMenu,52,"Geral");
	GFAddButton(gump, larguraCelula * 4, alturaBarraMenu,2117,2118,GF_PAGE_BTN,FABRICACAO_AJUDA_ABA_COMPONENTES);
	GFTextLine(gump, larguraCelula * 4 + 20, alturaBarraMenu,52,"Componentes");
	GFAddButton(gump, larguraCelula * 7, alturaBarraMenu,2117,2118,GF_PAGE_BTN,FABRICACAO_AJUDA_ABA_MONTAGEM);
	GFTextLine(gump, larguraCelula * 7 + 20, alturaBarraMenu,52,"Montagem");
	GFAddButton(gump, larguraCelula * 10, alturaBarraMenu,2117,2118,GF_PAGE_BTN,FABRICACAO_AJUDA_ABA_TRATAMENTO);
	GFTextLine(gump, larguraCelula * 10 + 20, alturaBarraMenu,52,"Tratamento.");

	GFPage(gump, FABRICACAO_AJUDA_ABA_GERAL);
	GFHTMLArea(gump,larguraCelula * 1,alturaBarraMenu + 40,larguraCelula * 10, alturaCelula * 8,parametros.geral.textoPrincipal);

	GFPage(gump, FABRICACAO_AJUDA_ABA_COMPONENTES);
	GFHTMLArea(gump,larguraCelula * 1,alturaBarraMenu + 40,larguraCelula * 10, alturaCelula * 8,parametros.profissao.textoComponentes);
	GFPage(gump, FABRICACAO_AJUDA_ABA_MONTAGEM);
	GFHTMLArea(gump,larguraCelula * 1,alturaBarraMenu + 40,larguraCelula * 10, alturaCelula * 8,parametros.profissao.textoMontagem);
	GFPage(gump, FABRICACAO_AJUDA_ABA_TRATAMENTO);
	GFHTMLArea(gump,larguraCelula * 1,alturaBarraMenu + 40,larguraCelula * 10, alturaCelula * 8,parametros.profissao.textoTratamento);

	gump.cur_page := cint(parametros.pagina);
	return gump;
endfunction

function MontaEscolhaComponenteModoGmGump(byref gump, sessao)
	gump := GFECreateGump("Escolha de componente no modo GM",300,300,array{BORDER_GOLDEN});

	var margemEsquerda := espacamentoAplicado;
	var alturaBarraMenu := 80;

	var gerenciador := sessao.gerenciadorDropDown;
	var dropDownComponetes := gerenciador.BuscaDropDownPorPosicao(1);
	var dropDownQualidade := gerenciador.BuscaDropDownPorPosicao(2);
	var dropDownMateriais := gerenciador.BuscaDropDownPorPosicao(3);

	var componenteNome := sessao.modoGm.componenteEscolhido.name ? sessao.modoGm.componenteEscolhido.name : "Escolha o componente";
	var qualidadeDesc := sessao.modoGm.qualidadeEscolhida.desc ? sessao.modoGm.qualidadeEscolhida.desc : "Escolha a qualidade";
	var materialNome := sessao.modoGm.materialEscolhido.name ? sessao.modoGm.materialEscolhido.name : "Escolha o material";

	dropDownComponetes.AtribuiX(larguraCelula * 1);
	dropDownComponetes.AtribuiY(alturaBarraMenu + 20);
	dropDownQualidade.AtribuiX(larguraCelula * 1);
	dropDownQualidade.AtribuiY(alturaBarraMenu + 60);
	dropDownMateriais.AtribuiX(larguraCelula * 1);
	dropDownMateriais.AtribuiY(alturaBarraMenu + 100);
	
	GFAddButton(gump, larguraCelula * 1, alturaBarraMenu,252,253,GF_CLOSE_BTN,dropDownComponetes.PegaIdBottao());
	GFTextLine(gump, Cint(larguraCelula * 1.5) , alturaBarraMenu,369,componenteNome);
	GFAddButton(gump, larguraCelula * 1, alturaBarraMenu + 40,252,253,GF_CLOSE_BTN,dropDownQualidade.PegaIdBottao());
	GFTextLine(gump,  Cint(larguraCelula * 1.5) , alturaBarraMenu + 40,369,qualidadeDesc);
	GFAddButton(gump, larguraCelula * 1, alturaBarraMenu + 80,252,253,GF_CLOSE_BTN,dropDownMateriais.PegaIdBottao());
	GFTextLine(gump,  Cint(larguraCelula * 1.5) , alturaBarraMenu + 80,369,materialNome);

	GFAddButton(gump, larguraCelula * 2, alturaBarraMenu + 170,2311,2312,GF_CLOSE_BTN, COMPLEXOS_BTN_OKAY_GM_CMPNT);
	return gump;
endfunction
