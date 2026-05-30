use uo;

include ":abilities:models/CombatAbility";

/**
 * Mancha da Corrupcao (Corruption Mark) - ABILITY_ID::MANCHA_DA_CORRUPCAO (1902)
 *
 * Passive (via HasAbility in HitController.inc:276-282):
 * - Attacker: +THEOLOGY/10 hit chance bonus
 * - Defender: -THEOLOGY/10 hit evasion
 */
class CorruptionMark(CombatAbility)

    function CorruptionMark(this)
        Super(ABILITY_ID::MANCHA_DA_CORRUPCAO);
    endfunction

endclass
