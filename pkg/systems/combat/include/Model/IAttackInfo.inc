class IAttackInfo()
	uninit function IAttackInfo(this, attacker, defender, default moment);

	uninit function GetMoment(this);
	uninit function SetMoment(this, moment);

	uninit function GetAttacker(this);
	uninit function SetAttacker(this, attacker);

	uninit function GetDefender(this);
	uninit function SetDefender(this, defender);

	uninit function GetBodyRegion(this);
	uninit function SetBodyRegion(this, bodyRegion);

	uninit function GetDirection(this);
	uninit function SetDirection(this, direction);

	uninit function GetDamage(this);
	uninit function SetDamage(this, damage);

	uninit function GetDamageHand(this);
	uninit function SetDamageHand(this, damageHand);

	uninit function GetAttackType(this);
	uninit function SetAttackType(this, attackType);

	uninit function GetWeapon(this);
	uninit function SetWeapon(this, weapon);

	uninit function GetAmmo(this);
	uninit function SetAmmo(this, ammo);
	
	uninit function GetHitChance(this);
	uninit function SetHitChance(this, value);
	
	uninit function GetHitChanceMod(this);
	uninit function SetHitChanceMod(this, value);
	
	uninit function GetHitChanceResult(this);
	uninit function GetCriticalHitChance(this);
	
	uninit function SetCriticalHitChance(this, value);
	uninit function GetCriticalHitChanceMod(this);
	
	uninit function SetCriticalHitChanceMod(this, value);
	uninit function GetCriticalHitChanceResult(this);
	
	uninit function GetParryEquip(this);
	uninit function SetParryEquip(this, equip);
	
	uninit function GetParryChance(this);
	uninit function SetParryChance(this, parryChance);
	
	uninit function GetParryChanceMod(this);
	uninit function SetParryChancePenalty(this, penalty);
	
	uninit function GetParryChanceResult(this);
	uninit function GetAttackerAbilities(this);
	uninit function GetDefenderAbilities(this);

	uninit function RollHitChance(this);
	uninit function RollParryChance(this);
	uninit function RollCriticalHitChance(this);
	uninit function Succeeded(this);
	uninit function Parried(this);
	uninit function ExecuteAbilitesOnMomentum(this, momentum);
endclass