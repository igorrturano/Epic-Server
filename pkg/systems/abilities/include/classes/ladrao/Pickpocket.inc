use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";

/**
 * Batedor de Carteira (Pickpocket) - ABILITY_ID::BATEDOR_DE_CARTEIRA (1600)
 *
 * Active (ON_USE):
 * - Grants speed boost for 30 seconds via "velocidade" timed script
 *
 * Passive (via HasAbility in pickpocketing.src):
 * - Never fails stealing coins, jewels, and keys
 * - Can steal from the front (not just behind)
 */
class Pickpocket(CombatAbility)

    function Pickpocket(this)
        Super(ABILITY_ID::BATEDOR_DE_CARTEIRA);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        TS_StartTimer(who, "velocidade", 30);
        Message("*velocidade sobrehumana*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message("[BATEDOR_DE_CARTEIRA] Velocidade ativada por 30s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
