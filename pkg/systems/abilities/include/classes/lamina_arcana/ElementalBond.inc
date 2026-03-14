include ":abilities:models/CombatAbility";

/**
 * Vinculo Elemental (Elemental Bond) - ABILITY_ID::VINCULO_ELEMENTAL (2300)
 *
 * Passive ability (no active ExecuteOn):
 * - Allows casting spells while wielding weapons [cast_spell.src]
 * - Prevents disarm [desarmar.src, ghaia/desarmar.src, timedScripts/desarmar.src]
 * - Bypasses movement penalties for spellcasting [cast_spell.src]
 * - Elemental weapon affinity check [epicMageryUtils.inc]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::VINCULO_ELEMENTAL)
 */
class ElementalBond(CombatAbility)

    function ElementalBond(this)
        Super(ABILITY_ID::VINCULO_ELEMENTAL);
    endfunction

endclass
