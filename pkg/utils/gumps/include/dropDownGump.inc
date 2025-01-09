include ":gumps:/include/epicGumps";

function DropDownGump(x,y,valores)
	var gump := GFCreateGump(x, y, 200, 235);
	GFMovable(gump, 0);
	GFResizePic( gump, x, y, 2620, 200, 235 );
	GFAddAlphaRegion( gump, x + 5, y + 5, 190, 225 );

	var opcoesTabela := dictionary{ 
							MAX_PAGINAS -> 30,
							QTD_LINHA_LIMITE -> 8
							};
	
	var colCfg := dictionary;
	colCfg["opcao"] := struct{titulo := "Opção", largura := 9};
	colCfg["botao"] := struct{titulo := "X",ordem := 1,largura := 3};
	CriaTabelaGumpPaginada(x + 10,y + 10,190,valores,gump,opcoesTabela,colCfg);
	return gump;
endfunction 

function PreparaOpcaoDropDown(byref arrayExterno,opcao, valorExterno)
	if( TypeOfInt(arrayExterno) != OT_ARRAY)
		Print("Erro: parâmetro obrigatório não foi informado ao tentar incluir opção ao DropDownGump.");
		return 1;
	endif

	var objeto := struct;
	objeto.+opcao := opcao;
	objeto.+botao := struct{isColunaBotao := true, valor := valorExterno}; 
	arrayExterno.append(objeto);
endfunction

function SendDropDownGump( quem, pid, x, y, valores ,dropDownId := 0)
	return Start_Script( ":gumps:scripts/dropDown/dropDownGump", array{quem, pid, x, y, valores, dropDownId} );
endfunction
