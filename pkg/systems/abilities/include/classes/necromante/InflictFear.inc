use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":timedscripts:timedScripts";
include ":wrappers:Objects/Model/MobileWrapper";

/**
 * Inflingir Medo (Inflict Fear) - ABILITY_ID::INFLINGIR_MEDO (2001)
 *
 * Active (ON_USE):
 * - Target enemy (LOS required)
 * - Aura Divina provides immunity
 * - Entropy <= -8000 provides immunity
 * - Applies "temor" timed script (duration = ARCANA/1.5)
 */
class InflictFear(CombatAbility)

    function InflictFear(this)
        Super(ABILITY_ID::INFLINGIR_MEDO);
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
            Message("Escolha o alvo do toque sombrio", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            targ := Target(who, TGTOPT_CHECK_LOS + TGTOPT_NEUTRAL);
        endif

        if (!targ)
            return false;
        endif

        if (!CheckLOSAt(who, targ.x, targ.y, targ.z))
            Message("O Alvo nao esta visivel para voce.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif

        if (MobileWrapper(targ).HasAbility(ABILITY_ID::AURA_DIVINA))
            Message("O alvo resistiu a magia.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            Message("Voce nao conhece o medo.", Color(SYSTEM_COLOR::INFO)).SendSys(targ);
            return false;
        endif

        var entropy := MobileWrapper(targ).GetEntropy();
        if (entropy <= -8000)
            Message("Voce nao conhece o medo.").SendSys(targ);
            Message("O alvo resistiu a magia.").SendSys(who);
            return false;
        endif

        var lvl := CInt(mobile.GetSkill(ARCANA) / 1.5);
        TS_StartTimer(targ, "temor", lvl, who);

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[INFLINGIR_MEDO] Duracao: {lvl}s", Color(SYSTEM_COLOR::SUCCESS))
        );
        return true;
    endfunction

endclass
