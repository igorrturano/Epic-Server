use util;

include ":combat:Controller/DamageController_legacy";
include ":medicine:disease";
include ":tn:cooldown";
include "include/imersao";

function AfetaMobileIncendiado(quem,estado,propPropagacao,rolagemDano := 0)
	var danoDado;
	if(!rolagemDano)
		danoDado := propPropagacao[cstr(estado)]; 
		rolagemDano := RandomDiceRoll(danoDado);
	endif

	DamageFLS(quem, cint(rolagemDano), DMG_FIRE, 0, "Incêndio");
endfunction

function AfetaMobileInfectado(quem,propPropagacao, resfriamento)

	if(!(getCooldown(quem, PROPNAME_GAS_PATOGENICO_COOLDOWN) > 0))
		
		var duracaoPatogenico := propPropagacao.marcadorProps.duracaoPatogenico;
		var nivelInfeccao := propPropagacao.marcadorProps.nivelInfeccao;
		var gravidadeInfeccao := propPropagacao.marcadorProps.gravidadeInfeccao;
		var arraySintomas := propPropagacao.marcadorProps.arraySintomas;


		Imersao(quem,"tossir1");
		setCooldown(quem, PROPNAME_GAS_PATOGENICO_COOLDOWN,resfriamento);
		
		var diseases_system := GetObjProperty(quem, "Diseases_System");
		if (diseases_system)
		return;
		endif

		InfectMobile(
			quem,
			duracaoPatogenico,
			0,
			nivelInfeccao,
			arraySintomas,
			gravidadeInfeccao
		);


	endif

endfunction