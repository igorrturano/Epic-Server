use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";

/**
 * Reflexo Estrategico (Strategic Reflex) - ABILITY_ID::REFLEXO_ESTRATEGICO (1501)
 *
 * Active (ON_USE):
 * - DEX bonus proportional to missing stamina
 * - Higher bonus when more exhausted
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
        var currentStam := AP_GetVital(who, STAMINA);
        var maxStam := AP_GetVitalMaximumValue(who, STAMINA);

        if (maxStam <= 0)
            return false;
        endif

        var missingRatio := CDbl(maxStam - currentStam) / CDbl(maxStam);
        var bonus := CInt(missingRatio * (mobile.GetSkill(TACTICS) / 5));

        if (bonus <= 0)
            Message("Voce nao esta cansado o suficiente.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        var duration := CInt(mobile.GetSkill(TACTICS) / 3);
        if (duration < 5)
            duration := 5;
        endif

        TS_StartTimer(who, "agility", duration, bonus);
        Message($"*reflexos estrategicos* (+{bonus} DEX)").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[REFLEXO_ESTRATEGICO] +{bonus} DEX por {duration}s (stamina {CInt(missingRatio * 100)}% faltando)", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
