include ":attributes:attributes";
include ":tn:cooldown";
include ":charactercreation:habilidades";
include "include/say";
include "include/creationUtils";
include "include/epicConstants";

const FORTITUDE := "Fortitude";
const REFLEXOS := "Reflexos";
const VONTADE := "Vontade";

function GetResist(who, resist)
	var chardata := GetObjProperty(who, "chardata");
	var classe := chardata.classe;
	var vantagem := chardata.vantagemracial;
	var kit := chardata.trait;
	var raca := chardata.raca;
	var valor := 0;

	Recalcvitals(who);
    if (resist == VONTADE && (TemHabilidade(who, "Ladrao de Magias")))
		resist := REFLEXOS;
	endif

	if (resist == VONTADE && (TemHabilidade(who, "Fúria Indomável") && GetCooldown(who, "furia")))
		resist := FORTITUDE;
	endif
	
	if (kit == "Fortitude do Urso")
		if (resist == FORTITUDE)
			valor := valor + 5;
		endif
	elseif (kit == "Reflexos de Pantera")
		if (resist == REFLEXOS)
			valor := valor + 5;
		endif
	elseif (kit == "Vontade da Aguia")
		if (resist == VONTADE)
			valor := valor + 5;
		endif
	endif

	if (resist == FORTITUDE)
		if (TemHabilidade(who, "Caçador Implacável"))
			valor := valor + 2;
        endif
        if (TemHabilidade(who, "Campeao das Montanhas"))
            valor := valor + 2;
		endif

		if (GetObjProperty(who, "corruption"))
			var corruption_penalty := GetObjProperty(who, "corruption_penalty");
			valor := valor - corruption_penalty;
		endif

		if (classe == "Guerreiro")
			valor := valor + 5;
		else
			valor := valor + 2;
		endif

		valor := valor + CInt(AP_GetStat(who, STRENGTH)/20);
		valor := valor + CInt(GetObjProperty(who, "#FortitudeMod"));
	elseif (resist == REFLEXOS)
		if (TemHabilidade(who, "Reaproveitar Pergaminho"))
			valor := valor + 5;
		endif
		if (classe == "Ladino")
			valor := valor + 5;
		else
			valor := valor + 2;
		endif

		valor := valor + CInt(AP_GetStat(who, DEXTERITY)/20); 
		valor := valor + CInt(GetObjProperty(who, "#ReflexosMod"));
	else // (resist == VONTADE)
		if (classe == "Sabio")
			valor := valor + 5; 
		else
			valor := valor + 2;
		endif

		valor := valor + CInt(AP_GetStat(who, INTELLIGENCE)/20);
		valor := valor + CInt(GetObjProperty(who, "#VontadeMod"));
		if ((GetObjProperty(who, "chardata")).culture == ALURIEL)
			valor := valor + 2;
		endif

		if ( GetCooldown(who, "VontadeIndomavel"))
			valor := valor + 5;
		elseif (temHabilidade(who, "Dominatio Animae"))
			valor := valor + 5;
        elseif (TemHabilidade(who, "Campeao das Montanhas"))
            valor := valor + 2;
		endif
	endif

    if (raca == HUMANO)
		valor := valor + 2;
    endif

	if (TemHabilidade(who, "Shintai to Kokoro"))
		valor := valor + 5;
	endif

	if (who.acctname)
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

function CheckResist(defender, resist_type, dif, mod := 0)
    var dado := RandomDiceRoll("1d20");
    var valor := GetResist(defender, resist_type);
    var chance := dado + valor + mod;

    if (dado == 1)
        SendSysMessageEx(defender, "Falha crítica no teste de resistência", SSM_FAIL);
        return 0;
    elseif (dado == 20)
        SendSysMessageEx(defender, "Sucesso crítico no teste de resistência", SSM_INFO);
        return 1;
    endif

    SendSysMessageEx(defender, "Chance: " + chance + " Dif: " + dif, SSM_INFO);
    
    if (chance >= dif)
        return 1;
    else
        if (ShouldGetSecondChance(defender, resist_type))
            dado := RandomDiceRoll("1d20");
            chance := dado + valor + mod;
            if (chance >= dif)
                SendSysMessageEx(defender, GetSecondChanceMessage(defender, resist_type), SSM_INFO);
                return 1;
            endif
        endif
        
        SendSysMessageEx(defender, "Falhou no teste de resistência", SSM_INFO);
        return 0;
    endif
endfunction

function ShouldGetSecondChance(defender, resist_type)
    var chardata := GetObjProperty(defender, "chardata");
    return (
        (chardata.culture == HAROLOTH && resist_type == FORTITUDE) ||
        (TemHabilidade(defender, "Fortitudo Impia") && resist_type == FORTITUDE) ||
        (chardata.raca == ANAO && resist_type == VONTADE) ||
        (chardata.culture == POLKINEA)
    );
endfunction

function GetSecondChanceMessage(defender, resist_type)
    var chardata := GetObjProperty(defender, "chardata");
    if (chardata.culture == HAROLOTH && resist_type == FORTITUDE)
        return "Seu organismo drow expurga o veneno";
    elseif (TemHabilidade(defender, "Fortitudo Impia") && resist_type == FORTITUDE)
        return "Sua resiliência ímpia expurga o veneno";
    elseif (chardata.raca == ANAO && resist_type == VONTADE)
        return "Sua constituição anã o salvou da magia";
    elseif (chardata.culture == POLKINEA)
        return "Um polski sempre pode contar com a própria sorte";
    endif
    return "";
endfunction