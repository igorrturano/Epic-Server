use uo;

include ":abilities:models/CombatAbility";
include ":timedScripts:timedScripts";  // TS_StartTimer for stun
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Golpe Mortal (Murder Blow) - ABILITY_ID::GOLPE_MORTAL (201)
 *
 * Active (BeforeDamage): Full armor bypass + stun TT/10 seconds
 * Passive (OnKill): Attack nearby enemy
 *
 * Note: This is a single-charge ability - ends after one hit
 */
class MurderBlow(CombatAbility)

    function MurderBlow(this)
        Super(ABILITY_ID::GOLPE_MORTAL);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    // Window to land the hit: TT/10 seconds (min 2)
    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 10);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    // ACTIVE: Ignore armor (full bypass) + stun TT/10s
    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();
        var tactics := attacker.GetSkill(TACTICS);

        // Full armor bypass - SetIgnoreArmor is checked by CalculateDefenderResist
        // which runs AFTER this hook, skipping all armor reduction
        attackInfo.SetIgnoreArmor(true);

        // Stun duration: TT/10 seconds (min 2, max 10)
        var stunDuration := CInt(tactics / 10);
        if (stunDuration < 2)
            stunDuration := 2;
        endif
        if (stunDuration > 10)
            stunDuration := 10;
        endif

        TS_StartTimer(defender.GetObjRef(), "paralysis", stunDuration);

        Message("*acerta em cheio um ponto vulneravel!*")
            .SendAbove(attacker.GetObjRef());

        Message("*e atordoado pelo golpe mortal!*", Color(SYSTEM_COLOR::FAIL))
            .SendAbove(defender.GetObjRef());

        DebugMsg(
            attacker.GetObjRef(),
            DEBUG_HAB,
            Message($"[GOLPE_MORTAL] Ignorou armadura, stun: {stunDuration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

    // PASSIVE: On kill, attack nearby enemy
    function OnPassiveOnKill(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();

        // Find nearby hostile targets within 2 tiles with line of sight
        var nearbyHostiles := ListHostiles(attacker.GetObjRef(), 2, LH_FLAG_LOS);
        var validTargets := array{};

        foreach mobile in (nearbyHostiles)
            // Don't attack self or the just-killed defender
            if ((mobile.serial != attacker.GetSerial()) &&
                (mobile.serial != defender.GetSerial()) &&
                (GetVital(mobile, HITS) > 0))
                validTargets.append(mobile);
            endif
        endforeach

        if (validTargets.size() > 0)
            // Pick random target
            var nextTarget := validTargets[RandomInt(validTargets.size()) + 1];
            nextTarget := MobileWrapper(nextTarget);

            Message("*executa o oponente e ataca outro inimigo!*")
                .SendAbove(attacker.GetObjRef());

            attacker.Attack(nextTarget);

            DebugMsg(
                attacker.GetObjRef(),
                DEBUG_HAB,
                Message("[GOLPE_MORTAL] Passivo: Atacou inimigo proximo!", Color(SYSTEM_COLOR::SUCCESS))
            );

            return true;
        endif

        return false;
    endfunction

endclass
