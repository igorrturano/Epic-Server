include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";
include ":destroyable:include/destroyItemPersist";
include ":destroyable:include/destroyItem";

const ALTURA_LINHA := 20;
const QUALTIDADE_COLUNAS := 12;

const GUMP_LARGURA := 800;
const GUMP_ALTURA := 600;
const TEMPO_DIA := 86400;

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

enum IMPORTANCIA_ITEM
	DISPENSAVEL := "dispensavel",
	EMPERRADA := "emperrada",
	INDESTRUTIVEL := "Indestrutível"
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


function ConfiguraAlvo(quem,alvo,cfgselecao, mostrarMensagem := true)
	if(alvo.ISA(POLCLASS_ITEM) && !alvo.ISA(POLCLASS_EQUIPMENT))
		var material := configRecursos[cfgselecao.material];
		if(cfgselecao.durabTotal)
			SetObjProperty(alvo,PROPNAME_DURABILIDADE_MAX,CInt(cfgselecao.durabTotal));
			var atributos := MontaAtributosItem(alvo);
			SetObjProperty(alvo,PROPNAME_DURABILIDADE,CInt(atributos.durabTotal));
		endif
		if(cfgselecao.mltRes)
			SetObjProperty(alvo,PROPNAME_RESITEM_MULT,CDbl(cfgselecao.mltRes));
		endif
		if(cfgselecao.material)
			SetObjProperty(alvo,PROPNAME_MATERIAL, Cint(cfgselecao.material));
		endif
		if(cfgselecao.isEmperrada)
			if(alvo.ISA(POLCLASS_LOCKABLE))
				var durabTotal := GetObjProperty(alvo,PROPNAME_DURABILIDADE_MAX);
				SetObjProperty(alvo,PROPNAME_DURABILIDADE,CInt(durabTotal / 3));
			else
				SendSysMessageEx(quem," Configuração [Emperrada] pulada. O item {} não é trancável.".format(alvo.desc), SSM_INFO_IC);
			endif
		endif
		SetObjProperty(alvo,PROPNAME_DISPENSAVEL_FLAG,Cint(cfgselecao.isDispensavel));
		SetObjProperty(alvo,PROPNAME_INDESTRUTIVEL,Cint(cfgselecao.isIndestrutivel));

		alvo.color := material.Color;
		if(mostrarMensagem)
			SendSysMessageEx(quem,"Configurações aplicadas.", SSM_INFO);
		endif
	endif
endfunction

