class IAttackInfo()
	uninit function IAttackInfo(this, attacker, defender, default moment);

	uninit function GetMoment(this);
	uninit function GetAttacker(this);
	uninit function GetDefender(this);
	uninit function GetBodyRegion(this);
	uninit function GetDirection(this);
	uninit function GetDamage(this);
	uninit function GetDamageHand(this);
	uninit function GetAttackType(this);
	uninit function GetAmmo(this);
	uninit function GetHitChance(this);
	uninit function GetHitChanceMod(this);
	uninit function GetHitChanceResult(this);
	uninit function GetCriticalHitChance(this);
	uninit function GetCriticalHitChanceMod(this);
	uninit function GetCriticalHitChanceResult(this);
	uninit function GetParryEquip(this);
	uninit function GetParryChance(this);
	uninit function GetParryChanceMod(this);
	uninit function GetParryChanceResult(this);
	uninit function GetAttackerAbilities(this);
	uninit function GetDefenderAbilities(this);

	uninit function SetMoment(this, moment);
	uninit function SetAttacker(this, attacker);
	uninit function SetDefender(this, defender);
	uninit function SetBodyRegion(this, bodyRegion);
	uninit function SetDirection(this, direction);
	uninit function SetDamage(this, damage);
	uninit function SetDamageHand(this, damageHand);
	uninit function SetAttackType(this, attackType);
	uninit function SetAmmo(this, ammo);
	uninit function SetHitChance(this, value);
	uninit function SetHitChanceMod(this, value);
	uninit function SetCriticalHitChance(this, value);
	uninit function SetCriticalHitChanceMod(this, value);
	uninit function SetParryEquip(this, equip);
	uninit function SetParryChance(this, parryChance);
	uninit function SetParryChancePenalty(this, penalty);

	uninit function RollHitChance(this);
	uninit function RollParryChance(this);
	uninit function RollCriticalHitChance(this);
	uninit function Succeeded(this);
	uninit function Parried(this);
	uninit function ExecuteAbilitesOnMomentum(this, momentum);
endclass