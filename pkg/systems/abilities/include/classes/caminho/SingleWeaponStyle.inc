use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Single Weapon Style - ABILITY_ID::SINGLE_WEAPON_STYLE (3100)
 *
 * Passive: When using a one-handed weapon WITHOUT a shield:
 * - Attacker: +3 damage
 * - Defender: -2 enemy hit chance (evasion)
 */
class SingleWeaponStyle(CombatAbility)

	function SingleWeaponStyle(this)
		Super(ABILITY_ID::SINGLE_WEAPON_STYLE);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return false;
		endif
		return Upper(weapon.GetAttribute()) == Upper(ONEHANDED) && !mobile.GetObjRef().shield;
	endfunction

	// Defender: -2 evasion
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		if (!this._MatchesContext(defender))
			return false;
		endif
		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) - 2);
		DebugMsg(defender.GetObjRef(), DEBUG_HAB,
			Message("[SINGLE_WEAPON] -2 acerto inimigo (evasao)", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

	// Attacker: +3 damage
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(3)
			.IncreasePiercingRawDamage(3)
			.IncreaseBluntRawDamage(3);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[SINGLE_WEAPON] +3 dano", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction


endclass
