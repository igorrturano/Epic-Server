use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Dual Wield Style - ABILITY_ID::DUAL_WIELD_STYLE (3103)
 *
 * Passive: When dual wielding:
 * - Attacker: +3 hit chance
 * - Defender: -2 enemy hit chance (evasion)
 */
class DualWieldStyle(CombatAbility)

	function DualWieldStyle(this)
		Super(ABILITY_ID::DUAL_WIELD_STYLE);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return Upper(weapon.GetAttribute()) == Upper(DUALWIELD);
	endfunction

	// Attacker: +3 hit chance
	function OnPassiveBeforeHitAttacker(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif
		attacker.SetProperty("#hit_mod", CInt(attacker.GetProperty("#hit_mod")) + 3);
		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[DUAL_WIELD] +3 acerto", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

	// Defender: -2 evasion
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		if (!this._MatchesContext(defender))
			return false;
		endif
		defender.SetProperty("#evasion_mod", CInt(defender.GetProperty("#evasion_mod")) + 2);
		DebugMsg(defender.GetObjRef(), DEBUG_HAB,
			Message("[DUAL_WIELD] -2 acerto inimigo (evasao)", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
