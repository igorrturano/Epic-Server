include ":attributes:attributes";
include ":tn:cooldown";
include ":charactercreation:habilidades";
include "include/say";
include "include/creationUtils";

const FORTITUDE := "Fortitude";
const REFLEXOS := "Reflexos";
const VONTADE := "Vontade";
//const VALOR_REFERENCIA := 10;

//------------------------------------------------------
//DIFICULDADES REFERÊNCIA PARA OS TESTES DE RESISTÊNCIA
//
//Fácil 				30
//Médio 				45
//Difícil 				60
//Muito Difícil 		75
//Extremamente Difícil 	90
//Quase Impossível 		105
//
//------------------------------------------------------


function GetResist(who, resist)
	// Coleta informações da ficha do personagem
	var chardata := GetObjProperty(who, "chardata");
	var classe := chardata.classe;
	var vantagem := chardata.vantagemracial;
	var kit := chardata.trait;
	var valor := 0;

	EraseResist(who, resist);//Para uso da barra de status, apaga o que tem para calcular de novo
    // Rework do TH, IGOR. Mudei pra maos magicas
 	// Faz a resistência de Vontade para utilizar o valor de Reflexos para quem tem habilidade "Mente Evasiva"
	if (resist == VONTADE && (TemHabilidade(who, "Ladrao de Magias")))
		resist := REFLEXOS;
	endif

	// Aplica os bonus e redutores dos Kits
	if (kit == "Fortitude do Urso")
		if (resist == FORTITUDE)
			valor := valor + 30;
		endif
	elseif (kit == "Reflexos de Pantera")
		if (resist == REFLEXOS)
			valor := valor + 30;
		endif
	elseif (kit == "Vontade da Aguia")
		if (resist == VONTADE)
			valor := valor + 30;
		endif
	endif

	if (resist == FORTITUDE)
		if (TemHabilidade(who, "Grande Fortitude"))
			valor := valor + 30;
        endif
        if (TemHabilidade(who, "Campeao das Montanhas"))
            valor := valor + 20;
		endif

		// Aplica os bonus base do Caminho para a resistência
		if (classe == "Guerreiro")
			valor := valor + 20;
		elseif (classe == "Artifice")
			valor := valor + 15;
		else
			valor := valor + 10;
		endif
        if (vantagem == "Resiliente")
			valor := valor + 20; // +200% do VALOR_REFERENCIA
		endif
		// Aplica bonus de Atributo atual para a resistência
		valor := valor + CInt(AP_GetStat(who, STRENGTH)/4); // STRENGTH % do VALOR_REFERENCIA

		// Aplica os modificadores temporários para a resistência
		valor := valor + CInt(GetObjProperty(who, "#FortitudeMod"));

	elseif (resist == REFLEXOS)
		if (TemHabilidade(who, "Reaproveitar Pergaminho"))
			valor := valor + 40;
		endif
		// Aplica os bonus base do Caminho para a resistência
		if (classe == "Ladino")
			valor := valor + 20;
		elseif (classe == "Artifice")
			valor := valor + 15;
		else
			valor := valor + 10;
		endif
        if (vantagem == "Agilidade")
			valor := valor + 20; // +200% do VALOR_REFERENCIA
		endif
		// Aplica bonus de Atributo atual para a resistência
		valor := valor + CInt(AP_GetStat(who, DEXTERITY)/4); // DEXTERITY % do VALOR_REFERENCIA


		// Aplica os modificadores temporários para a resistência
		valor := valor + CInt(GetObjProperty(who, "#ReflexosMod"));

	else // (resist == VONTADE)

		// Aplica os bonus base do Caminho para a resistência
		if (classe == "Sabio")
			valor := valor + 20; // +300% do VALOR_REFERENCIA
		elseif (classe == "Artifice")
			valor := valor + 15;
		else
			valor := valor + 10; // +150% do VALOR_REFERENCIA
		endif

		// Aplica bonus de Atributo atual para a resistência
		valor := valor + CInt(AP_GetStat(who, INTELLIGENCE)/4); // INTELLIGENCE % do VALOR_REFERENCIA

		// Aplica os modificadores temporários para a resistência
		valor := valor + CInt(GetObjProperty(who, "#VontadeMod"));

		// Aplica modificadores de vantagens raciais
		if ((GetObjProperty(who, "chardata")).raca == RACA.ELFO)
		//if (vantagem == "Vontade de Ferro" || who.npctemplate)
			valor := valor + 20; // +200% do VALOR_REFERENCIA
		endif

		// Aplica modificadores decorrentes de habilidades
		if ( GetCooldown(who, "VontadeIndomavel")) // Habilidade ativa
			valor := valor + 40; // +400% do VALOR_REFERENCIA
		elseif (temHabilidade(who, "Devocao")) // Habilidade passiva
			valor := valor + 40; // +400% do VALOR_REFERENCIA
        elseif (TemHabilidade(who, "Campeao das Montanhas"))
            valor := valor + 20;
		endif
	endif

    //vantagem dos zharkianos q da +10 nos 3 resists
    if (vantagem == "Resistentes")
		valor := valor + 10; // +100% do VALOR_REFERENCIA
    endif

