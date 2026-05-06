use uo;

include ":wrappers:Objects/Model/MobileWrapper";
include ":combat:Model/AttackInfo";
include ":combat:Controller/CombatAbilityController";
include ":combat:Controller/DamageController_legacy";
include "MagicItemBonus";

/**
 * SC_DealDamage — canonical entry point for spell damage.
 *
 * Delegates the actual damage application to DamageFLS (resist calc,
 * DisplayDamage, ApplyRawDamage, last-damage-info + autopsy), then fires
 * the post-damage ability hooks (CombatAbility AFTER_DAMAGE / ON_KILL
 * and PassiveAbility AFTER_DAMAGE) via a spell-flagged AttackInfo.
 * Spells that migrate to this call stop using DamageFLS directly so all
 * spell damage consistently triggers ability hooks.
 *
 * @param caster       Mobile - the attacker
 * @param targ         Mobile - the victim
 * @param spellId      Integer - spell id (for AttackInfo.GetSpellId)
 * @param amount       Integer - base damage amount, before resist
 * @param damageType   String  - DMG_FIRE / DMG_COLD / DMG_ENERGY / DMG_POISON
 *                               / DMG_PHYSICAL; DMG_FORCED to bypass resist
 * @return Integer - the base amount passed in (resist is handled internally)
 */
function SC_DealDamage(caster, targ, spellId, amount, damageType := DMG_FORCED)
	amount := CInt(amount);
	if (!caster || !targ || amount <= 0)
		return 0;
	endif

	// Magic item elemental bonus from equipped focus (staff/wand/spellbook).
	// Reads `dano <element>` sub-value from the focus's crafted properties.
	amount := amount + GetMagicItemElementalBonus(caster, damageType);

	// DamageFLS: resist calc + DisplayDamage + ApplyRawDamage + last-damage info.
	DamageFLS(targ, amount, damageType, caster, $"spell_{spellId}");

	// Build a spell-origin AttackInfo and fire post-damage hooks so abilities
	// like ON_KILL / AFTER_DAMAGE effects work for spell damage, not just melee.
	var attInfo := AttackInfo(MobileWrapper(caster), MobileWrapper(targ));
	attInfo.SetSpellOrigin(spellId, damageType);
	_SC_StampSpellDamage(attInfo, damageType, amount);

	ExecuteCombatAbilitiesAfterDamage(attInfo);
	ExecutePassiveAbilitiesAfterDamage(attInfo);

	if (targ.dead)
		ExecuteCombatAbilitiesOnKill(attInfo);
		ExecutePassiveAbilitiesOnKill(attInfo);
	endif

	return amount;
endfunction

// Stamp the landed damage onto the AttackInfo's Damage object in the right
// type slot so ability hooks that read Damage see a non-zero value.
function _SC_StampSpellDamage(byref attInfo, damageType, amount)
	var dmg := attInfo.GetDamage();
	case (damageType)
		DMG_FIRE:     dmg.SetFireBaseDamage(amount);
		DMG_COLD:     dmg.SetColdBaseDamage(amount);
		DMG_POISON:   dmg.SetPoisonBaseDamage(amount);
		DMG_ENERGY:   dmg.SetEnergyBaseDamage(amount);
		DMG_PHYSICAL: dmg.SetSlashBaseDamage(amount); // physical → slash as the generic physical slot
		default:      dmg.SetEnergyBaseDamage(amount); // DMG_FORCED or unknown — park it in energy
	endcase
endfunction
