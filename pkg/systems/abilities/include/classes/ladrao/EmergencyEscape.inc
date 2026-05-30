use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";

/**
 * Fuga de Emergencia (Emergency Escape) - ABILITY_ID::FUGA_DE_EMERGENCIA (1604)
 *
 * Active (ON_USE):
 * - Grants concealment for 30 seconds, allowing stealth running
 *
 * Passive (via HasAbility in stealth.src):
 * - Can hide while under combat threat
 */
class EmergencyEscape(CombatAbility)

    function EmergencyEscape(this)
        Super(ABILITY_ID::FUGA_DE_EMERGENCIA);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        TS_StartTimer(who, "conceal", 30);
        Message("*corre furtivamente*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message("[FUGA_DE_EMERGENCIA] Corrida furtiva ativada por 30s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
