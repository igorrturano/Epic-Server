include "IAttackInfo";

include "Damage";
include "CombatAbility";
include ":diceroll:model/Dice";

class AttackInfo(IAttackInfo)
	var _attacker; // MobileWrapper
	var _attackerWeapon; // EquipWrapper
	var _defender; // MobileWrapper
	var _defenderWeapon; // EquipWrapper
	var _defenderArmor; // EquipWrapper

	var _damage := Damage();
	var _hitChance := 100;
	var _hitChanceResult := 0; // Resultado da rolagem do dado de hit chance

	var _attackerAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENTUM e contém arrays de CombatAbility()
	var _defenderAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENTUM e contém arrays de CombatAbility()

	function AttackInfo(this, attacker, defender)
		this._damage := AttackInfo::_damage;
		this._attackerAbilities := AttackInfo::_attackerAbilities;
		this._defenderAbilities := AttackInfo::_defenderAbilities;

		this._attacker := attacker;
		this._defender := defender;
	endfunction

	function GetAttacker(this)
		return this._attacker;
	endfunction

	function GetAttackerWeapon(this)
		return this._attackerWeapon;
	endfunction

	function GetDefender(this)
		return this._defender;
	endfunction

	function GetDefenderWeapon(this)
		return this._defenderWeapon;
	endfunction

	function GetDefenderArmor(this)
		return this._defenderArmor;
	endfunction

	function GetHitChance(this)
		return this._hitChance;
	endfunction

	function GetHitChanceDice(this)
		return this._hitChanceDice;
	endfunction

	function GetHitChanceResult(this)
		return this._hitChanceDiceResult;
	endfunction

	function GetDamage(this)
		return this._damage;
	endfunction

	function GetAttackerAbilities(this)
		return this._attackerAbilities;
	endfunction

	function GetDefenderAbilities(this)
		return this._defenderAbilities;
	endfunction

	function SetAttacker(this, attacker)
		this._attacker := attacker;
		return this;
	endfunction

	function SetAttackerWeapon(this, weapon)
		this._attackerWeapon := weapon;
		return this;
	endfunction

	function SetDefender(this, defender)
		this._defender := defender;
		return this;
	endfunction

	function SetDefenderWeapon(this, weapon)
		this._defenderWeapon := weapon;
		return this;
	endfunction

	function SetDefenderArmor(this, armor)
		this._defenderArmor := armor;
		return this;
	endfunction

	function SetHitChance(this, hitChance)
		this._hitChance := hitChance;
		return this;
	endfunction

	function SetHitChanceDice(this, dice)
		this._hitChanceDice := dice;
		return this;
	endfunction

	function SetDamage(this, damage)
		this._damage := damage;
		return this;
	endfunction

	function AddAttackerAbility(this, combatAbility)
		if (!this._attackerAbilities[combatAbility.GetMomentum()]) this._attackerAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._attackerAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function AddDefenderAbility(this, combatAbility)
		if (!this._defenderAbilities[combatAbility.GetMomentum()]) this._defenderAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._defenderAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function RollHitChance(this)
		this._hitChanceResult := Dice("1d" + this._hitChance).Roll();
		return this;
	endfunction

	function Succeeded(this)
		return this._hitChanceResult <= this._hitChance;
	endfunction

	function ExecuteAbilitesOnMomentum(this, momentum)
		if (this._abilitiesToExecute[momentum])
			foreach ability in (this._abilitiesToExecute[momentum])
				ability.ExecuteOn(this._attacker, this._defender);
			endforeach
		endif
	endfunction
endclass