use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";

/**
 * Medium Armor Specialization - ABILITY_ID::MEDIUM_ARMOR_SPEC (3201)
 *
 * Passive: +1 evasion for each medium armor piece equipped.
 * Also reduces dex penalty by 1 (handled in equipsys/equipitem.src and equipsys/unequipitem.src).
 */
class MediumArmorSpec(CombatAbility)

	function MediumArmorSpec(this)
		Super(ABILITY_ID::MEDIUM_ARMOR_SPEC);
	endfunction

	// Defender: +1 evasion per medium armor piece
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var items := ListEquippedItems(defender.GetObjRef());
		var count := 0;

		foreach item in items
			if (item.IsA(POLCLASS_ARMOR))
				var armor := ArmorWrapper(item);
				var protType := armor.GetTipoProtecao();
				if (protType && protType["Armadura Media"])
					count += 1;
				endif
			endif
		endforeach

		if (count <= 0)
			return false;
		endif

		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) - count);

		DebugMsg(defender.GetObjRef(), DEBUG_HAB,
			Message($"[MEDIUM_ARMOR] {count} pecas medias, -{count} acerto inimigo", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
