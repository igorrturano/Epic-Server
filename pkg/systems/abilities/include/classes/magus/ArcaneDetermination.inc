include ":abilities:models/CombatAbility";

/**
 * Determinacao Arcana (Arcane Determination) - ABILITY_ID::DETERMINACAO_ARCANA (2200)
 *
 * Passive ability (no active ExecuteOn):
 * - Allows spending stamina when mana is insufficient for spellcasting [cast_spell.src]
 * - Bypasses reagent requirements [cast_spell.src]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::DETERMINACAO_ARCANA)
 */
class ArcaneDetermination(CombatAbility)

    function ArcaneDetermination(this)
        Super(ABILITY_ID::DETERMINACAO_ARCANA);
    endfunction

endclass
