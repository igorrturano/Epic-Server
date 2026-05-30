use uo;
use os;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Golpes Furiosos (Rage Cut) - ABILITY_ID::GOLPES_FURIOSOS (203)
 *
 * OnUse: Immediately perform TT/40 extra attacks (min 1, max 5) on current opponent
 */
class RageCut(CombatAbility)

    function RageCut(this)
        Super(ABILITY_ID::GOLPES_FURIOSOS);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

    // Check if player has an opponent to attack
    function CanActivate(this, mobile)
        var who := mobile.GetObjRef();
        if (!who.opponent)
            Message("Voce precisa estar em combate.", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
            return false;
        endif
        return true;
    endfunction

    // OnUse: Perform multiple attacks
    function ExecuteOn(this, mobile)
        var who := mobile.GetObjRef();

        // Calculate number of attacks: TT/40 (min 1, max 5)
        var extraAttacks := CInt(mobile.GetSkill(TACTICS) / 40);
        if (extraAttacks < 1)
            extraAttacks := 1;
        endif
        if (extraAttacks > 5)
            extraAttacks := 5;
        endif

        Message($"*desfere {extraAttacks} golpes furiosos!*").SendAbove(who);

        for i := 1 to extraAttacks
            who.attack_once();
            sleepms(500);
        endfor

        DebugMsg(
            who,
            DEBUG_HAB,
            Message($"[GOLPES_FURIOSOS] Executou {extraAttacks} ataques extras!", Color(SYSTEM_COLOR::SUCCESS))
        );

        return true;
    endfunction

endclass
