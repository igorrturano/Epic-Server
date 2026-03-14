use uo;

include ":abilities:models/CombatAbility";
include ":timedScripts:timedScripts";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Valentao (Bully) - ABILITY_ID::VALENTAO (1602)
 *
 * Active (BEFORE_DAMAGE, SingleUse):
 * - Stuns the defender on next hit for Pickpocketing/10 seconds
 *
 * Passive (via HasAbility in pickpocketing.src):
 * - Crowd bonus to stealing (+5 per person over 4 nearby)
 */
class Bully(CombatAbility)

    function Bully(this)
        Super(ABILITY_ID::VALENTAO);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
        this.SingleUse := true;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();
        var defender := attackInfo.GetDefender();
        var pickpocket := attacker.GetSkill(PICKPOCKETING);

        var stunDuration := CInt(pickpocket / 10);
        if (stunDuration < 2)
            stunDuration := 2;
        endif
        if (stunDuration > 10)
            stunDuration := 10;
        endif

        TS_StartTimer(defender.GetObjRef(), "paralysis", stunDuration);

        Message("*golpe atordoante*").SendAbove(attacker.GetObjRef());
        Message("*atordoado*", Color(SYSTEM_COLOR::FAIL)).SendAbove(defender.GetObjRef());

        DebugMsg(
            attacker.GetObjRef(),
            DEBUG_HAB,
            Message($"[VALENTAO] Stun: {stunDuration}s", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
