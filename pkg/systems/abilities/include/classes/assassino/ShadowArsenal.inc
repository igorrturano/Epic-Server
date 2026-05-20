include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Arsenal Sombrio (Shadow Arsenal) - ABILITY_ID::ARSENAL_SOMBRIO (1202)
 *
 * Passive (BeforeDamage): When wielding an assassin weapon, adds SNEAK/20
 * bonus damage to all physical damage types.
 *
 * Migrated from DamageController_legacy.inc:477-483.
 */
class ShadowArsenal(CombatAbility)

	function ShadowArsenal(this)
		Super(ABILITY_ID::ARSENAL_SOMBRIO);
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var weapon := attacker.GetEquipBySlot(WEAPON);

		if (!weapon || !weapon.IsAssassin())
			return false;
		endif

		var bonus := CInt(attacker.GetSkill(SNEAK) / 20);
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
			Message($"[ARSENAL_SOMBRIO] +{bonus} dano", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
