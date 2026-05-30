use uo;

include ":abilities:models/CombatAbility";

/**
 * Sigilo de Protecao (Protection Sigil) - ABILITY_ID::SIGILO_DE_PROTECAO (1900)
 *
 * Passive (via HasAbility in external files):
 * - +5 Will bonus (intrinsicMods.src)
 * - Bonus damage vs Demon NPCs (DamageController_legacy.inc)
 * - Demon wing variant in ChooseWings (epicUtils.inc)
 * - Class identification for Demon Hunter spell access
 */
class ProtectionSigil(CombatAbility)

    function ProtectionSigil(this)
        Super(ABILITY_ID::SIGILO_DE_PROTECAO);
    endfunction

endclass
