include ":gumps:/include/gumps";
include ":gumps:/include/epicGumps";
include ":gumps:/include/gumps_ex";

const ALTURA_LINHA := 20;

function CriaTabelaGump(x,y,largura,dados,byref gump)

	if(dados.size() > 0 )
		var exemplo := dados[1];
		var cabecalho := dictionary;
		var nomesColunas := array;
		
		if(TypeOf(exemplo) == "Struct")
			nomesColunas := exemplo.keys();
		endif

		var quantidadeColunas := nomesColunas.size();
		var larguraColuna := largura / quantidadeColunas;

		foreach nome in nomesColunas
			GFTextLine(gump, x + larguraColuna * (_nome_iter - 1), y, 0, nome);
		endforeach

		foreach dado in dados
			foreach nome in nomesColunas
				GFTextLine(gump, x + larguraColuna * (_nome_iter - 1), y + ALTURA_LINHA * _dado_iter, 0, dado[nome]);
			endforeach
		endforeach
	endif

	return gump;
endfunction;