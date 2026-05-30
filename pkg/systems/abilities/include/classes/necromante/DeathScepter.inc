use uo;

include ":abilities:models/CombatAbility";

/**
 * Cetro da Morte (Death Scepter) - ABILITY_ID::CETRO_DA_MORTE (2002)
 *
 * Passive (via HasAbility in epicMageryUtils.inc):
 * - Makes Kaijin (necromancy) spells harder to resist (RESIST_EASY → RESIST_MEDIUM)
 */
class DeathScepter(CombatAbility)

    function DeathScepter(this)
        Super(ABILITY_ID::CETRO_DA_MORTE);
    endfunction

endclass
