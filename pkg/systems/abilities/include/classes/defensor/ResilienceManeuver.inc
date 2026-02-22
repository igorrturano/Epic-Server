use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Manobra da Resiliencia (Resilience Maneuver) - ABILITY_ID::MANOBRA_DA_RESILIENCIA (303)
 *
 * OnUse (posture-based):
 * - Offensive: Immune to crits/stuns for PARRY/3 seconds ("vitalidadeinabalavel")
 * - Defensive: HP regen over time ("recuperacaoaligera" timed script, 60s)
 *
 * Inline effects:
 *   - DamageController: checks "vitalidadeinabalavel" to prevent critical hits
 *   - paralysis.src/congelamento.src: check for stun/freeze immunity
 */
class ResilienceManeuver(CombatAbility)

    function ResilienceManeuver(this)
        Super(ABILITY_ID::MANOBRA_DA_RESILIENCIA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    function CanActivate(this, mobile)
        var postura := mobile.GetObjRef().GetProp("Postura");
        if (!postura || (postura != "Ofensiva" && postura != "Defensiva"))
            Message("Defina sua postura primeiro (.postura ofensiva ou .postura defensiva).", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
            return false;
        endif
        return true;
    endfunction

    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();
        var postura := who.GetProp("Postura");
        var parrySkill := CInt(mobile.GetSkill(PARRY));

        if (postura == "Ofensiva")
            var duration := CInt(parrySkill / 3);
            if (duration < 1)
                duration := 1;
            endif

            SetCooldown(who, "vitalidadeinabalavel", duration);
            Message("Seu jogo de defesas eh impecavel.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            DebugMsg(who, DEBUG_HAB, Message($"[MANOBRA_DA_RESILIENCIA] Imune a crits/stuns {duration}s", Color(SYSTEM_COLOR::SUCCESS)));

        elseif (postura == "Defensiva")
            TS_StartTimer(who, "recuperacaoaligera", 60);
            Message("Voce se motiva a continuar o combate.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            DebugMsg(who, DEBUG_HAB, Message("[MANOBRA_DA_RESILIENCIA] Recuperacao ativada", Color(SYSTEM_COLOR::SUCCESS)));
        endif

        return true;
    endfunction

endclass