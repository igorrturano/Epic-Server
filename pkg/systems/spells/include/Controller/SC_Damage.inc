use uo;

include ":damage:damage";
include ":wrappers:Objects/Model/MobileWrapper";
include ":combat:Model/AttackInfo";
include ":combat:Controller/CombatAbilityController";

/**
 * SC_DealDamage — canonical entry point for spell damage.
 *
 * Applies resist-aware damage and fires the after-damage ability hooks
 * (CombatAbility AFTER_DAMAGE / ON_KILL and PassiveAbility AFTER_DAMAGE)
 * via a spell-flagged AttackInfo. Spells that migrate to this call stop
 * using the legacy DamageFLS / ApplyRawDamage path directly.
 *
 * @param caster       Mobile - the attacker
 * @param targ         Mobile - the victim
 * @param spellId      Integer - spell id (for AttackInfo.GetSpellId)
 * @param amount       Integer - base damage amount, before resist
 * @param damageType   String  - DMG_FIRE / DMG_COLD / DMG_ENERGY / DMG_POISON
 *                               / DMG_PHYSICAL; DMG_FORCED to bypass resist
 * @return Integer - final damage applied after resist (0 if no damage landed)
 */
function SC_DealDamage(caster, targ, spellId, amount, damageType := DMG_FORCED)
	amount := CInt(amount);
	if (!caster || !targ || amount <= 0)
		return 0;
	endif

	if (damageType != DMG_FORCED)
		amount := CInt(CalcDamageAmount(targ, amount, damageType, caster));
	endif
	if (amount <= 0)
		return 0;
	endif

	SetLastDamageInfo(targ, amount, damageType, caster, 1, $"spell_{spellId}");
	SetScriptController(caster);

	DisplayDamage(targ, amount, caster);
	ApplyRawDamage(targ, amount);

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
// type slot so ability hooks that read Damage see a non-zero value. We mirror
// the base-damage pattern DC uses after resist.
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
