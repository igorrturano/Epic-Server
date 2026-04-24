include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Precisao Cirurgica (Surgical Precision) - ABILITY_ID::PRECISAO_CIRURGICA (1403)
 *
 * Passive (BeforeDamage): Adds SCIENCE/20 bonus damage to all physical types.
 *
 * Migrated from DamageController_legacy.inc:471-475.
 */
class SurgicalPrecision(CombatAbility)

	function SurgicalPrecision(this)
		Super(ABILITY_ID::PRECISAO_CIRURGICA);
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();

		var bonus := CInt(attacker.GetSkill(SCIENCE) / 20);
		if (bonus <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(bonus)
			.IncreasePiercingRawDamage(bonus)
			.IncreaseBluntRawDamage(bonus);

		DebugMsg(
			attacker.GetObjRef(),
			DEBUG_HAB,
			Message($"[PRECISAO_CIRURGICA] +{bonus} dano", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
