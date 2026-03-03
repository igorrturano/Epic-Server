include ":abilities:models/CombatAbility";

/**
 * Evocator - ABILITY_ID::EVOCATOR (2202)
 *
 * Passive ability (no active ExecuteOn):
 * - Boosts Evocatium school spell range [epicMageryUtils.inc - distanceAllowedForSpells]
 * - +20% spell damage for Evocatium [epicMageryUtils.inc - bonusSpellDamageForSpecialists]
 * - +20% spell duration for Evocatium [epicMageryUtils.inc - bonusSpellDurationForSpecialists]
 * - Increases spell resist difficulty for Evocatium [epicMageryUtils.inc - handleSpellResistDiffForSpecialists]
 * - +1 magic missiles [magicmissile.src]
 * - +20% wall of fire/stone area and fuel [walloffire.src, wallofstone.src]
 * - +20% light globe level [lightglobe.src]
 * - +20% haste duration [haste.src]
 *
 * All effects are checked inline via HasAbility(ABILITY_ID::EVOCATOR)
 */
class Evocator(CombatAbility)

    function Evocator(this)
        Super(ABILITY_ID::EVOCATOR);
    endfunction

endclass
