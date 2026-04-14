use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Ranged Style - ABILITY_ID::RANGED_STYLE (3104)
 *
 * Passive: When using ranged weapons:
 * - Attacker: +2 hit chance and +2 damage
 */
class RangedStyle(CombatAbility)

	function RangedStyle(this)
		Super(ABILITY_ID::RANGED_STYLE);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return Upper(weapon.GetAttribute()) == Upper(RANGED);
	endfunction

	// Attacker: +2 hit chance
	function OnPassiveBeforeHitAttacker(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif
		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) + 2);
		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[RANGED_STYLE] +2 acerto", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

	// Attacker: +2 damage
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(2)
			.IncreasePiercingRawDamage(2)
			.IncreaseBluntRawDamage(2);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[RANGED_STYLE] +2 dano", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
