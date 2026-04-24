include ":abilities:models/CombatAbility";

/**
 * Chamado da Forma Primordial (Primordial Form Call) - ABILITY_ID::CHAMADO_DA_FORMA_PRIMORDIAL (2104)
 *
 * Active (ON_USE):
 * - Transform into elemental forms (fire/cold/energy/poison)
 * - Logic is in the player command: .formaprimordial (formaprimordial.src)
 * - This class exists for V3 registration and HasAbility checks
 *
 * The command formaprimordial.src checks HasAbility(ABILITY_ID::CHAMADO_DA_FORMA_PRIMORDIAL)
 * before allowing the transformation.
 */
class PrimordialFormCall(CombatAbility)

    function PrimordialFormCall(this)
        Super(ABILITY_ID::CHAMADO_DA_FORMA_PRIMORDIAL);
        this.ActivationMoment := ABILITY_ACTIVATION::ON_USE;
    endfunction

endclass
