use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";
include ":hotbar:custombuff";

/**
 * Reflexo Estrategico (Strategic Reflex) - ABILITY_ID::REFLEXO_ESTRATEGICO (1501)
 *
 * Active (ON_USE):
 * - Stamina regen over time (tactics/10 every 2s for tactics/5 seconds)
 *
 * Passive (permanent via HasAbility in ParryController:138-142):
 * - Uses TACTICS instead of PARRY for parry chance calculation
 */
class StrategicReflex(CombatAbility)

    function StrategicReflex(this)
        Super(ABILITY_ID::REFLEXO_ESTRATEGICO);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 5)
            duration := 5;
        endif

        TS_StartTimer(who, "recuperacaostamina", duration);
        SendAbilityBuff(who, ABILITY_ID::REFLEXO_ESTRATEGICO, duration, "Recuperacao de stamina");
        Message("*reflexos estrategicos*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[REFLEXO_ESTRATEGICO] Recuperacao stamina por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
