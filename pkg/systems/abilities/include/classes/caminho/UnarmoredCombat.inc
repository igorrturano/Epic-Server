use uo;

include ":abilities:models/CombatAbility";
include ":message:model/Message";
include ":debug:Controller/DebugController";
include ":attributes:attributes";

/**
 * Combate Leve (Unarmored Combat) - ABILITY_ID::UNARMORED_COMBAT (3628)
 *
 * Passive (DefenderBeforeDamage): reduz dano fisico recebido em
 * max(Wrestling, Tactics) / 2 % — multiplicativo, GM = 50%.
 * Aplica apenas se totalmente nu (sem nenhuma peca com TipoProtecao).
 *
 * Unica fonte de absorcao para builds sem armor apos o redesign de combate
 * (BarbarianResilience, DragonStance e CrabStyle perderam absorcao passiva).
 */
class UnarmoredCombat(CombatAbility)

	function UnarmoredCombat(this)
		Super(ABILITY_ID::UNARMORED_COMBAT);
		this.IsDefenderSide := true;
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
		print($"[UNARMORED_COMBAT.OnPassiveBeforeDamage] attacker={attackInfo.GetAttacker().GetObjRef().serial} defender={attackInfo.GetDefender().GetObjRef().serial}");
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

		var reductionPct := CInt(skill / 2);
		if (reductionPct <= 0)
			return false;
		endif

		attackInfo.GetDamage()
			.DecreaseSlashPercentDamage(reductionPct)
			.DecreasePiercingPercentDamage(reductionPct)
			.DecreaseBluntPercentDamage(reductionPct);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message($"[UNARMORED_COMBAT] Reducao passiva: -{reductionPct}%", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
