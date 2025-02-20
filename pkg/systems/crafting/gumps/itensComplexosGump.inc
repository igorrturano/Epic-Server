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
	COMPLEXOS_ESQUEMAS := 300,
	COMPLEXOS_ESQUEMAS_CPNT := 330,
	COMPLEXOS_DROPDOWN_INICIAL := 2000,
	COMPLEXOS_DROPDOWN_OPCOES_INICIAL :=	3000,
	COMPLEXOS_DROPDOWN_MAX := 20,
	COMPONENTES_DROPDOWN_INICIAL := 100,
	COMPONENTES_DROPDOWN_OPCOES_INICIAL :=	200,
	COMPONENTES_DROPDOWN_MAX := 10,
	COMPONENTES_TABELA_INICIAL := 1000
endenum

enum BTN_IDS // ids, não variaveis, de botões e entradas de cada painel.
	COMPONENTES_INPT_QTD := 2,
	COMPONENTES_BTN_FABRICAR :=	3,
	COMPLEXOS_ESQUEMAS_INPT_NOME := 2,
	COMPLEXOS_ESQUEMAS_BTN_SALVAR:= 3
	
endenum

enum CLASSE_RETORNO_GUMP_COMPLX // (RT = RETORNO) Classificação do retorno do Gump de intes complexos.
	RT_RECEITAS,
	RT_CATEGORIAS,
	RT_ESQUEMAS_CPNT,
	RT_ESQUEMAS,
	RT_ESQUEMAS_SALVAR,
	RT_BOTAO_MENU,
	RT_CANCELADO_COMPLX,
	RT_ESCOLHA_DROPDOWN_CPX,
	RT_ESCOLHA_DD_OPCOES_CPX
endenum

enum CLASSE_RETORNO_GUMP_COMPNT //(RT = RETORNO) Classificação do retorno do Gump de Componentes.
	RT_ESCOLHA_DROPDOWN,
	RT_ESCOLHA_DD_OPCOES,
	RT_ESCOLHA_COMPONENTE,
	RT_BTN_FABRICAR,
	RT_CANCELADO
endenum

enum GUMP_ATUAL_COMPLEXOS //Definição do Gump atual da sessão no gump de itens complexos.
	COMPLEXOS,
	ESQUEMAS
endenum

enum ACAO_COMPLEXOS //Definição do Gump atual da sessão no gump de itens complexos.
	ADICIONAR,
	EDITAR
endenum

var alturaCelula :=  G_ALTURA / G_CELULA_DIVISOR;
var larguraCelula :=  G_LARGURA / G_CELULA_DIVISOR;

var espacamentoAplicado := G_ESPACAMENTO * 2;

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
	
	PreencheCategorias(gump,larguraPainelCategoria,parametros);

	var painelReceitasX := G_MARGEM_ESQUERDA + espacamentoAplicado + larguraPainelCategoria;
	var painelReceitasY := G_MARGEM_SUPERIOR + G_ESPACAMENTO ;
	GFResizePic(gump, painelReceitasX, painelReceitasY , 1460, larguraPainelReceita, alturaPainelReceita); //Painel Receitas.
	GFResizePic(gump, painelReceitasX,  painelReceitasY + alturaPainelReceita + espacamentoAplicado, 1755, larguraPainelEsquema, alturaPainelEsquema); //Painel Esquemas.
	
	GFHTMLArea(gump,painelReceitasX + larguraPainelEsquema / 2 - 35,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 15,70,20,"<basefont size=55 color=#ffffff><center>Esquemas</center>");
	
	PreencheReceitas(gump , painelReceitasX, 45, painelReceitasY, 55,larguraPainelReceita,alturaPainelReceita,parametros.visaoReceitas[sessao.categoriaSelecionada], sessao.receitaSelecionada);
	PreencheEsquemas (gump,painelReceitasX + 15,painelReceitasY + alturaPainelReceita + espacamentoAplicado + 40,sessao);
	
	GFGumpPic(gump,painelReceitasX + larguraPainelReceita / 2 - 102,20,1764); //Placa Titulo Receitas.
	GFHTMLArea(gump,painelReceitasX + larguraPainelReceita / 2 - 102,35,205,20,"<basefont size=55 color=#ffffff><center>Receitas</center>");

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,alturaCelula * 11 + 10, 5054, larguraPainelRodape, alturaPainelRodape);// Painel Rodapé

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
		GFAddButton(gump,x + deslocamentoX * coluna,y + deslocamentoY * linha,4011,4012, GF_CLOSE_BTN, baseIdBotao + 1);
		GFTooltipText(gump,"Editar.");
		GFAddButton(gump,x + 30 + deslocamentoX * coluna,y + deslocamentoY * linha,4020,4021, GF_CLOSE_BTN, baseIdBotao + 2);
		GFTooltipText(gump,"Excluir.");
		GFAddButton(gump,x + 60 + deslocamentoX * coluna,y + deslocamentoY * linha,4023,4024, GF_CLOSE_BTN, baseIdBotao + 3);
		GFTooltipText(gump,"Montar.");

		++indice;
	endforeach
