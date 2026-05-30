use uo;

include ":abilities:models/CombatAbility";

/**
 * Elemental Specialization (Pool Ability)
 * Each purchase grants +2 spell damage with the matching element.
 * Effect handled in epicMageryUtils.inc (bonusElementalSpecDamage).
 * These classes exist for registry + pool tracking purposes.
 */
class ElementalSpecBase(CombatAbility)

	function ElementalSpecBase(this, abilityEnum)
		Super(abilityEnum);
		this.IsPool := true;
	endfunction

endclass

class ElementalSpecFire(ElementalSpecBase)
	function ElementalSpecFire(this)
		Super(ABILITY_ID::ELEMENTAL_SPEC_FIRE);
	endfunction
endclass

class ElementalSpecCold(ElementalSpecBase)
	function ElementalSpecCold(this)
		Super(ABILITY_ID::ELEMENTAL_SPEC_COLD);
	endfunction
endclass

class ElementalSpecEnergy(ElementalSpecBase)
	function ElementalSpecEnergy(this)
		Super(ABILITY_ID::ELEMENTAL_SPEC_ENERGY);
	endfunction
endclass

class ElementalSpecPoison(ElementalSpecBase)
	function ElementalSpecPoison(this)
		Super(ABILITY_ID::ELEMENTAL_SPEC_POISON);
	endfunction
endclass
