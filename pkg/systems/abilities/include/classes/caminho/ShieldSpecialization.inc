use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":tn:cooldown";

/**
 * Shield Specialization - ABILITY_ID::SHIELD_SPECIALIZATION (3300)
 *
 * Passive: Reduces parry cooldown by 20% when parrying with a shield.
 */
class ShieldSpecialization(CombatAbility)

	function ShieldSpecialization(this)
		Super(ABILITY_ID::SHIELD_SPECIALIZATION);
	endfunction

	function OnPassiveOnParry(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var who := defender.GetObjRef();

		if (!who.shield)
			return false;
		endif

		var currentCooldown := GetCooldown(who, "parry");
		if (currentCooldown > 0)
			var reduction := CInt(currentCooldown * 20 / 100);
			if (reduction < 1)
				reduction := 1;
			endif
			SetCooldown(who, "parry", currentCooldown - reduction);

			DebugMsg(who, DEBUG_HAB,
				Message($"[SHIELD_SPEC] Cooldown de aparar reduzido em {reduction}s", Color(SYSTEM_COLOR::SUCCESS)));
		endif

		return true;
	endfunction

endclass