function EnviaConfigDesGump(quem)
	var chaves := GetConfigStringKeys(configRecursos);
	var visaoMateriais := array;
	
	var opcoesTabela := dictionary{ 
							QTD_LINHA_LIMITE -> 15,
							TITULO_TABELA -> "Escolha um material:",
							BOTAO_SOLTO -> 2510,
							BOTAO_APERTADO -> 2511
							};
	var valor := 1; 

	foreach chave in chaves
		var material := struct;
		material.+nome := configRecursos[chave].Name;
		material.+tipo := configRecursos[chave].Type;
		material.+marque :=  struct{isColunaBotao := true, valor := chave}; 
		visaoMateriais.append(material);
	endforeach

	var colCfg := dictionary;
	colCfg["nome"] := struct{titulo := "Material",ordem := 1,largura := 5};
	colCfg["tipo"] := struct{titulo := "Tipo",ordem := 2,largura := 5};
	colCfg["marque"] := struct{titulo := "Selecione",ordem := 3,largura := 2};

	var dispensavel := false;
	var radioImportancia;
	var configSelecionada := struct;
	var materialElem;

	while(valor[0])
		var margenEsquerdaLabels := 25;
		var margenEsquerdaValores := 140;
		var margenHorizontalCheckBox := 80;
		var margenHorizontalFinal := 560;
		var corLabels := 1891;
		var dictImportancia := dictionary{ EMPERRADA -> 1,DISPENSAVEL -> 2,INDESTRUTIVEL -> 3};
		var temChaves := valor.keys != error;

		var gump := GFECreateGump("Configura Destruíveis",400,600,array{BORDER_BLUE});
		GFPage(gump,0);
		GFTextLine( gump, margenEsquerdaLabels + 130, 118, 1891, "Durabilidade Total [Vazio = Padrão]");
		GFGumpPic( gump, margenEsquerdaLabels, 115, 1588);
		GFTextEntry( gump, margenEsquerdaLabels + 15, 118, 89, 20, 1153,temChaves && GFExtractData( valor, 4 ) != error? GFExtractData( valor, 4 ) : "", 4);
		GFTooltipText( gump,"Define um limite de durabilidade para o item (Pode ser ultrapassado pelos mod de material) [Vazio = Padrão] ");

		GFTextLine( gump, margenEsquerdaLabels + 130, 148, 1891, "Multiplicador de Resitência [V = P]");
		GFGumpPic( gump, margenEsquerdaLabels, 145, 1588);
		GFTextEntry( gump, margenEsquerdaLabels + 15, 148, 89, 20, 1153, temChaves && GFExtractData( valor, 5 ) != error? GFExtractData( valor, 5 ) : "", 5);
		GFTooltipText( gump, "Define um numero real como multiplicador de durabilidade base de item [base={}] (ex: 1.2, 2, 3) [Vazio = Padrão] ".format(configDestruiveis[ELEM_CFG_DEST].resistenciaBase));

		GFTextLine( gump, margenEsquerdaLabels + 20, margenHorizontalCheckBox, corLabels, EMPERRADA);
		GFTextLine( gump, margenEsquerdaLabels + 150, margenHorizontalCheckBox, corLabels, DISPENSAVEL);
		GFTextLine( gump, margenEsquerdaLabels + 270, margenHorizontalCheckBox, corLabels, INDESTRUTIVEL);

		GFRadioButton(gump, margenEsquerdaLabels, margenHorizontalCheckBox, 6439, 6440, temChaves && valor[dictImportancia[EMPERRADA]], dictImportancia[EMPERRADA]);
		GFTooltipText( gump, "Define a durabilidade do item para 1 terço do total, ficando no status emperrada.");
		GFRadioButton(gump, margenEsquerdaLabels + 130, margenHorizontalCheckBox, 6439, 6440, temChaves && valor[dictImportancia[DISPENSAVEL]], dictImportancia[DISPENSAVEL]);
		GFTooltipText( gump, "Define o item como dispensavel, dessa forma será ignorado pelo vigilante de recriação.");
		GFRadioButton(gump, margenEsquerdaLabels + 250, margenHorizontalCheckBox, 6439, 6440, temChaves && valor[dictImportancia[INDESTRUTIVEL]], dictImportancia[INDESTRUTIVEL]);
		GFTooltipText( gump, "Bloqueia a possibilidade de destruir o item pelo novo sistema.");

		GFTextLine( gump, margenEsquerdaLabels, 190, 768, "Material Escolhido:");
		var textoMaterial := configRecursos[valor[0]].Name? configRecursos[valor[0]].Name : "";
		GFTextLine( gump, margenEsquerdaValores, 190, 1258, UPPER(textoMaterial));

		gump := CriaTabelaGumpPaginada(25,210,360,visaoMateriais,gump,opcoesTabela,colCfg);

		GFPage(gump,0);
		GFAddButton(gump,margenEsquerdaLabels,margenHorizontalFinal,2453,2454,GF_CLOSE_BTN,6);
		GFAddButton(gump,margenEsquerdaLabels + 250,margenHorizontalFinal ,22300,22302,GF_CLOSE_BTN,7);
		GFTooltipText( gump, "Alvo por cadeia de objetos unicos.");
		GFAddButton(gump,margenEsquerdaLabels + 300,margenHorizontalFinal ,22306,22308,GF_CLOSE_BTN,8);
		GFTooltipText( gump, "Alvo por retangulo de seleção.");
		
		valor := GFSendGump(quem,gump);

		var tipoAlvo := 0; 
		if(valor[6])
			break;
		elseif(valor[7])
			tipoAlvo := "cadeiaUnica";
		elseif(valor[8])
			tipoAlvo := "retanguloSelecao";
		endif

		if(tipoAlvo)
			configSelecionada.+durabTotal := GFExtractData( valor, 4 );
			configSelecionada.+mltRes := GFExtractData( valor, 5 );
			configSelecionada.+isIndestrutivel := valor[dictImportancia[INDESTRUTIVEL]] == 1;
			configSelecionada.+isDispensavel := valor[dictImportancia[DISPENSAVEL]] == 1;
			configSelecionada.+isEmperrada := valor[dictImportancia[EMPERRADA]] == 1;
			configSelecionada.+tipoAlvo := tipoAlvo;
			if(materialElem)
				configSelecionada.+material := CInt(materialElem);
			endif
			break;
		endif

		materialElem := valor[0];
		Sleepms(2);
	endwhile
	return configSelecionada;
