use uo;

include "/include/epicPropNames";
include ":gumps:/include/epicGumps";

const ATUALIZA_POS_PAI_ID := 9;
const ATUALIZA_POS_ID := 15;

enum GUMP_POSICAO_DIRECAO
	POSICAO_TOPO := 3,
	POSICAO_DIREITA := 4,
	POSICAO_BAIXO := 5,
	POSICAO_ESQUERDA := 6,
	POSICAO_RESETA := 10
endenum

enum GUMP_POSICAO_DISTANCIA
	DISTANCIA_AUMENTA := 7,
	DISTANCIA_DIMINUI := 8
endenum

function MontaControleGump(controleX,controleY,descIncremento := "Curta")
	var parLargura := 140;
	var altura := 70;
	var gump := GFCreateGump(controleX, controleY, parLargura, altura);
	GFResizePic( gump, controleX, controleY, 2620, parLargura + 10, altura + 10 );
	GFResizePic(gump, controleX + 5,controleY + 5, 5054, parLargura, altura);
	GFAddAlphaRegion( gump, controleX + 5, controleY + 5, parLargura, altura );

	var centroCruzX := controleX + 25;
	var centroCruzY := controleY + 30;
	GFAddButton(gump,centroCruzX,centroCruzY - 15,5600,5600,GF_CLOSE_BTN,POSICAO_TOPO);
	GFAddButton(gump,centroCruzX + 15,centroCruzY ,5601,5601,GF_CLOSE_BTN,POSICAO_DIREITA);
	GFAddButton(gump,centroCruzX,centroCruzY + 15,5602,5602,GF_CLOSE_BTN, POSICAO_BAIXO);
	GFAddButton(gump,centroCruzX - 15,centroCruzY ,5603,5603,GF_CLOSE_BTN, POSICAO_ESQUERDA);
	GFAddButton(gump,centroCruzX + 2,centroCruzY + 2 ,5032,5032,GF_CLOSE_BTN, POSICAO_RESETA);

	GFTextLine(gump,centroCruzX + 40,centroCruzY,1258,descIncremento);

	GFAddButton(gump,centroCruzX + 95,6 + centroCruzY - 15 ,2435,2436,GF_CLOSE_BTN, DISTANCIA_AUMENTA);
	GFAddButton(gump,centroCruzX + 95,6 + centroCruzY + 15,2437,2438,GF_CLOSE_BTN, DISTANCIA_DIMINUI);

	return gump;
	
endfunction

function PegaPosicaoProp(quem)
	var posicaoProp := GetObjProperty(quem, PROPNAME_POSICAO_GUMP);

	if(!posicaoProp)
		SetObjProperty(quem,PROPNAME_POSICAO_GUMP, dictionary);
		return dictionary;
	endif

	return posicaoProp;

endfunction

function PegaPosicaoGump(quem, apelidoGump)
	var posicaoProp := PegaPosicaoProp(quem);
	var posicao2D := struct { x := 0, y := 0};

	if(posicaoProp.exists(apelidoGump))
		return posicaoProp[apelidoGump];
	else
		posicaoProp.insert(apelidoGump,posicao2D);
		SetObjProperty(quem,PROPNAME_POSICAO_GUMP,posicaoProp);
		return posicao2D;
	endif
endfunction

function AtualizaPosicaoProp(quem, apelidoGump, posicao2D)
	var posicaoProp := PegaPosicaoProp(quem);
	if(posicaoProp.exists(apelidoGump))
		posicaoProp[apelidoGump] := posicao2D;
	else
		posicaoProp.insert(apelidoGump, posicao2D);
	endif
	SetObjProperty(quem,PROPNAME_POSICAO_GUMP,posicaoProp);

	return posicaoProp;
endfunction