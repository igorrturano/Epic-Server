use uo;
use cfgfile;

include ":gumps:gumps";
include "include/arrays";
include ":attributes:attributes";
include ":gumps:yesNo";

function ApplySkillCap(who)

	var classe := GetObjProperty(who, "classe");
	if (classe != error)

		var secundarias := getSecundarias(who);
		foreach secundaria in secundarias
			if ( AP_GetTrueSkill(who, secundaria) > 70 )
				AP_SetTrueSkill(who, secundaria, 70);
			endif
		endforeach

		var terciarias := getTerciarias(who);
		foreach terciaria in terciarias
			if ( AP_GetTrueSkill(who, terciaria) > 40 )
				AP_SetTrueSkill(who, terciaria, 40);
			endif
		endforeach


	endif

endfunction

function getTerciarias(who)

	var chardata := GetObjProperty(who, "chardata");
	if ( !chardata)
		return;
	endif

	var myskills := chardata.Skills;
	var primarias := array;
	//sendsysmessage(who, "skills" + myskills);
	foreach skill in (myskills.keys())
		if (myskills[skill] == 1)
			primarias.append( skill );
		endif
	endforeach


	return primarias;

endfunction


function getSecundarias(who)

	var chardata := GetObjProperty(who, "chardata");
	if ( !chardata)
		return;
	endif

	var myskills := chardata.Skills;
	var primarias := array;
	//sendsysmessage(who, "skills" + myskills);
	foreach skill in (myskills.keys())
		if (myskills[skill] == 2)
			primarias.append( skill );
		endif
	endforeach


	return primarias;

endfunction


function getPrimarias(who)

	var chardata := GetObjProperty(who, "chardata");
	if ( !chardata)
		return;
	endif

	var myskills := chardata.Skills;
	var primarias := array;
	//sendsysmessage(who, "skills" + myskills);
	foreach skill in (myskills.keys())
		if (myskills[skill] == 3)
			primarias.append( skill );
		endif
	endforeach


	return primarias;

endfunction

