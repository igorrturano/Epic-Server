include ":attributes:attributes";
include ":tn:cooldown";
include ":charactercreation:habilidades";
include "include/say";
include "include/creationUtils";
include "include/epicConstants";

function GetResist(who, resist) // ATENçÃO: este metodo só existe para compatibilidade com o sistema antigo de resistências
	return AP_GetStat(who, resist);
endfunction

function CheckResist(defender, resist_type, dif, mod := 0)
    var dado := RandomDiceRoll("1d20");
    var valor := GetResist(defender, resist_type);
    var chance := dado + valor + mod;

    if (dado == 1)
        SendSysMessageEx(defender, "Falha crítica no teste de resistência", SSM_FAIL);
        return -1;
    elseif (dado == 20)
        SendSysMessageEx(defender, "Sucesso crítico no teste de resistência", SSM_INFO);
        return 1;
    endif

	if (defender.cmdlevel > 2)
    	SendSysMessageEx(defender, "Chance: " + chance + " Dif: " + dif, SSM_INFO);
    endif

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
        
        SendSysMessageEx(defender, "Falhou no teste de resistência", SSM_FAIL);
        return 0;
    endif

	return 0;
endfunction

function ShouldGetSecondChance(defender, resist_type)
    var chardata := GetObjProperty(defender, "chardata");
    return (
        (chardata.culture == HAROLOTH && resist_type == FORTITUDE) ||
        (TemHabilidade(defender, "Aura Divina") && resist_type == FORTITUDE) ||
        (chardata.raca == ANAO && resist_type == WILL) ||
        (chardata.culture == POLKINEA)
    );
endfunction

function GetSecondChanceMessage(defender, resist_type)
    var chardata := GetObjProperty(defender, "chardata");
    if (chardata.culture == HAROLOTH && resist_type == FORTITUDE)
        return "Seu organismo drow expurga o veneno";
    elseif (TemHabilidade(defender, "Aura Divina") && resist_type == FORTITUDE)
        return "Sua aura divina expurga o veneno";
    elseif (chardata.raca == ANAO && resist_type == WILL)
        return "Sua constituição anã o salvou da magia";
    elseif (chardata.culture == POLKINEA)
        return "Um pequilho sempre pode contar com a própria sorte";
    endif
    return "";
endfunction
