use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedScripts:timedScripts";

/**
 * Desmoralizar (Demoralize) - ABILITY_ID::DESMORALIZAR (1502)
 *
 * Active (ON_USE with target):
 * - Targets an enemy and applies "finta" timed script
 * - "finta" gives -20 hitchance_mod to the target
 * - Duration based on Tactics/5
 */
class Demoralize(CombatAbility)

    function Demoralize(this)
        Super(ABILITY_ID::DESMORALIZAR);
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        var targ;
        if (who.npctemplate)
            targ := who.opponent;
            if (!targ)
                targ := GetRandomNearbyHostile(who);
            endif
        else
            SendSysMessageEx(who, "Selecione o alvo para desmoralizar.", SSM_INFO);
            targ := Target(who);
        endif

        if (!targ)
            return false;
        endif

        if (!targ.isa(POLCLASS_MOBILE))
            Message("Alvo invalido.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        if (targ == who)
            Message("Voce nao pode desmoralizar a si mesmo.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 5)
            duration := 5;
        endif

        TS_StartTimer(targ, "finta", duration);
        Message("*desmoraliza o oponente*").SendAbove(who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[DESMORALIZAR] Finta aplicada em {targ.name} por {duration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
