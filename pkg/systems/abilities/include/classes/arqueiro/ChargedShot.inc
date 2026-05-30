use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Tiro Carregado (Charged Shot) - ABILITY_ID::TIRO_CARREGADO (2)
 *
 * BeforeDamage: Adds DEX-based bonus to piercing damage for TT/5 seconds.
 * Requires ranged weapon.
 */
class ChargedShot(CombatAbility)

	function ChargedShot(this)
		Super(ABILITY_ID::TIRO_CARREGADO);
		this.ActivationMoment := ABILITY_ACTIVATION::BEFORE_DAMAGE;
	endfunction

	function CanActivate(this, mobile)
		var who := mobile.GetObjRef();
		var weapon := mobile.GetEquipBySlot(WEAPON);

		if (!weapon || weapon.GetAttribute() != RANGED)
			Message("Voce precisa estar usando uma arma de longo alcance!", Color(SYSTEM_COLOR::FAIL)).SendSys(who);
			return false;
		endif

		return true;
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

		// Re-check ranged weapon (player may have switched during duration)
		var weapon := attacker.GetEquipBySlot(WEAPON);
		if (!weapon || weapon.GetAttribute() != RANGED)
			return false;
		endif

		var ranged := CInt(attacker.GetSkill(RANGED));
		var bonus := CInt(ranged / 2);

		if (bonus > 0)
			attackInfo.GetDamage().IncreasePiercingRawDamage(bonus);

			DebugMsg(
				attacker.GetObjRef(),
				DEBUG_HAB,
				Message($"[TIRO_CARREGADO] Bonus de dano por RANGED: +{bonus}", Color(SYSTEM_COLOR::SUCCESS))
			);
		endif

		return true;
	endfunction

endclass
