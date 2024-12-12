include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";
include ":destroyable:include/destroyItemPersist";
include ":destroyable:include/destroyItem";

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
	var filtroRegiao := 0;
	var filtroTempo := 0;
	var paginaAtual := 1;
	var qtdPaginas := 0;
	var qtdeMaxLinhas := 22;
	var deveRequisitarLog := true;
	var arrayLogPaginado := array;

	while(valor[0])
		if(deveRequisitarLog)
			arrayLogPaginado := array;
			Print( "filtro regiao " + filtroRegiao);
			var arrayLog := ListaLogItemDestruido(quem,filtroRegiao,filtroTempo);
			qtdPaginas := Cint(arraylog.size() / qtdeMaxLinhas);
			Print( "filtro regiao " + arraylog.size());
			var paginaLog := array;
			var indice;
			
			for (indice:= 1; indice <= arraylog.size(); indice:= indice + 1)

				paginaLog.append(arraylog[indice]);
				if(paginaLog.size() >= qtdeMaxLinhas || indice == arraylog.size())
					arrayLogPaginado.append(paginaLog);
					paginaLog := array;
				endif
				
				Sleepms(2);
			endfor
			paginaAtual := 1;
			deveRequisitarLog := false;
		endif
		var gump := GFECreateGump("Destruiveis Log",800,600,array{BORDER_BLUE,HIDE_TITLE});

		GFPage( gump, 0);

		GFAddButton(gump,50, 40,9720,9724,GF_CLOSE_BTN, 1);
		GFTextLine( gump, 50 + 35, 40 + 5 , 1051, "Em um raio de 20 tiles.");

		GFAddButton(gump,250, 40,9720,9724,GF_CLOSE_BTN, 2);
		GFTextLine( gump, 250 + 35, 40 + 5 , 1051,"Todos registros.");

		GFAddButton(gump,450, 40,9720,9724,GF_CLOSE_BTN, 3);
		GFTextLine( gump, 450 + 35, 40 + 5 , 1051, "Ultimos 7 dias.");

		GFAddButton(gump,650, 40,9720,9724,GF_CLOSE_BTN, 4);
		GFTextLine( gump, 650 + 35, 40 + 5 , 1051, "Limpar Log.");

		gump := CriaTabelaGump(25,80,705,arrayLogPaginado[paginaAtual],gump,0,dictionary{QTD_LINHA_LIMITE -> qtdeMaxLinhas},colCfg);

		//Controle de Paginação.
		if(paginaAtual != 1)
			GFAddButton( gump,  pontoMedio - 40, 570, 2223, 2223, GF_CLOSE_BTN, 5 ); //Volta pagina
		endif
		if(paginaAtual < arrayLogPaginado.size())
			GFAddButton( gump, pontoMedio + 40, 570, 2224, 2224, GF_CLOSE_BTN, 6); //Passa pagina
		endif

		valor := GFSendGump(quem,gump);

		if(valor[1])
			filtroRegiao := 20;
			deveRequisitarLog := true;
		elseif(valor[2] )
			deveRequisitarLog := true;
		elseif(valor[3])
			filtroTempo := TEMPO_DIA * 7;
			deveRequisitarLog := true;
		elseif(valor[4])
			LimpaLogItemDestruido();
			deveRequisitarLog := true;
		elseif(valor[5]) // volta pagina
			paginaAtual -= 1;
		elseif(valor[6]) // passa pagina
			paginaAtual += 1;
		else
			break;
		endif
	endwhile
endfunction