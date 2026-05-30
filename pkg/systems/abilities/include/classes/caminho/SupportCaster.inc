use uo;

include ":abilities:models/CombatAbility";
include ":tn:cooldown";
include ":attributes:vitals";
include ":message:model/Message";
include ":debug:Controller/DebugController";

/**
 * Support Caster - ABILITY_ID::SUPPORT_CASTER (3401)
 *
 * Passive: Maintain spells at half mana cost + refund 50% mana on spell fail.
 * Merges old "Manter Magias" + "Mago Precavido" into one ability.
 * Effects handled in mantainSpell.src and cast_spell.src.
 *
 * Arcane Archer synergy:
 * - OnPassiveOnMiss: Refund 50% mana (2 of 4) on arcane shot miss.
 * - OnPassiveAfterDamage: Fire arrow hit illuminates target for 10s.
 */
class SupportCaster(CombatAbility)

	function SupportCaster(this)
		Super(ABILITY_ID::SUPPORT_CASTER);
	endfunction

	function OnPassiveOnMiss(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (attacker.GetEquipBySlot(WEAPON).GetCategory() != WEAPON_CATEGORY::MAGICBOW)
			return false;
		endif

		HealMana(who, 2);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[SUPPORT_CASTER] Reembolso de 2 mana (disparo arcano errou).", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

	function OnPassiveAfterDamage(this, attackInfo)
		var attacker := attackInfo.GetAttacker();
		var who := attacker.GetObjRef();

		if (attacker.GetEquipBySlot(WEAPON).GetCategory() != WEAPON_CATEGORY::MAGICBOW)
			return false;
		endif

		var weapon := attacker.GetEquipBySlot(WEAPON);
		if (!weapon)
			return false;
		endif

		if (!weapon.GetObjRef().GetProp("fire_arrow"))
			return false;
		endif

		var defender := attackInfo.GetDefender().GetObjRef();
		SetCooldown(defender, "fire_arrow_lit", 10);

		Message("*iluminado pelo fogo*").SendAbove(defender);

		DebugMsg(
			who,
			DEBUG_HAB,
			Message("[SUPPORT_CASTER] Alvo iluminado por 10s.", Color(SYSTEM_COLOR::SUCCESS))
		);

		return true;
	endfunction

endclass
