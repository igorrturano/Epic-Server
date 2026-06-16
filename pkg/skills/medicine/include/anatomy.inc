/* $Id: anatomy.src 834 2005-11-02 14:09:50Z austin $
 * Foi deletado o anatomy.src e passado tudo para cá, teste para não ser mais possível realizar diagnostico com .skill medicine, apenas com estetoscopio
 */ 
use uo;

include ":charactercreation:resists";
include ":attributes:attributeConstants";
include ":attributes:skills";
include ":attributes:attributes";
include ":attributes:skillCheck";
include "include/say";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";

/*program skill_Anatomy(who)

	EraseObjProperty(who, "IsMeditating");
	EraseObjProperty(who, "HealTimer");

	SendSysMessageEx(who, "Quem voce deseja examinar?", SSM_REQUEST);
	var subject := Target(who);
	// target checks begin here
	if(!subject)
		return 0;
	elseif(subject.graphic == 0x2006)
		PrintTextAbove(who, "*examinando o corpo*");
		if (MobileWrapper(who).HasAbility(ABILITY_ID::ACADEMIC_TRAINING))
			if (GetObjProperty(subject, "morto") == 1)
				SendSysMessageEx(who, "Parece morto para voce.", SSM_INFO);
				if( SkillCheck(who, MEDICINE, 50) > 0 )
					if (GetObjProperty(subject, "WeaponKill"))
						SendSysMessageEx(who, "O corpo foi morto por: " + GetObjProperty(subject, "WeaponKill"), SSM_INFO);
					endif
					if( SkillCheck(who, MEDICINE, 70) > 0 )
						if (GetObjProperty(subject, "deathtimer"))
							SendSysMessageEx(who, "Foi morto a: " + tempoMorto(subject, "deathtimer"), SSM_INFO);
						endif
						if( SkillCheck(who, MEDICINE, 90) > 0 )
							if (GetObjProperty(subject, "deathdetails"))
								SendSysMessageEx(who, "Detalhes: " + GetObjProperty(subject, "deathdetails"), SSM_INFO);
							endif
						endif
					endif
				endif
				return 0;
			else
				var mob := SystemFindObjectBySerial(subject.ownerserial);
				if (mob)
					if (mob.concealed)
						SendSysMessageEx(who, "Parece morto para voce.", SSM_INFO);
						return 0;
					endif
				endif
				SendSysMessageEx(who, "Esta desacordado.", SSM_INFO);
				return 0;
			endif
		else
			SendSysMessageEx(who, "Voce nao tem certeza...", SSM_FAIL);
			return 0;
		endif
	elseif(subject.objtype == 0xEFFF)
		PrintTextAbove(who, "*examinando os ossos*");
		if (MobileWrapper(who).HasAbility(ABILITY_ID::ACADEMIC_TRAINING))
			SendSysMessageEx(who, "Parece inanimado para voce.", SSM_INFO);
			if( SkillCheck(who, MEDICINE, 50) > 0 )
				if (GetObjProperty(subject, "WeaponKill"))
					SendSysMessageEx(who, "O corpo foi morto por: " + GetObjProperty(subject, "WeaponKill"), SSM_INFO);
				endif
				if( SkillCheck(who, MEDICINE, 70) > 0 )
					if (GetObjProperty(subject, "deathtimer"))
						SendSysMessageEx(who, "Foi morto a: " + tempoMorto(subject, "deathtimer"), SSM_INFO);
					endif
					if( SkillCheck(who, MEDICINE, 90) > 0 )
						if (GetObjProperty(subject, "enterrado"))
							SendSysMessageEx(who, "Foi enterrado a: " + tempoMorto(subject, "enterrado"), SSM_INFO);
						endif
						if (GetObjProperty(subject, "deathdetails"))
							SendSysMessageEx(who, "Detalhes: " + GetObjProperty(subject, "deathdetails"), SSM_INFO);
						endif
					endif
				endif
			endif
			return 0;
		else
			SendSysMessageEx(who, "Voce nao tem certeza...", SSM_FAIL);
			return 0;
		endif
		return 0;
	elseif(!subject.isA(POLCLASS_MOBILE))
		SendSysMessageEx(who, "Alvo invalido.", SSM_FAIL);
		return 0;
	endif

	// variables for sys messages
  	var streng := getStreng(AP_GetStat(subject, "Strength"));
	var dext := getDext(AP_GetStat(subject, "Dexterity"));

	var fort := getFort(GetResist(subject, FORTITUDE));
	var refl := getRefl(GetResist(subject, REFLEXOS));
	var vont := getVont(GetResist(subject, VONTADE));
	//var con := getVit( Cint(GetObjProperty(who, "Vitality") ));
	var endur_percentage := CInt(CInt(AP_GetVital(subject, "Stamina"))/CDbl(AP_GetVitalMaximumValue(subject, "Stamina"))*100);
	var endur := getEnd(endur_percentage);


	var strMsg, strFail;

	if(subject.serial == who.serial)
		strMsg := "-se";
	else
		strMsg := " "+subject.name;
		strFail := "das caracteristicas fisicas dessa criatura.";
	endif

	PrintTextAbove(who, "*examinando" + strMsg + "*");
	if (!Wait(subject, 2))
		SendSysMessageEx(who, "O paciente precisa ficar preparado.", SSM_FAIL);
		return 0;
	endif
	var hunger := CInt(AP_GetVital(subject, "Hunger") / 10);
	PrintTextPrivate(subject, "Niveis de fome: " + hunger , who);

	var sucesso := 0;
	if (who.serial == subject.serial) sucesso := 1; //PARA O PLAYER SEMPRE ACERTAR E NUNCA UPAR, CASO USE NELE MESMO
	else sucesso := SkillCheck(who, MEDICINE, 30);
	endif

	if (MobileWrapper(who).HasAbility(ABILITY_ID::ACADEMIC_TRAINING))
		if(sucesso > 0)
			SendSysMessageEx(who, strMsg + " parece " + streng +" (" +AP_GetStat(subject, "Strength") + "),", SSM_INFO);
			SendSysMessageEx(who, dext +" (" + AP_GetStat(subject, "Dexterity") + "),", SSM_INFO);
			SendSysMessageEx(who, endur + ".", SSM_INFO);
			SendSysMessageEx(who, fort + " (" + GetResist(subject, FORTITUDE) + "),", SSM_INFO);
			SendSysMessageEx(who, refl + " (" + GetResist(subject, REFLEXOS) + "),", SSM_INFO);
			SendSysMessageEx(who, vont + " (" + GetResist(subject, VONTADE) + "),", SSM_INFO);
            if (who.serial == subject.serial)
			    SendSysMessageEx(who, "O alvo esta " + DPMsg(subject,who), SSM_INFO);
            endif
			return 1;
		else
			SendSysMessageEx(who, "Voce nao faz ideia " + strFail, SSM_FAIL);
			return 0;
		endif
	else
		if(sucesso > 0)
			SendSysMessageEx(who, strMsg + " parece " + streng + ", " + dext + " e " + endur + ".", SSM_INFO);
			if ((valorInicial(who, MEDICINE) >= 40) || (who.serial == subject.serial))
				SendSysMessageEx(who, strMsg + " parece " + fort + ", " + refl + " e "+ vont + ".", SSM_INFO);
			endif
            if (who.serial == subject.serial)
			   SendSysMessageEx(who, "O alvo esta " + DPMsg(subject,who), SSM_INFO);
			elseif (GetObjProperty(subject, "bond") == "fiel")
			   if (AP_Getvital(subject, "DP") == 20)
				SendSysMessageEx(who, "O alvo esta bastante disposto", SSM_INFO);
			   else
				SendSysMessageEx(who, "O alvo esta quase morto", SSM_INFO);
				endif
			endif
            return 1;
		else
			SendSysMessageEx(who, "Voce nao faz ideia " + strFail, SSM_FAIL);
			return 0;
		endif
	endif
endprogram
*/

