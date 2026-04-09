use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Sentenca de Morte (Death Sentence) - ABILITY_ID::SENTENCA_DE_MORTE (204)
 *
 * Active (BeforeDamage): All successful hits are guaranteed criticals for TT/5 seconds
 */
class DeathSentence(CombatAbility)

    function DeathSentence(this)
        Super(ABILITY_ID::SENTENCA_DE_MORTE);
        this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
    endfunction

    function GetDurationFor(this, mobile)
        // Duration = Tactics / 5
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 1)
            duration := 1;
        endif
        return duration;
    endfunction

    // ACTIVE: Force critical hit on all attacks
    function OnBeforeDamage(this, attackInfo)
        var attacker := attackInfo.GetAttacker();

        // Force critical hit
        attackInfo.GetDamage().SetCritical(true);

        DebugMsg(
            attacker.GetObjRef(),
            DEBUG_HAB,
            Message("[SENTENCA_DE_MORTE] Critico garantido!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