endfunction

function MontaEsquemaGump(byref gump,byref sessao)
	gump := GFECreateGump("GumpEsquema",G_LARGURA,G_ALTURA,array{BORDER_BLUE,HIDE_TITLE,HIDE_PAGE});
	var margemEsquerdaLocal := G_MARGEM_ESQUERDA + G_ESPACAMENTO + 10;
	var margemSuperiorLocal := G_MARGEM_ESQUERDA + G_ESPACAMENTO;

	var larguraEncaixeHeader := DefineLarguraPainel(12);
	var alturaEncaixeHeader := DefineAlturaPainel(1);

	var larguraPainelInfo := DefineLarguraPainel(12);
	var alturaPainelInfo := DefineAlturaPainel(4.8);
	
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO, G_MARGEM_SUPERIOR + G_ESPACAMENTO, 83, larguraPainelInfo, alturaPainelInfo);

	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO,CInt(alturaCelula * 4.9), 5054, larguraEncaixeHeader, alturaEncaixeHeader);
	GFResizePic(gump, G_MARGEM_ESQUERDA + G_ESPACAMENTO + (larguraCelula * 4.4) - 5,CInt(alturaCelula * 4.9) + 4, 3000, larguraCelula * 3, alturaEncaixeHeader - 7);
	GFHTMLArea(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO,CInt(alturaCelula * 4.9) + 7,larguraEncaixeHeader,alturaEncaixeHeader,"<basefont size=55 color=#024d95><center>Encaixes de Componente</center>");
	GFTextLine(gump,margemEsquerdaLocal + 10, margemSuperiorLocal + Cint(alturaCelula * 0.5),1153,"Receita: {}".format("PlaceHolder"));
	
	GFTextLine(gump,margemEsquerdaLocal + 10, margemSuperiorLocal + Cint(alturaCelula),1153,"Nome do Esquema:");
	if(sessao.acao == ADICIONAR)
		GFTextEntry(gump,margemEsquerdaLocal  + (larguraCelula * 2), margemSuperiorLocal + Cint(alturaCelula), 120, 17, 1153, sessao.receitaSelecionada.esquemaSelecionado.nome, 2,40);
	elseif(sessao.acao == EDITAR)
		GFTextLine(gump,margemEsquerdaLocal  + (larguraCelula * 2), margemSuperiorLocal + Cint(alturaCelula),1153,sessao.receitaSelecionada.esquemaSelecionado.nome);
	endif
	
	GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + Cint(larguraCelula * 10.5),Cint(alturaCelula * 4),2460,2461,GF_CLOSE_BTN,COMPLEXOS_ESQUEMAS_BTN_SALVAR);
	GFTooltipText(gump,"Salvar Receita.");
	//Print("Componentes: " + sessao);
	var pagina :=  1;
	var posx := 1;
	GFPage(gump,pagina);
	foreach componente in (sessao.receitaSelecionada.receita.componentesNecessarios)
		PreencheEncaixes(gump, margemEsquerdaLocal + (larguraCelula * 4) * ((_componente_iter - 1) % 3), margemSuperiorLocal + Cint(alturaCelula * 5.3), _componente_iter,componente,sessao);
		
		//Gerencia Paginação.
		if(_componente_iter % 3 == 0)
			var calculoProxPagona := (pagina + 1) - 0.6;
			if(calculoProxPagona < 1)
				GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + larguraCelula * 11 + 10,Cint(alturaCelula * 4.9) + 5,4005,4006,GF_PAGE_BTN,gump.cur_page + 1);
			endif
			++pagina;
			GFPage(gump,pagina);
			if(pagina > 1)
				GFAddButton(gump,G_MARGEM_ESQUERDA + G_ESPACAMENTO + 5,CInt(alturaCelula * 4.9) + 5,4014,4015,GF_PAGE_BTN,gump.cur_page - 1);
			endif
		endif

		Sleepms(2);
	endforeach
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
	GFTextEntry( gump, G_MARGEM_ESQUERDA+ G_ESPACAMENTO * 2 + 70, G_MARGEM_SUPERIOR + G_ESPACAMENTO * 2 , 25, 17, 1153, sessao.quantidade ? sessao.quantidade : "1", COMPONENTES_INPT_QTD,3);
	
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

