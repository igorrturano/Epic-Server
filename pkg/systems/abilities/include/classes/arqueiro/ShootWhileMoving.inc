include ":abilities:models/CombatAbility";

/**
 * Tiro em Movimento (Shoot While Moving) - ABILITY_ID::TIRO_EM_MOVIMENTO (3)
 *
 * Purely passive ability - no combat hooks.
 * Effects are handled inline:
 *   - habilidades.inc: GrantPrivilege(who, "firewhilemoving") + who.enable("firewhilemoving")
 */
class ShootWhileMoving(CombatAbility)

	function ShootWhileMoving(this)
		Super(ABILITY_ID::TIRO_EM_MOVIMENTO);
	endfunction

endclass