// Formata o tipo de dano para exibição
function FormatDamageType(damage_type)
    if (typeof(damage_type) != "String")
        return "Indeterminado";
    endif

    var t := damage_type;
    if (len(t) > 6 && t[len(t) - 5, 6] == "Resist")
        t := t[1, len(t) - 6];
    elseif (len(t) > 6 && t[len(t) - 5, 6] == "Damage")
        t := t[1, len(t) - 6];
    endif

    case (Upper(t))
        "SLASHDAMAGE":
        "SLASH":
            return "Cortante";
        "PIERCINGDAMAGE":
        "PIERCING":
            return "Perfurante";
        "BLUNTDAMAGE":
        "BLUNT":
            return "Contundente";
        "FIRE":     return "Fogo";
        "COLD":     return "Gelo";
        "POISON":   return "Veneno";
        "ENERGY":   return "Energia";
        "FORCED":   return "Místico";
        default:    return "Indeterminado";
    endcase
endfunction

function getStreng(subject, graphic, gender)
    var strSuper, strType, strGender;
    if(graphic != 0x190 && graphic != 0x191)
        strSuper := "ter força";
        strType := "criaturas";
        strGender := "ela";
    else
        strSuper := "ter força";
        strType := "pessoas";
        if(gender == 0)
            strGender := "ele";
        elseif(gender == 1)
            strGender := "ela";
        endif
    endif
    if(subject >= 120)
        return strSuper + " sobrehumana";
    elseif(subject >= 110)
        return "ser como uma das mais fortes " + strType + " que você já viu";
    elseif(subject >= 100)
        return "ser forte como uma besta";
    elseif(subject >= 90)
        return "ser extraordinariamente forte";
    elseif(subject >= 80)
        return "ser bem forte";
    elseif(subject >= 70)
        return "ser forte";
    elseif(subject >= 60)
        return "ser um pouco forte";
    elseif(subject >= 50)
        return "ser normal";
    elseif(subject >= 40)
        return "ser fraco";
    elseif(subject >= 30)
        return "ser bastante fraco";
    else
        return "ter problemas levantando uma pena";
    endif
