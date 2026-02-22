use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Encontrar Brecha (Find Opening) - ABILITY_ID::ENCONTRAR_BRECHA (4)
 *
 * BeforeDamage: All attacks ignore armor for TT/5 seconds.
 * Works with any weapon type (no weapon restriction).
 */
class FindOpening(CombatAbility)

	function FindOpening(this)
		Super(ABILITY_ID::ENCONTRAR_BRECHA);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function GetDurationFor(this, mobile)
		var duration := CInt(mobile.GetSkill(TACTICS) / 5);
		if (duration < 1)
			duration := 1;
		endif
		return duration;
	endfunction

	function OnBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		attackInfo.SetIgnoreArmor();

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message("[ENCONTRAR_BRECHA] Ativo - ignorando armadura!", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
