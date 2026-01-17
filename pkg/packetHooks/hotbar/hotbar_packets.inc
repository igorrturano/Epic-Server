include "include/shared_constants"; // Action types, skill maps
include "include/hotbar_actions";   // Shared execution functions (DRY)
include ":spells:include/spell_executor";

exported function HandleHotbarPacket(who, packet)
    var subcommand := packet.GetInt16(7);
    case (subcommand)
        SUBCOMMAND_REQUEST:
            return HandleRequest(who, packet);
        SUBCOMMAND_EXECUTE:
            return HandleExecute(who, packet);
        default:
            return 0;
    endcase
endfunction

// Handle data requests from client
function HandleRequest(who, packet)
    
    var serial := packet.GetInt32(3);
    var request_type := packet.GetInt8(9);

    return SendAllHotbarData(who);
endfunction

// Handle execute commands from client
function HandleExecute(who, packet)
    if (who.dead)
        Message("You cannot do that while dead!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
        return 1;
    endif

    var actionId   := packet.GetInt8(9); 
    var actionType := packet.GetInt8(10);

    case (actionType)
        ACTION_TYPE_POSTURE: return ExecutePosture(who, actionId);
        ACTION_TYPE_ABILITY: return ExecuteAbility(who, actionId);
        ACTION_TYPE_SPECIAL: return ExecuteCombatHotkey(who, actionId);
        ACTION_TYPE_SPELL:   return ExecuteSpell(who, actionId);
        ACTION_TYPE_COMMAND: return ExecuteCommand(who, actionId);
        ACTION_TYPE_SKILL:   return ExecuteSkill(who, actionId);
        default:
            SendSysMessage(who, "Erro: Tipo de ação desconhecido.");
            return false;
    endcase
    
    return true;
endfunction