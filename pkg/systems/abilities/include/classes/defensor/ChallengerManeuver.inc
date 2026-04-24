use uo;

include ":abilities:models/CombatAbility";
include ":wrappers:Objects/Model/MobileWrapper";
include ":wrappers:Objects/Model/WeaponWrapper";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":timedScripts:timedScripts";
include ":tn:cooldown";
include ":attributes:attributes";

/**
 * Manobra Desafiadora (Challenger Maneuver) - ABILITY_ID::MANOBRA_DESAFIADORA (302)
 *
 * Active (OnParry): Triggers on successful parry while ability is activated.
 * - Offensive posture: Shield bash + paralysis (PARRY/10 seconds)
 * - Defensive posture: Disarm attacker (PARRY/10 seconds)
 *
 * Single-use: Consumes on first parry trigger.
 */
class ChallengerManeuver(CombatAbility)

    function ChallengerManeuver(this)
        Super(ABILITY_ID::MANOBRA_DESAFIADORA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_PARRY;
        this.SingleUse := true;
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

        if (postura == "Ofensiva")
            Message("Sua proxima defesa sera gloriosa.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        elseif (postura == "Defensiva")
            Message("Sua proxima defesa sera surpreendente.", Color(SYSTEM_COLOR::INFO)).SendSys(who);
        endif
    endfunction

    function OnParry(this, attackInfo)
        var defender := attackInfo.GetDefender();
        var who := defender.GetObjRef();
        var postura := who.GetProp("Postura");
        var parrySkill := CInt(defender.GetSkill(PARRY));

        if (postura == "Ofensiva")
            var stunDuration := CInt(parrySkill / 10);
            if (stunDuration < 1)
                stunDuration := 1;
            endif

            TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "paralysis", stunDuration);

            Message("*golpeia com o escudo!*").SendAbove(who);
            Message("*eh atordoado pelo golpe de escudo!*", Color(SYSTEM_COLOR::FAIL))
                .SendAbove(attackInfo.GetAttacker().GetObjRef());

            DebugMsg(who, DEBUG_HAB, Message($"[MANOBRA_DESAFIADORA] Shield bash! Stun {stunDuration}s", Color(SYSTEM_COLOR::SUCCESS)));

        elseif (postura == "Defensiva")
            var attackerWeapon := attackInfo.GetAttacker().GetEquipBySlot(WEAPON);

            if (!attackerWeapon || !attackerWeapon.GetObjRef())
                return false;
            endif

            if (attackerWeapon.GetCategory() == WEAPON_CATEGORY::UNARMED)
                return false;
            endif

            var disarmDuration := CInt(parrySkill / 10);
            if (disarmDuration < 1)
                disarmDuration := 1;
            endif

            var attackerPos := attackInfo.GetAttacker().GetPosition();
            var dropX := attackerPos.GetX() + RandomIntMinMax(-1, 1);
            var dropY := attackerPos.GetY() + RandomIntMinMax(-1, 1);
            var dropZ := GetWorldHeight(dropX, dropY);

            if (MoveObjectToLocation(
                attackerWeapon.GetObjRef(),
                dropX,
                dropY,
                dropZ,
                attackerPos.GetRealm(),
                MOVEOBJECT_FORCELOCATION
            ))
                TS_StartTimer(attackInfo.GetAttacker().GetObjRef(), "desarmar", disarmDuration, 0, who);

                Message("*desarma o oponente com uma manobra desafiadora!*").SendAbove(who);
                Message("*sua arma foi desarmada!*", Color(SYSTEM_COLOR::FAIL))
                    .SendAbove(attackInfo.GetAttacker().GetObjRef());

                DebugMsg(who, DEBUG_HAB, Message($"[MANOBRA_DESAFIADORA] Disarm {disarmDuration}s", Color(SYSTEM_COLOR::SUCCESS)));
            endif
        endif

        return true;
    endfunction

endclass