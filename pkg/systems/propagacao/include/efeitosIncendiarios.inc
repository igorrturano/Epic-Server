include ":propagacao:include/propagacao";

enum graphicsFirewall
	GRAFICO_FIREWALL_OESTE_LESTE := 0x398C,
	GRAFICO_FIREWALL_NORTE_SUL := 0x3996,
endenum

function CriaMuralhaFogo(x,y,z,realm,direcao, dadoDano, combustivel, conjurador, fatorInflamavel := 5)

	var catalogoDano := MontaCatalogoDano();
	var dict := dictionary;
	
	if(dadoDano)
		catalogoDano.insert(cstr(ESTADO_PROPAGACAO_INTENSO),dadoDano);
	endif
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
	muralha.Abastecer(500);
endfunction

function CriaFagulhaIncendiaria(quem, alvo)
	var marcador := CriarMarcadorPropagacao(
										alvo.x,
										alvo.y,
										alvo.z,
										alvo.realm, 
										NovaPropPropagacao(0,quem.serial,-1,PROPAGACAO_TIPO_INCENDIO,ESTADO_PROPAGACAO_PRINCIPIO)
										);
endfunction