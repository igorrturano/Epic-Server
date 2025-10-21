use uo;

include "IAttackInfo";

include "Damage";
include "CombatAbility";
include ":charsys:ViewModel/MobileWrapper";
include ":equipsys:ViewModel/EquipWrapperArmor";
include ":equipsys:ViewModel/EquipWrapperWeapon";
include ":diceroll:model/Dice";

class AttackInfo(IAttackInfo)
	var _moment := ATTACKMOMENT::COMBATING;
	var _attacker; // MobileWrapper
	var _attackerWeapon; // EquipWrapper
	var _defender; // MobileWrapper
	var _defenderWeapon; // EquipWrapper
	var _defenderArmor; // EquipWrapper

	var _damage := Damage();
	var _damageHand := DAMAGEHAND::MAINHAND; 
	var _hitChance := 100;
	var _hitChancePenalty := 0;
	var _hitChanceResult := 0; // Resultado da rolagem do dado de hit chance

	var _attackerAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENT e contém arrays de CombatAbility()
	var _defenderAbilities := dictionary{}; // O dictionary é indexado por COMBATABILITY_MOMENT e contém arrays de CombatAbility()

	function AttackInfo(this, attacker, defender, moment := ATTACKMOMENT::COMBATING)
		this._moment := moment;
		this._attacker := attacker;
		this._defender := defender;
		this._damageHand := AttackInfo::_damageHand;

		this._attackerAbilities := AttackInfo::_attackerAbilities;
		this._defenderAbilities := AttackInfo::_defenderAbilities;
	endfunction

	function GetMoment(this)
		return this._moment;
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

	function GetHitChancePenalty(this)
		return this._hitChancePenalty;
	endfunction

	function GetHitChanceResult(this)
		return this._hitChanceDiceResult;
	endfunction

	function GetDamage(this)
		return this._damage;
	endfunction

	function GetDamageHand(this)
		return this._damageHand;
	endfunction

	function GetAttackerAbilities(this)
		return this._attackerAbilities;
	endfunction

	function GetDefenderAbilities(this)
		return this._defenderAbilities;
	endfunction

	function SetMoment(this, moment)
		this._moment := moment;
		return this;
	endfunction

	function SetAttacker(this, attacker)
		if (!(attacker is @MobileWrapper)) return error{ errortext := "Parameter must be a MobileWrapper class!" }; endif
		
		this._attacker := attacker;
		return this;
	endfunction

	function SetAttackerWeapon(this, weapon)
		if (!(weapon is @EquipWrapperWeapon)) return error{ errortext := "Parameter must be an EquipWrapperWeapon class!" }; endif

		this._attackerWeapon := weapon;
		return this;
	endfunction

	function SetDefender(this, defender)
		if (!(defender is @MobileWrapper)) return error{ errortext := "Parameter must be a MobileWrapper class!" }; endif

		this._defender := defender;
		return this;
	endfunction

	function SetDefenderWeapon(this, weapon)
		if (!(weapon is @EquipWrapperWeapon)) return error{ errortext := "Parameter must be an EquipWrapperWeapon class!" }; endif

		this._defenderWeapon := weapon;
		return this;
	endfunction

	function SetDefenderArmor(this, armor)
		if (!(armor is @EquipWrapperArmor)) return error{ errortext := "Parameter must be an EquipWrapperArmor class!" }; endif

		this._defenderArmor := armor;
		return this;
	endfunction

	function SetHitChance(this, hitChance)
		this._hitChance := hitChance;
		return this;
	endfunction

	function SetHitChancePenalty(this, penalty)
		this._hitChancePenalty := penalty;
		return this;
	endfunction

	function SetDamage(this, damage)
		if (!(damage is @Damage)) return error{ errortext := "Parameter must be a Damage class!" }; endif

		this._damage := damage;
		return this;
	endfunction

	function SetDamageHand(this, damageHand)
		this._damageHand := damageHand;
		return this;
	endfunction

	function AddAttackerAbility(this, combatAbility)
		if (!(combatAbility is @CombatAbility)) return error{ errortext := "Parameter must be a CombatAbility class!" }; endif
		if (!this._attackerAbilities[combatAbility.GetMomentum()]) this._attackerAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._attackerAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function AddDefenderAbility(this, combatAbility)
		if (!(combatAbility is @CombatAbility)) return error{ errortext := "Parameter must be a CombatAbility class!" }; endif
		if (!this._defenderAbilities[combatAbility.GetMomentum()]) this._defenderAbilities[combatAbility.GetMomentum()] := array{}; endif

		this._defenderAbilities[combatAbility.GetMomentum()].append(combatAbility);

		return this;
	endfunction

	function RollHitChance(this)
		if (!this._hitChance) return error{ errortext := "Hit chance not set!" }; endif
		
		this._hitChanceResult := Dice("1d" + this._hitChance).Roll();
		return this;
	endfunction

	function Succeeded(this)
		if (!this._hitChance) return error{ errortext := "Hitchance not set!" }; endif
		if (!this._hitChanceResult) return error{ errortext := "Hitchance Dice not rolled yet!" }; endif

		return this._hitChanceResult <= this._hitChance;
	endfunction

	function ExecuteAbilitesOnMomentum(this, momentum)
		if (this._abilitiesToExecute[momentum])
			foreach ability in (this._abilitiesToExecute[momentum])
				ability.ExecuteOn(this._attacker, this._defender);
			endforeach
		endif
	endfunction

	function ToString(this)
		// TODO: Implementar uma representação em string do AttackInfo
	endfunction
endclass

enum class ATTACKMOMENT
	ENGAGING,
	COMBATING
endenum

enum class DAMAGEHAND
	MAINHAND,
	OFFHAND
endenum