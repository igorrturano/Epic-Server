use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Finta Esgrimista (Swashbuckler Feint) - ABILITY_ID::FINTA_ESGRIMISTA (1503)
 *
 * Active (BEFORE_HIT - as DEFENDER):
 * - Every incoming attack triggers a counter-attack
 * - Duration: Tactics/5 seconds
 * - Similar to CrossCut but without guaranteed first parry
 */
class SwashbucklerFeint(CombatAbility)

    function SwashbucklerFeint(this)
        Super(ABILITY_ID::FINTA_ESGRIMISTA);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_HIT;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 1)
            duration := 1;
        endif
        return duration;
    endfunction

    function OnActivate(this, mobile)
        Message("*postura de contra-ataque*").SendAbove(mobile.GetObjRef());
    endfunction

    function OnBeforeHit(this, attackInfo)
        var defender := attackInfo.GetDefender();

        attackInfo.SetCounterAttack(true);

        DebugMsg(
            defender.GetObjRef(),
            DEBUG_HAB,
            Message("[FINTA_ESGRIMISTA] Contra-ataque ativado!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
