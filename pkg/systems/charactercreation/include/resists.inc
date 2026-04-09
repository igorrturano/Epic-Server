include ":attributes:attributes";
include ":tn:cooldown";
include ":charactercreation:habilidades";
include "include/say";
include "include/creationUtils";
include "include/epicConstants";
include ":wrappers:Objects/Model/MobileWrapper";
include ":abilities:models/Ability";

 // ATENçÃO: este metodo só existe para compatibilidade com o sistema antigo de resistências
 // A atuação das resistências agora é feita via atributos intrínsecos e ganchos. veja: pkg/systems/attributes/hooks/intrinsicMods.src
function GetResist(who, resist)
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
        (MobileWrapper(defender).HasAbility(ABILITY_ID::AURA_DIVINA) && resist_type == FORTITUDE) ||
        (chardata.raca == ANAO && resist_type == WILL) ||
        (chardata.culture == COLHESIANA) ||
		(MobileWrapper(defender).GetEntropy() <= -8000 && resist_type == FORTITUDE)
    );
endfunction

function GetSecondChanceMessage(defender, resist_type)
    var chardata := GetObjProperty(defender, "chardata");
    if (chardata.culture == HAROLOTH && resist_type == FORTITUDE)
        return "Seu organismo drow expurga o veneno";
    elseif (MobileWrapper(defender).HasAbility(ABILITY_ID::AURA_DIVINA) && resist_type == FORTITUDE)
        return "Sua aura divina expurga o veneno";
    elseif (chardata.raca == ANAO && resist_type == WILL)
        return "Sua constituição anã o salvou da magia";
    elseif (chardata.culture == COLHESIANA)
        return "Um pequilho sempre pode contar com a própria sorte";
	elseif (MobileWrapper(defender).GetEntropy() <= -8000 && resist_type == FORTITUDE)
		return "Sua alma corrupta expurga o veneno";
    endif
    return "";
endfunction