//print("o valor eh "+valor);
//print("os valores sao "+who.lower_reagent_cost+who.spell_damage_increase+who.faster_casting);

if (!who.npcdesc) //se nao for jogador passa reto
	if(resist == FORTITUDE)
		var fortitude_mod := Cint(who.lower_reagent_cost_mod) + valor;
		who.lower_reagent_cost_mod := fortitude_mod;
	elseif(resist == VONTADE)
		var vontade_mod := Cint(who.spell_damage_increase_mod) + valor;
		who.spell_damage_increase_mod := vontade_mod;
	else
		var reflexos_mod := Cint(who.faster_casting_mod) + valor;
		who.faster_casting_mod := reflexos_mod;
	endif
	return valor;
endif
endfunction

function CheckResist(who, resist, dif, mod := 0)

	// Rola o dado da aleatoriedade permitindo superar testes de até +400% do VALOR_REFERENCIA além da resistência base do personagem
	var dado := RandomInt(40) + 1;
	var valor := GetResist(who, resist);

	//printtextabove(who, "dado " + dado + " valor " + valor + " diff " + dif);
	if (dado + valor + mod >= dif)
		return 1;

	//Quando é Zharkiano e estiver Bloodied ativa a habilidade racial "Segunda chance"
	elseif ((GetObjProperty(who, "chardata")).raca == RACA.HUMANO && IsBloodied(who))
		dado := RandomInt(40) + 1;
		if (dado + valor + mod >= dif)
			SendSysMessageEx(who, "Humanos são insistentes!", SSM_INFO);
			return 1;
		endif
    elseif((GetObjProperty(who, "chardata")).vantagemracial == "Toxina natural" && resist == FORTITUDE)
        mod += 30;
		if (dado + valor + mod >= dif)
			SendSysMessageEx(who, "Seu organismo drow expurga o veneno", SSM_INFO);
			return 1;
		endif
    elseif(GetObjProperty(who, "chardata").vantagemracial == "Constituicao Ana" && resist == VONTADE)
        mod += 30; 
		if (dado + valor + mod >= dif)
			SendSysMessageEx(who, "Sua constituição anã o salvou da magia", SSM_INFO);
			return 1;
		endif
    elseif(GetObjProperty(who, "chardata").culture == CULTURA.POLKINEA)
		dado := RandomInt(40) + 1;
		if (dado + valor + mod >= dif)
			SendSysMessageEx(who, "Um polski sempre pode contar com a própria sorte", SSM_INFO);
			return 1;
		endif
	//Não passando no teste de resistência, informa ao pobre infeliz.
	else
		SendSysMessageEx(who, "Falhou no teste de resistência", SSM_INFO);
		return 0;
	endif
endfunction

function EraseResist(who, resist)

if (!who.npcdesc)
	if(resist == FORTITUDE)
		who.lower_reagent_cost_mod := 0;
		who.lower_reagent_cost := 0;
	elseif(resist == VONTADE)
		who.spell_damage_increase_mod := 0;
		who.spell_damage_increase := 0;
	else
		who.faster_casting_mod := 0;
		who.faster_casting := 0;
	endif
endif
	Recalcvitals(who);
endfunction