function PreencheEncaixes(byref gump, x, y, indice,encaixeComponente,sessao)
	var larguraLocal := larguraCelula * 3.7;
	var alturaPainel := alturaCelula * 3;
	var alturaLinha := 30;
	var componenteEscolhido := sessao.receitaSelecionada.esquemaSelecionado.componentesEscolhidos[indice];
	
	var quantidadeEfeitosAdicionais := componenteEscolhido.propriedades.efeitosAdicionais.size();
	if(quantidadeEfeitosAdicionais > 0)
		alturaPainel := alturaPainel + alturaLinha * quantidadeEfeitosAdicionais;
	endif

	GFResizePic(gump, x - 10,y, 3600, larguraLocal, alturaPainel - 10);
	GFPicTiled(gump, x,y + 10, larguraLocal - 20, alturaPainel - 30,40012);
	GFPicTiled(gump, x, y + 50, larguraLocal - 20, 2, 30002); // 2700 
	GFPicTiled(gump, x, y + 90, larguraLocal - 20, 2, 30002); // 2700 

	GFTextLine(gump,x + larguraCelula * 2 ,y + 20,1153,encaixeComponente.encaixeDesc);

	var xBotao := x + 5;
	var yBotao := y + 60;
	var gerenciador := sessao.gerenciadorDropDown;
	gerenciador.dropDowns[indice].x := xBotao;
	gerenciador.dropDowns[indice].y := yBotao + 20;
	
	GFAddButton(gump,xBotao, yBotao ,252,253,GF_CLOSE_BTN,gerenciador.dropDowns[indice].idBottao);
	GFTooltipText(gump,"Escolher Componente.");

	if(componenteEscolhido)
		GFTextLine(gump,x + 22 ,y + 60,1153,"{} [{}] [{}]".format(
		componenteEscolhido.item.nome,
		componenteEscolhido.material.nome,
		componenteEscolhido.tier.desc));
	else
		GFTextLine(gump,x + 22 ,y + 60,1153,"Escolha um componente.");
	endif

	var posYcaracteristicas := y + 100;
	var posYSetasDeslocamento := 2;
	GFTextLine(gump,x + 5,posYcaracteristicas,1153,encaixeComponente.caracteristica); // caracteristica;
	GeraSetas(gump, x + 205,posYcaracteristicas + posYSetasDeslocamento,componenteEscolhido.propriedades.valorPrincipal);
	
	if(quantidadeEfeitosAdicionais > 0)
		foreach efeitoAdicional in (componenteEscolhido.propriedades.efeitosAdicionais)
			GFTextLine(gump,x + 5,posYcaracteristicas +  alturaLinha * _efeitoAdicional_iter,1153," + {}".format(efeitoAdicional.efeito)); // caracteristica;
			GeraSetas(gump, x + 205,posYcaracteristicas + posYSetasDeslocamento + alturaLinha * _efeitoAdicional_iter,efeitoAdicional.valor);
			Sleepms(2);
		endforeach
	endif
endfunction


function GeraSetas(byref gump,x,y, valor)
	var deslocamentoX := 16;
	var isPositivo := true;

	if(valor < 0)
		isPositivo := false;
		valor := valor * -1;
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

function PreencheReceitas(byref gump,x,deslocamentoX,y,deslocamentoY,largura,altura,receitas, receitaSelecionada)
	//TODO Implementar uma forma de lembrar onde estava da ultima vez que abriu o gump. Talvez usando a CProp legado "Craft".
	var page := 1;
	var colunasLinha := 4;
	GFPage(gump,page);
	if(receitas != error && receitas.size() > 0)
		var linha := 0;
		foreach receita in receitas
			MontaJanelaReceita(gump , x + deslocamentoX , y + deslocamentoY,linha, (_receita_iter - 1) % colunasLinha,receita,_receita_iter,receitaSelecionada);

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
	//TODO fazer um highlight para a categoria selecionada.
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

function MontaJanelaReceita(byref gump , x, y,linha, coluna, receita, indice, receitaSelecionada)
	y := y + 150 * linha;
	x := x + 120 * coluna;

	var numeroBotoes := 3;
	var baseIdBotao :=  (indice - 1) * numeroBotoes + COMPLEXOS_RECEITAS;

	GFResizePic(gump, x, y, 1579, 120, 150);//5170, 1755,
	GFTooltipText(gump,receita.textoDescItem);
	GFAddButton(gump,x + 15, y + 120,4011,4012,GF_CLOSE_BTN,baseIdBotao + 1);
	GFTooltipText(gump,"Selecionar Receita.");
	GFAddButton(gump,x + 45,y + 120,4029,4030,GF_CLOSE_BTN,baseIdBotao + 2);
	GFTooltipText(gump,"Criar novo esquema.");
	GFAddButton(gump,x + 75,y + 120,4023,4024,GF_CLOSE_BTN,baseIdBotao + 3);
	GFTooltipText(gump,"Montar usando melhores opções de componentes.");
	var corPino := 0;
	if(receita.dificuldade.chanceBase > 45)
		corPino := 1367; // verde
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

	if(receitaSelecionada.receita.objtype == receita.objtype)
		GFGumpPic(gump,x + 17 ,y + 95,30076,0x57); // seletor
	endif

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
