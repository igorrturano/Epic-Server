use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Sword and Board Style - ABILITY_ID::SWORD_AND_BOARD (3101)
 *
 * Passive: When using a one-handed weapon WITH a shield:
 * - Attacker: +2 damage
 * - Defender: -2 enemy hit chance (evasion)
 */
class SwordAndBoard(CombatAbility)

	function SwordAndBoard(this)
		Super(ABILITY_ID::SWORD_AND_BOARD);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return Upper(weapon.GetAttribute()) == Upper(ONEHANDED) && mobile.GetObjRef().shield;
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
			Message("[SWORD_AND_BOARD] +2 dano", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction


	// Defender: -2 evasion
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		if (!this._MatchesContext(defender))
			return false;
		endif
		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) - 2);
		DebugMsg(defender.GetObjRef(), DEBUG_HAB,
			Message("[SWORD_AND_BOARD] -2 acerto inimigo (evasao)", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