endfunction


// dexterity
function getDext(subject, graphic, gender)
    // fix sys msgs
    var strSuper, strType, strGender;
    if(graphic != 0x190 && graphic != 0x191)
        strSuper := "anormalmente";
        strType := "criaturas";
        strGender := "ela";
    else
        strSuper := "sobre-humanamente";
        strType := "pessoa";
        // Determine the proper pronoun for later use in messages.
        if(gender == 0)
            strGender := "ele";
        elseif(gender == 1)
            strGender := "ela";
        endif
    endif
    if(subject >= 100)
        return "sobre-humananmente ágil";
    elseif(subject >= 99)
        return "uma das pessoas mais rápidas que você já viu";
    elseif(subject >= 89)
        return "ágil como mercúrio";
    elseif(subject >= 79)
        return "extraordinariamente ágil";
    elseif(subject >= 69)
        return "extremamente ágil";
    elseif(subject >= 59)
        return "muito ágil";
    elseif(subject >= 49)
        return "um pouco ágil";
    elseif(subject >= 39)
        return "moderadamente ágil";
    elseif(subject >= 29)
        return "meio descoordenado";
    elseif(subject >= 19)
        return "muito pouco ágil";
    else
        return "mal consegue se mover";
    endif
endfunction

function getHit(subject)
	if(subject >= 140)
		return "parece ter saude sobre-humana";
	elseif(subject >= 130)
		return "uma das pessoas mais saudaveis que voce ja viu";
	elseif(subject >= 120)
		return "parece que nunca deve ficar doente";
	elseif(subject >= 110)
		return "extraordinariamente saudavel";
	elseif(subject >= 100)
		return "extremamente saudavel";
	elseif(subject >= 90)
		return "muito saudavel";
	elseif(subject >= 80)
		return "um pouco saudavel";
	elseif(subject >= 70)
		return "moderadamente saudavel";
	elseif(subject >= 60)
		return "parece ter a saude comprometida";
	elseif(subject >= 50)
		return "parece doentio";
	else
		return "";
	endif

endfunction

function getSta(subject)
	if(subject >= 100)
		return "com disposicao completa";
	elseif(subject >= 90)
		return "com muita disposicao";
	elseif(subject >= 80)
		return "com disposicao";
	elseif(subject >= 70)
		return "com alguma disposicao";
	elseif(subject >= 60)
		return "com pouca disposicao";
	elseif(subject >= 50)
		return "com muita pouca disposicao";
	elseif(subject >= 40)
		return "com quase nenhuma diposicao";
	elseif(subject >= 30)
		return "com cansaco";
	elseif(subject >= 20)
		return "com muito cansaco";
	elseif(subject >= 10)
		return "quase exausto";
	else
		return "totalmente exausto";
	endif

endfunction

function getMana(subject)
	if(subject >= 120)
		return "de conexão profunda com a magia";
	elseif(subject >= 80)
		return "tem bastante aptidão para as artes arcanas";
	elseif(subject >= 60)
		return "que a magia parece emanar acima do normal em seu corpo";
	elseif(subject >= 40)
		return "algum dom para magia";
	else
		return "incongruente com artes arcanas";
	endif

endfunction

