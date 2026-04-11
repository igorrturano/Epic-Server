use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Combate Leve (Unarmored Combat) - ABILITY_ID::UNARMORED_COMBAT (3628)
 *
 * Passive (DefenderBeforeDamage): Reduces incoming physical damage by Skill/20,
 * where Skill = max(Wrestling, Tactics).
 * Only works when wearing no armor.
 */
class UnarmoredCombat(CombatAbility)

	function UnarmoredCombat(this)
		Super(ABILITY_ID::UNARMORED_COMBAT);
	endfunction

	function _IsWearingArmor(this, who)
		var itemdesc := ReadConfigFile(":*:itemdesc");
		foreach item in (ListEquippedItems(who))
			if (item.isa(POLCLASS_ARMOR))
				var tipoProtecao := CStr(itemdesc[item.objtype].TipoProtecao);
				if (tipoProtecao["Armadura"])
					return true;
				endif
			endif
		endforeach
		return false;
	endfunction

	function OnPassiveBeforeDamage(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var who := defender.GetObjRef();

		if (this._IsWearingArmor(who))
			return false;
		endif

		var wrestling := CInt(defender.GetSkill(WRESTLING));
		var tactics := CInt(defender.GetSkill(TACTICS));
		var skill := wrestling;
		if (tactics > wrestling)
			skill := tactics;
		endif

		var reduction := CInt(skill / 20);
		if (reduction <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.DecreaseSlashBaseDamage(reduction)
			.DecreasePiercingBaseDamage(reduction)
			.DecreaseBluntBaseDamage(reduction);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[UNARMORED_COMBAT] Reducao passiva: -{reduction}", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
