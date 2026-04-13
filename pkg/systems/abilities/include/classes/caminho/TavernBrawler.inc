use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";
include ":attributes:attributes";

/**
 * Tavern Brawler - ABILITY_ID::TAVERN_BRAWLER (3105)
 *
 * Passive: When fighting unarmed (wrestling):
 * - Attacker: +4 damage
 */
class TavernBrawler(CombatAbility)

	function TavernBrawler(this)
		Super(ABILITY_ID::TAVERN_BRAWLER);
	endfunction

	function _MatchesContext(this, mobile)
		var weapon := mobile.GetEquipBySlot(WEAPON);
		if (!weapon || weapon is @Exception)
			return true; // No weapon = unarmed
		endif
		return Upper(weapon.GetAttribute()) == Upper(WRESTLING);
	endfunction

	// Attacker: +4 damage
	function OnPassiveBeforeDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		if (!this._MatchesContext(attacker))
			return false;
		endif

		attackInfo.GetDamage()
			.IncreaseSlashRawDamage(4)
			.IncreasePiercingRawDamage(4)
			.IncreaseBluntRawDamage(4);

		DebugMsg(attacker.GetObjRef(), DEBUG_HAB,
			Message("[TAVERN_BRAWLER] +4 dano", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
