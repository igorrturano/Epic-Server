use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";
include ":hotbar:custombuff";

/**
 * Manobra da Invencibilidade (Invincibility Maneuver) - ABILITY_ID::MANOBRA_DA_INVENCIBILIDADE (304)
 *
 * Active (BeforeHit - as DEFENDER):
 * - Offensive posture: Counter-attack every incoming hit for PARRY/3 seconds
 * - Defensive posture: +25 evasion, cancels on movement (walk hook sends ABORT)
 *
 * Lifecycle:
 *   OnActivate: applies evasion bonus (defensive) or message (offensive)
 *   OnBeforeHit: sets counter-attack flag (offensive only)
 *   OnDeactivate: removes evasion bonus (defensive)
 */
class InvincibilityManeuver(CombatAbility)

    function InvincibilityManeuver(this)
        Super(ABILITY_ID::MANOBRA_DA_INVENCIBILIDADE);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(PARRY) / 3);
        if (duration < 1)
            duration := 1;
        endif
        return duration;
    endfunction

    function CanActivate(this, mobile)
        var postura := mobile.GetObjRef().GetProp("Postura");
        if (!postura || (postura != "Ofensiva" && postura != "Defensiva"))
            Message("Defina sua postura primeiro (.postura ofensiva ou .postura defensiva).", Color(SYSTEM_COLOR::FAIL)).SendSys(mobile.GetObjRef());
            return false;
        endif
        return true;
    endfunction

    function OnActivate(this, mobile)
        var who := mobile.GetObjRef();
        var postura := who.GetProp("Postura");

        var duration := this.GetDurationFor(mobile);

        if (postura == "Ofensiva")
            Message("Voce se prepara para contra-atacar.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            SendAbilityBuff(who, ABILITY_ID::MANOBRA_DA_INVENCIBILIDADE, duration, "Contra-ataque a cada golpe");
        elseif (postura == "Defensiva")
            who.evasionchance_mod := who.evasionchance_mod + 25;
            mobile.SetProperty("#invencibilidade_evasion", 1);
            Message("*defesa total*").SendAbove(who);
            Message("Voce se concentra em uma defesa total.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
            SendAbilityBuff(who, ABILITY_ID::MANOBRA_DA_INVENCIBILIDADE, duration, "+25 evasao");

            // Walk packet hook sends ABORT when player moves
            var monitor := mobile.GetAbilityMonitor(this);
            mobile.SetProperty("#cancel_on_move_pid", monitor.GetPid());
        endif
    endfunction

    function OnDeactivate(this, mobile)
        if (mobile.GetProperty("#invencibilidade_evasion"))
            mobile.GetObjRef().evasionchance_mod := mobile.GetObjRef().evasionchance_mod - 25;
            mobile.GetObjRef().EraseProp("#invencibilidade_evasion");
            mobile.GetObjRef().EraseProp("#cancel_on_move_pid");
        endif
    endfunction

    function OnBeforeHit(this, attackInfo)
        var who := attackInfo.GetDefender().GetObjRef();
        var postura := who.GetProp("Postura");

        if (postura == "Ofensiva")
            attackInfo.SetCounterAttack(true);

            DebugMsg(
                who,
                DEBUG_HAB,
                Message("[INVENCIBILIDADE] Contra-ataque!", Color(SYSTEM_COLOR::SUCCESS))
            );
        endif

        return true;
    endfunction

endclass
