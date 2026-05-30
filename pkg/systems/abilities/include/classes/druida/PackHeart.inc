include ":abilities:models/CombatAbility";

/**
 * Coracao da Matilha (Pack Heart) - ABILITY_ID::CORACAO_DA_MATILHA (2103)
 *
 * Passive ability (no active ExecuteOn):
 * - Doubles max wildshape forms (Magery/20 -> Magery/10) [NaturalBond._StoreTransformation]
 * - Doubles polymorph duration [assumirforma.src, polymorph.src]
 * - Boosts polymorph stats with Magery bonus [assumirforma.src, polymorph.src, formaprimordial.src]
 * - Extends enchant person duration [encantarpessoa.src]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::CORACAO_DA_MATILHA)
 */
class PackHeart(CombatAbility)

    function PackHeart(this)
        Super(ABILITY_ID::CORACAO_DA_MATILHA);
    endfunction

endclass