function CreateBasicRecipes()

	var x := array;

	var spellinfo1 := struct;
 	spellinfo1.+type := "Cura Leve";
 	spellinfo1.+id := 56321;
	spellinfo1.+description := "Sua formula ja esta bastante conhecida entre os alquimistas, basta utilizar duas raizes de ginseng e destilar o extrato. Dificuldade facil.";

	x.append(spellinfo1);

	var spellinfo2 := struct;
	spellinfo2.+type := "Cura Media";
	spellinfo2.+id := 56322;
	spellinfo2.+description := "Aumentando o numero de raizes da cura leve para quatro, obtem-se um extrato mais forte e com propriedades curativas melhores. Dificuldade media.";
	x.append(spellinfo2);


	var spellinfo3 := struct;
	spellinfo3.+type := "Veneno fraco de ervas";
        spellinfo3.+id := 0xbf00;
	spellinfo3.+description := "Veneno feito com a erva solenaceae, apenas duas plantas já podem matar pequenas criaturas. Dificuldade facil.";
	x.append(spellinfo3);

	var spellinfo4 := struct;
	spellinfo4.+type := "Veneno de ervas";
	spellinfo4.+id := 0xdc04;
	spellinfo4.+description := "Veneno feito com a erva solenaceae, com a dosagem de 4 ramos da planta, ja pode matar um homem. Dificuldade media.";
	x.append(spellinfo4);

	var spellinfo5 := struct;
	spellinfo5.+type := "Explosivo Fraco";
        spellinfo5.+id := 0xef60;
	spellinfo5.+description := "Utilizando quatro cinzas de enxofre e possivel fazer um artefato explosivo. Dificuldade baixa.";
	x.append(spellinfo5);

	var spellinfo6 := struct;
	spellinfo6.+type := "Explosivo Medio";
	spellinfo6.+id := 0xef61;
	spellinfo6.+description := "Utilizando seis porcoes cinzas de enxofre e possivel fazer um artefato explosivo que causa estragos consideraveis. Dificuldade media.";
	x.append(spellinfo6);


	var spellinfo7 := struct;
	spellinfo7.+type := "Antidoto fraco";
        spellinfo7.+id := 0xdc14;
	spellinfo7.+description := "Alho possui caracteristicas de neutralizar venenos, dois alhos podem curar um veneno fraco. Dificuldade muito baixa.";
	x.append(spellinfo7);

	var spellinfo8 := struct;
	spellinfo8.+type := "Antidoto medio";
	spellinfo8.+id := 0xdc15;
	spellinfo8.+description := "Com quatro alhos voce pode fazer um antidoto para venenos moderados. Dificuldade baixa.";
	x.append(spellinfo8);

	var spellinfo9 := struct;
        spellinfo9.+type := "Agilidade";
	spellinfo9.+id := 0xdc0c;
	spellinfo9.+description := "Duas raizes de mandragora estimulam a percepcao e agilidade de forma consideravel, tornando o usuario mais agil e veloz. Dificuldade facil.";
	x.append(spellinfo9);


	var spellinfo10 := struct;
 	spellinfo10.+type := "Vigor";
        spellinfo10.+id := 0xdc0e;
	spellinfo10.+description := "Perolas negras possuem substancias energeticas, duas delas podem fazer voce recuperar-se da fadiga e cansaco. Dificuldade baixa.";
	x.append(spellinfo10);


	var spellinfo11 := struct;
	spellinfo11.+type := "Forca";
        spellinfo11.+id := 0xdc11;
	spellinfo11.+description := "Dois musgos vermelhos conseguem aumentar consideravelmente a forca do usuario. Dificuldade baixa.";
	x.append(spellinfo11);


	var spellinfo12 := struct;
	spellinfo12.+type := "Aprimorar Visao";
        spellinfo12.+id := 56550;
	spellinfo12.+description := "Duas teias de aranha gigante fazem a pupila dos seus olhos dilatarem e receberem mais luz, consequentemente ambientes escuros ou com pouca luz ficam bem claros para quem beber esta pocao. Dificuldade baixa.";
	x.append(spellinfo12);


	var spellinfo13 := struct;
	spellinfo13.+type := "Inteligencia";
        spellinfo13.id := 0xbf07;
	spellinfo13.+description := "Dois cogumelos negros amplificam consideravelmente sua atividde cerebral acelerando o raciocinio. Dificuldade baixa.";
	x.append(spellinfo13);




	return x;
endfunction

function CreateVeryBasicRecipes()
	var x := array;

	var spellinfo1 := struct;
 	spellinfo1.+type := "Pocao de Cura Leve";
 	spellinfo1.+id := 56321;
	spellinfo1.+description := "Sua formula ja esta bastante conhecida entre os alquimistas, basta utilizar duas raizes de ginseng e destilar o extrato. Dificuldade facil.";

	x.append(spellinfo1);

	var spellinfo3 := struct;
	spellinfo3.+type := "Veneno Fraco de Ervas";
    spellinfo3.+id := 0xbf00;
	spellinfo3.+description := "Veneno feito com a erva solenaceae, apenas duas plantas já podem matar pequenas criaturas. Dificuldade facil.";
	x.append(spellinfo3);

	var spellinfo7 := struct;
	spellinfo7.+type := "Antidoto Fraco";
    spellinfo7.+id := 0xdc14;
	spellinfo7.+description := "Alho possui caracteristicas de neutralizar venenos, dois alhos podem curar um veneno fraco. Dificuldade muito baixa.";
	x.append(spellinfo7);

	var spellinfo12 := struct;
	spellinfo12.+type := "Aprimorar Visao";
    spellinfo12.+id := 56550;
	spellinfo12.+description := "Duas teias de aranha gigante fazem a pupila dos seus olhos dilatarem e receberem mais luz, consequentemente ambientes escuros ou com pouca luz ficam bem claros para quem beber esta pocao. Dificuldade baixa.";
	x.append(spellinfo12);

	return x;
endfunction

