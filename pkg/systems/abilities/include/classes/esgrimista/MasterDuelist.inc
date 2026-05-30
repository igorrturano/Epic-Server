use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":hotbar:custombuff";

/**
 * Mestre Duelista (Master Duelist) - ABILITY_ID::MESTRE_DUELISTA (1504)
 *
 * Active (ON_MISS - as ATTACKER):
 * - Re-attacks whenever you miss for Tactics/5 seconds
 * - Uses attack_once to re-roll the attack
 *
 * Requires ExecuteCombatAbilitiesOnMissAttacker() in AbilitiesController
 */
class MasterDuelist(CombatAbility)

    function MasterDuelist(this)
        Super(ABILITY_ID::MESTRE_DUELISTA);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_MISS;
    endfunction

    function GetDurationFor(this, mobile)
        var duration := CInt(mobile.GetSkill(TACTICS) / 5);
        if (duration < 2)
            duration := 2;
        endif
        return duration;
    endfunction

    function OnActivate(this, mobile)
        var who := mobile.GetObjRef();
        var duration := this.GetDurationFor(mobile);
        Message("*trabalho de pes*").SendAbove(who);
        SendAbilityBuff(who, ABILITY_ID::MESTRE_DUELISTA, duration, "Re-ataque ao errar");
    endfunction

    function OnMiss(this, attackInfo)
        var attacker := attackInfo.GetAttacker().GetObjRef();
        var defender := attackInfo.GetDefender().GetObjRef();

        attacker.attack_once(defender);
        Message("*re-ataca!*").SendAbove(attacker);

        DebugMsg(
            attacker,
            DEBUG_HAB,
            Message("[MESTRE_DUELISTA] Re-ataque apos erro!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