endfunction

function EnviaLogGump (quem)
	var colCfg := dictionary;
	colCfg["quem"] := struct{titulo := "Quem?",ordem := 1,largura := 2};
	colCfg["quando"] := struct{titulo := "Quando?",ordem := 2,largura := 3};
	colCfg["como"] := struct{titulo := "Como?",ordem := 3,largura := 2};
	colCfg["oque"] := struct{titulo := "O que?",ordem := 4,largura := 2};
	colCfg["x"] := struct{titulo := "x",ordem := 5,largura := 0.8};
	colCfg["y"] := struct{titulo := "y",ordem := 6,largura := 0.8};
	colCfg["z"] := struct{titulo := "z",ordem := 7,largura := 0.8};
	colCfg["realm"] := struct{titulo := "Reino",ordem := 8,largura := 1};
	colCfg["isReconstruivel"] := struct{titulo := "Volta?",largura := 0.8};

	Var pontoMedio := GUMP_LARGURA / 2 ;

	var valor := 1;
	while(valor[0])
		var filtroRegiao := 0;
		var filtroTempo := 0;

		if(valor[0] == 1)
			filtroRegiao := 20;
		elseif(valor[0] == 2)
		elseif(valor[0] == 3)
			filtroTempo := TEMPO_DIA * 7;
		elseif(valor[0] == 4)
			LimpaLogItemDestruido();
		else
			break;
		endif
		
		var arrayLog := ListaLogItemDestruido(quem,filtroRegiao,filtroTempo);
		var gump := GFECreateGump("Destruiveis Log",800,600,array{BORDER_BLUE,HIDE_TITLE});

		GFPage( gump, 0);

		GFAddButton(gump,50, 40,9720,9724,GF_CLOSE_BTN, 1);
		GFTextLine( gump, 50 + 35, 40 + 5 , 1051, "Em um raio de 20 tiles.");

		GFAddButton(gump,250, 40,9720,9724,GF_CLOSE_BTN, 2);
		GFTextLine( gump, 250 + 35, 40 + 5 , 1051,"Todos registros.[5 pg max]");

		GFAddButton(gump,450, 40,9720,9724,GF_CLOSE_BTN, 3);
		GFTextLine( gump, 450 + 35, 40 + 5 , 1051, "Ultimos 7 dias.");

		GFAddButton(gump,650, 40,9720,9724,GF_CLOSE_BTN, 4);
		GFTextLine( gump, 650 + 35, 40 + 5 , 1051, "Limpar Log.");

		var pagina;
		var qtdeMaxLinhas := 22;
		var algo := CDbl(arrayLog.size()) / qtdeMaxLinhas;
		for( pagina := 0; (pagina + 1) - algo < 1; pagina := pagina + 1)
			if(pagina + 1 > 5)
				break;
			endif
			GFPage( gump, pagina + 1);
			gump := CriaTabelaGump(25,80,705,arrayLog,gump,pagina,dictionary{QTD_LINHA_LIMITE -> qtdeMaxLinhas},colCfg);

			//Controle de Paginação.
			if(!pagina == 0)
				GFAddButton( gump,  pontoMedio - 40, 570, 2223, 2223, GF_PAGE_BTN, gump.cur_page - 1 );
			endif
			if((pagina + 2) - algo < 1)
				GFAddButton( gump, pontoMedio + 40, 570, 2224, 2224, GF_PAGE_BTN, gump.cur_page + 1 );
			endif
		endfor

		valor := GFSendGump(quem,gump);
	endwhile
endfunction