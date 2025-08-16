include ":propagacao:include/propagacao";

enum GRAPHICS_FIREWALL
	GRAFICO_FIREWALL_OESTE_LESTE := 0x398C,
	GRAFICO_FIREWALL_NORTE_SUL := 0x3996,
endenum

function CriaMuralhaFogo(x,y,z,realm,direcao, dadoDano, combustivel, conjurador)

	var catalogoDano := MontaCatalogoDano();
	var dict := dictionary;
	var fatorInflamavel := 5;

	catalogoDano.insert(cstr(ESTADO_PROPAGACAO_INTENSO),dadoDano);
	NovoEfeitoPersonalizado(dict,ESTADO_PROPAGACAO_INTENSO,direcao,0,0,0x1DE,0,0,-1);

	var propPropagacao := NovaPropPropagacao(
										0,
										conjurador.serial,
										-1,
										PROPAGACAO_TIPO_INCENDIO,
										ESTADO_PROPAGACAO_INTENSO,
										0,
										0,
										catalogoDano,
										dict,
										NovoStructCombustivelMagico(combustivel,fatorInflamavel));

	var muralha := CriarMarcadorPropagacao(x,y,z,realm,propPropagacao);
	muralha.Reacender(500);
endfunction