use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Two-Handed Style - ABILITY_ID::TWO_HANDED_STYLE (3102)
 *
 * Passive: When using a two-handed weapon:
 * - Attacker: +3 damage and +5 critical hit chance
 */
class TwoHandedStyle(CombatAbility)

	function TwoHandedStyle(this)
		Super(ABILITY_ID::TWO_HANDED_STYLE);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return Upper(weapon.GetAttribute()) == Upper(TWOHANDED);
	endfunction

	// Attacker: +3 damage, +5 critical
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(3)
			.IncreasePiercingRawDamage(3)
			.IncreaseBluntRawDamage(3);
		attacker.SetCriticalChanceMod(CInt(attacker.GetCriticalChanceMod()) + 5);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[TWO_HANDED] +3 dano, +5 critico", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
