include ":propagacao:include/propagacao";
include ":crafting:include/fabricacaoUtil";

enum graphicsFirewall
	GRAFICO_FIREWALL_OESTE_LESTE := 0x398C,
	GRAFICO_FIREWALL_NORTE_SUL := 0x3996,
endenum

function CriaMuralhaFogo(x,y,z,realm,direcao, dadoDano, combustivel, conjurador, fatorInflamavel := 5)

	var catalogoDano := MontaCatalogoDano();
	var dict := dictionary;
	
	if(dadoDano)
		catalogoDano.insert(cstr(ESTADO_PROPAGACAO::INTENSO),dadoDano);
	endif
	var opcoesEfeitos := NovoOpcoesEfeitoPersonalizado(direcao);
	opcoesEfeitos.duracao := 0;
	opcoesEfeitos.isInvisibel := 0;
	opcoesEfeitos.chanceRandom := -1;
	
	NovoEfeitoPersonalizado(dict,ESTADO_PROPAGACAO::INTENSO,opcoesEfeitos);

	var muralha := CriarMarcadorPropagacao(
											x,
											y,
											z,
											realm, 
											struct {
												origem := conjurador.serial,
												tipoPropagacao := PROPAGACAO_TIPO::INCENDIO,
												estadoInicial := ESTADO_PROPAGACAO::INTENSO,
												marcadorProps := struct{ catalogoDano := catalogoDano},
												efeitosPersonalizados := dict,
												combustivelExterno := NovoStructCombustivelMagico(combustivel,fatorInflamavel)
											}
											);
	muralha.Abastecer(true);
endfunction

function CriaFagulhaIncendiaria(quem, alvo)
	var marcador := CriarMarcadorPropagacao(
										alvo.x,
										alvo.y,
										alvo.z,
										alvo.realm,
										struct {
												origem := quem.serial,
												tipoPropagacao := PROPAGACAO_TIPO::INCENDIO,
												estadoInicial := ESTADO_PROPAGACAO::PRINCIPIO,
												combustivelExterno := NovoStructCombustivelMagico(5,1)
										} 
										);
endfunction

function TentaCriarFagulhaIncendiaria(quem, alvo)
	if(!CheckLineOfSight(quem, alvo) || !alvo.isA(POLCLASS_ITEM))
		return 0;
	endif

	var materialConfig := PegaMaterialConfig(GetObjProperty(alvo,PROPNAME_MATERIAL));
	if(materialConfig.inflamavel >= 3) 
		CriaFagulhaIncendiaria(quem, alvo);
		return 1;
	endif
endfunction