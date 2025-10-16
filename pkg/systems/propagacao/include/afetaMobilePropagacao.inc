use util;

include ":combat:damage";
include ":medicine:disease";

function AfetaMobileIncendiado(quem,estado,propPropagacao,rolagemDano := 0)
	var danoDado;
	if(!rolagemDano)
		danoDado := propPropagacao[cstr(estado)]; 
		rolagemDano := RandomDiceRoll(danoDado);
	endif

	DamageFLS(quem, cint(rolagemDano), DMG_FIRE, 0, "Incêndio");
endfunction

function AfetaMobileInfectado(quem,propPropagacao)

	var duracaoPatogenico := propPropagacao.marcadorProps.duracaoPatogenico;
	var nivelInfeccao := propPropagacao.marcadorProps.nivelInfeccao;
	var gravidadeInfeccao := propPropagacao.marcadorProps.gravidadeInfeccao;
	var arraySintomas := propPropagacao.marcadorProps.arraySintomas;

	PrintTextAbove(quem,"Infectado com " + arraySintomas.size() + " sintomas ");
	InfectMobile(
		quem,
		duracaoPatogenico,
		0,
		nivelInfeccao,
		arraySintomas,
		gravidadeInfeccao
	);
endfunction