function DPMsg(who, medic)
    if (GetObjProperty(who, "morto"))
        return "morto";
    endif
    if(MobileWrapper(medic).HasAbility(ABILITY_ID::ACADEMIC_TRAINING))
        var DPs := CInt(AP_GetVital(who, "DP") / 10);
        case (DPs)
            0: return "morto (0 DP)";
            1: return "quase morto (1 DP)";
            2: return "mal (2 DP)";
            3: return "relativamente bem (3 DP)";
            4: return "bastante disposto (4 DP)";
            5: return "preparado para tudo (5 DP)";
            6: return "mais do que preparado (6 DP)";
            7: return "bem mais do que preparado (7 DP)";
            8: return "extremamente preparado (8 DP)";
            default: return "extremamente preparado ("+ DPs +" DP)";
        endcase
    elseif(who.serial == medic.serial)
        var DPs := CInt(AP_GetVital(who, "DP") / 10);
        case (DPs)
            0: return "morto";
            1: return "quase morto";
            2: return "mal";
            3: return "relativamente bem";
            4: return "bastante disposto";
            5: return "preparado para tudo";
            6: return "mais do que preparado";
            7: return "bem mais do que preparado";
            8: return "extremamente preparado";
            default: return "extremamente preparado";
        endcase
    else
        var FakeDPs := CInt(AP_GetVital(who, "DP") / 10) - CInt(GetObjProperty(who, "dpmod")/10);
        case (FakeDPs)
            0: return "morto";
            1: return "quase morto";
            2: return "mal";
            3: return "relativamente bem";
            4: return "relativamente bem";
            5: return "bastante disposto";
            default: return "quase morto";
        endcase
    endif
endfunction


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

function getFort(subject)
	if (subject >= 35)
        return "de um deus";
    elseif(subject >= 30)
        return "de um semi-deus";
    elseif(subject >= 25)
        return "excepcional";
    elseif(subject >= 20)
        return "forte";
    elseif(subject >= 15)
        return "mediano";
    elseif(subject >= 10)
        return "frágil";
    elseif(subject >= 0)
        return "muito frágil";
    else
        return "inexistente";
    endif
endfunction

function getRefl(subject)
	if (subject >= 105)
		return "ter reflexos de um deus";
	elseif(subject >= 90)
		return "ter reflexos de um semi-deus";
	elseif(subject >= 75)
		return "ter reflexos excepcionalmente rapidos";
	elseif(subject >= 60)
		return "ter reflexos muito rapidos";
	elseif(subject >= 45)
		return "ter reflexos rapidos";
	elseif(subject >= 30)
		return "ter reflexos medianos";
	elseif(subject >= 15)
		return "ter reflexos lentos";
	elseif(subject >= 0)
		return "ter reflexos muito lentos";
	else
		return "nao ter reflexo algum";
	endif
endfunction

function getVont(subject)
	if (subject >= 105)
		return "ser perspicaz como um deus";
	elseif(subject >= 90)
		return "ser perspicaz como um semi-deus";
	elseif(subject >= 75)
		return "ser excepcionalmente perspicaz";
	elseif(subject >= 60)
		return "ser muito perspicaz";
	elseif(subject >= 45)
		return "ser perspicaz";
	elseif(subject >= 30)
		return "ter uma perspicacia mediocre";
	elseif(subject >= 15)
		return "ser pouco perspicaz";
	elseif(subject >= 0)
		return "ser muito pouco perspicaz";
	else
		return "ser nada perspicaz";
	endif
endfunction

// intelligence
function getInt(subject)
    if(subject >= 100)
        return "ser um gênio";
    elseif(subject >= 90)
        return "ter inteligência excepcional";
    elseif(subject >= 80)
        return "ser muito inteligente";
    elseif(subject >= 70)
        return "ter boa capacidade intelectual";
    elseif(subject >= 60)
        return "ter inteligência acima da média";
    elseif(subject >= 50)
        return "ter inteligência mediana";
    elseif(subject >= 40)
        return "ter inteligência abaixo da média";
    elseif(subject >= 30)
        return "ter dificuldades de aprendizado";
    elseif(subject >= 20)
        return "ter sérias limitações cognitivas";
    elseif(subject >= 10)
        return "ter capacidade mental muito reduzida";
    else
        return "ter comprometimento cognitivo severo";
    endif
endfunction

function tempoMorto(character, tipo)// para 10000 deu 292 segundos 300
    var criadoEm := cint(GetObjProperty(character, tipo));
    var tempoAgora := polcore().systime;// segundos
    var tempoDevida:= tempoAgora - criadoEm;
    var dias := cint(tempoDevida/(86400));//24horas*60minutos*60segundos = 86400 segundos
	if (dias <= 2)
		dias := "Corpo quente";
	elseif (dias > 2 && dias <= 8)
		dias := "Corpo frio";
	elseif (dias > 8 && dias <= 15)
		dias := "Corpo gelado";
	else
		dias := "Corpo decomposto";
	endif
    return dias;
endfunction

function CopyProps(origin, destiny)
	var propnames := GetObjPropertyNames(origin);
	foreach prop in propnames
		SetObjPRoperty(destiny, prop, GetObjProperty(origin, prop));
	endforeach
endfunction
