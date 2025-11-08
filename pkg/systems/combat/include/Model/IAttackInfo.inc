class IAttackInfo()
	uninit function IAttackInfo(this, attacker, defender, default moment);

	uninit function GetMoment(this);
	uninit function GetAttacker(this);
	uninit function GetDefender(this);
	uninit function GetHitChance(this);
	uninit function GetHitChancePenalty(this);
	uninit function GetHitChanceResult(this);
	uninit function GetDamage(this);
	uninit function GetAttackerAbilities(this);
	uninit function GetDefenderAbilities(this);

	uninit function SetMoment(this, moment);
	uninit function SetAttacker(this, attacker);
	uninit function SetDefender(this, defender);
	uninit function SetHitChance(this, hitChance);
	uninit function SetHitChancePenalty(this, penalty);
	uninit function SetDamage(this, damage);
	uninit function AddAttackerAbility(this, combatAbility);
	uninit function AddDefenderAbility(this, combatAbility);

	uninit function RollHitChance(this);
	uninit function Succeeded(this);
	uninit function ExecuteAbilitesOnMomentum(this, momentum);
endclass