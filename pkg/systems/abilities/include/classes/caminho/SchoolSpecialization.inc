use uo;

include ":abilities:models/CombatAbility";

/**
 * School Specialization (NOT pool - one purchase per school)
 * Reduces cast difficulty by 10 for the matching spell school.
 * Effect handled in cast_spell.src (checkCastingSuccess).
 * These classes exist for registry purposes.
 */
class SchoolSpecBase(CombatAbility)

	function SchoolSpecBase(this, abilityEnum)
		Super(abilityEnum);
	endfunction

endclass

class SchoolSpecEvocatium(SchoolSpecBase)
	function SchoolSpecEvocatium(this)
		Super(ABILITY_ID::SCHOOL_SPEC_EVOCATIUM);
	endfunction
endclass

class SchoolSpecVelkyn(SchoolSpecBase)
	function SchoolSpecVelkyn(this)
		Super(ABILITY_ID::SCHOOL_SPEC_VELKYN);
	endfunction
endclass

class SchoolSpecAhtaleloreth(SchoolSpecBase)
	function SchoolSpecAhtaleloreth(this)
		Super(ABILITY_ID::SCHOOL_SPEC_AHTALELORETH);
	endfunction
endclass

class SchoolSpecKaijin(SchoolSpecBase)
	function SchoolSpecKaijin(this)
		Super(ABILITY_ID::SCHOOL_SPEC_KAIJIN);
	endfunction
endclass
