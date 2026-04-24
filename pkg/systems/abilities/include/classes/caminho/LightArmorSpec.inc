use uo;

include ":abilities:models/CombatAbility";
include ":debug:Controller/DebugController";
include ":message:model/Message";

/**
 * Light Armor Specialization - ABILITY_ID::LIGHT_ARMOR_SPEC (3200)
 *
 * Passive: +2 evasion for each light armor piece equipped.
 */
class LightArmorSpec(CombatAbility)

	function LightArmorSpec(this)
		Super(ABILITY_ID::LIGHT_ARMOR_SPEC);
	endfunction

	// Defender: +2 evasion per light armor piece
	function OnPassiveBeforeHitDefender(this, attackInfo)
		var defender := attackInfo.GetDefender();
		var items := ListEquippedItems(defender.GetObjRef());
		var count := 0;

		foreach item in items
			if (item.IsA(POLCLASS_ARMOR))
				var armor := ArmorWrapper(item);
				var protType := armor.GetTipoProtecao();
				if (protType && protType["Armadura Leve"])
					count += 1;
				endif
			endif
		endforeach

		if (count <= 0)
			return false;
		endif

		var bonus := count * 2;
		attackInfo.SetHitChanceMod(CInt(attackInfo.GetHitChanceMod()) - bonus);

		DebugMsg(defender.GetObjRef(), DEBUG_HAB,
			Message($"[LIGHT_ARMOR] {count} pecas leves, -{bonus} acerto inimigo", Color(SYSTEM_COLOR::SUCCESS)));
		return true;
	endfunction

endclass
