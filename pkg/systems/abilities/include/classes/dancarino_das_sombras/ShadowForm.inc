use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Forma Umbral (Shadow Form) - ABILITY_ID::FORMA_UMBRAL (1804)
 *
 * Passive (via HasAbility in combatHook.src and shadowsteps.src):
 * - Can attack while concealed (combatHook.src doesn't strip concealment)
 * - Can stay in shadows while in warmode (shadowsteps.src protected())
 */
class ShadowForm(CombatAbility)

    function ShadowForm(this)
        Super(ABILITY_ID::FORMA_UMBRAL);
    endfunction

endclass
