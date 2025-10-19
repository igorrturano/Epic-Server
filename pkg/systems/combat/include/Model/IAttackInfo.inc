class IAttackInfo()
	uninit function IAttackInfo(this);

	uninit function SetAttacker(this, attacker);
	uninit function SetDefender(this, defender);
	uninit function SetArmorHit(this, armor);
	uninit function SetSlotHit(this, slot);

	uninit function SetWeapon(this, weapon);
	uninit function SetDamage(this, damage);

	uninit function AddCombatAbility_BeforeDamage(this, target, combatAbility);
	uninit function AddCombatAbility_AfterDamage(this, target, combatAbility);
	uninit function AddCombatAbility_OnHit(this, target, combatAbility);
	uninit function AddCombatAbility_OnMiss(this, target, combatAbility);
endclass