use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Danca das Sombras (Shadow Dance) - ABILITY_ID::DANCA_DAS_SOMBRAS (1800)
 *
 * Passive (via HasAbility in HitController.inc):
 * - Adds ARCANA/5 hit chance bonus
 */
class ShadowDance(CombatAbility)

    function ShadowDance(this)
        Super(ABILITY_ID::DANCA_DAS_SOMBRAS);
    endfunction

endclass
